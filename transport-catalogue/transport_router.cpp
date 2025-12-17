#include "transport_router.h"
#include "json_builder.h"

#include <algorithm>
#include <cmath>
#include <iostream>

namespace transport_router {

using namespace std;
using namespace transport_catalogue;

TransportRouter::TransportRouter(const TransportCatalogue& catalogue, RoutingSettings settings)
    : catalogue_(catalogue)
    , settings_(settings) {
    BuildGraph();
}

void TransportRouter::BuildGraph() {
    const auto& stops = catalogue_.GetStops();
    const size_t vertex_count = stops.size();
    
    graph_ = make_unique<graph::DirectedWeightedGraph<double>>(vertex_count);
    
    stop_name_to_vertex_id_.reserve(vertex_count);
    vertex_id_to_stop_name_.reserve(vertex_count);

    graph::VertexId v_id = 0;
    for (const auto& stop : stops) {
        stop_name_to_vertex_id_[stop.name] = v_id;
        vertex_id_to_stop_name_.push_back(stop.name);
        ++v_id;
    }

    const auto& buses = catalogue_.GetBuses();
    for (const auto& bus : buses) {
        const auto& route = bus.route;
        if (route.empty()) continue;

        auto add_route_edges = [&](const std::vector<const Stop*>& sequence) {
            for (size_t i = 0; i < sequence.size(); ++i) {
                double current_dist_sum = 0.0;
                int span_count = 0;

                for (size_t j = i + 1; j < sequence.size(); ++j) {
                    const Stop* from_stop = sequence[j - 1];
                    const Stop* to_stop = sequence[j];

                    current_dist_sum += catalogue_.GetLength(from_stop->name, to_stop->name);
                    span_count++;

                    double travel_time = (current_dist_sum / 1000.0) / settings_.bus_velocity * 60.0;
                    
                    double total_weight = settings_.bus_wait_time + travel_time;

                    graph::VertexId from_id = stop_name_to_vertex_id_.at(sequence[i]->name);
                    graph::VertexId to_id = stop_name_to_vertex_id_.at(sequence[j]->name);
                    
                    graph::EdgeId edge_id = graph_->AddEdge({from_id, to_id, total_weight});

                    if (edge_id >= edge_infos_.size()) {
                        edge_infos_.resize(edge_id + 1);
                    }
                    edge_infos_[edge_id] = {bus.name, travel_time, span_count};
                }
            }
        };

        if (bus.is_round_trip) {
            add_route_edges(route);
        } else {
            add_route_edges(route);
            
            if (!route.empty()) {
                std::vector<const Stop*> backward_route = route;
                std::reverse(backward_route.begin(), backward_route.end());
                add_route_edges(backward_route);
            }
        }
    }

    router_ = make_unique<graph::Router<double>>(*graph_);
}

std::optional<json::Node> TransportRouter::FindRoute(const std::string& from, const std::string& to, int request_id) const {
    if (stop_name_to_vertex_id_.count(from) == 0 || stop_name_to_vertex_id_.count(to) == 0) {
        return std::nullopt;
    }

    graph::VertexId from_id = stop_name_to_vertex_id_.at(from);
    graph::VertexId to_id = stop_name_to_vertex_id_.at(to);

    if (from_id == to_id) {
        return json::Builder{}
            .StartDict()
                .Key("request_id").Value(request_id)
                .Key("total_time").Value(0.0)
                .Key("items").StartArray().EndArray()
            .EndDict()
            .Build();
    }

    auto route_info = router_->BuildRoute(from_id, to_id);

    if (!route_info) {
        return std::nullopt;
    }

    json::Array items;
    double total_time = route_info->weight;

    for (graph::EdgeId edge_id : route_info->edges) {
        const auto& edge = graph_->GetEdge(edge_id);
        const auto& info = edge_infos_.at(edge_id);
        
        items.push_back(json::Builder{}
            .StartDict()
                .Key("type").Value("Wait")
                .Key("stop_name").Value(std::string(vertex_id_to_stop_name_[edge.from]))
                .Key("time").Value(settings_.bus_wait_time)
            .EndDict()
            .Build()
        );

        items.push_back(json::Builder{}
            .StartDict()
                .Key("type").Value("Bus")
                .Key("bus").Value(info.bus_name)
                .Key("span_count").Value(info.span_count)
                .Key("time").Value(info.travel_time)
            .EndDict()
            .Build()
        );
    }

    return json::Builder{}
        .StartDict()
            .Key("request_id").Value(request_id)
            .Key("total_time").Value(total_time)
            .Key("items").Value(std::move(items))
        .EndDict()
        .Build();
}

}
