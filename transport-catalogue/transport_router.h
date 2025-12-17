#pragma once

#include "transport_catalogue.h"
#include "router.h"
#include "graph.h"
#include "json.h"
#include "json_builder.h"

#include <memory>
#include <unordered_map>
#include <vector>
#include <string>

namespace transport_router {

struct RoutingSettings {
    int bus_wait_time = 0;
    double bus_velocity = 0.0;
};

class TransportRouter {
public:
    TransportRouter(const transport_catalogue::TransportCatalogue& catalogue, RoutingSettings settings);

    std::optional<json::Node> FindRoute(const std::string& from, const std::string& to, int request_id) const;

private:
    struct GraphEdgeInfo {
        std::string bus_name;
        double travel_time;
        int span_count;
    };

    const transport_catalogue::TransportCatalogue& catalogue_;
    RoutingSettings settings_;

    std::unique_ptr<graph::DirectedWeightedGraph<double>> graph_;
    std::unique_ptr<graph::Router<double>> router_;

    std::unordered_map<std::string_view, graph::VertexId> stop_name_to_vertex_id_;
    std::vector<std::string_view> vertex_id_to_stop_name_;
    
    std::vector<GraphEdgeInfo> edge_infos_;

    void BuildGraph();
    const graph::DirectedWeightedGraph<double>& GetGraph() const;
    const graph::Router<double>& GetRouter() const;
};

}