#include "json_reader.h"
#include "json_builder.h"
#include "transport_router.h"

#include <sstream>
#include <algorithm>
#include <utility>

using namespace std;

namespace jsonreader {
    json::Document JsonReader::ReadData(std::istream& input) {
        document_json_ = json::Load(input);
        return document_json_;
    }

    void JsonReader::SetCatalogueData(transport_catalogue::TransportCatalogue& tc) {
        const auto& root = document_json_.GetRoot().AsMap();
        if (!root.count("base_requests")) return;
        const json::Array& arr = root.at("base_requests").AsArray();

        for (const auto& item : arr) {
            const json::Dict& cmd = item.AsMap();
            if (cmd.at("type").AsString() == "Stop") {
                std::string name = cmd.at("name").AsString();
                double lat = cmd.at("latitude").AsDouble();
                double lng = cmd.at("longitude").AsDouble();
                tc.AddStop({ std::move(name), { lat, lng } });
            }
        }

        for (const auto& item : arr) {
            const json::Dict& cmd = item.AsMap();
            if (cmd.at("type").AsString() == "Stop") {
                if (cmd.count("road_distances")) {
                    const json::Dict& dists = cmd.at("road_distances").AsMap();
                    std::string from = cmd.at("name").AsString();
                    for (const auto& p : dists) {
                        tc.AddLength({ from, p.first }, p.second.AsInt());
                    }
                }
            }
        }

        for (const auto& item : arr) {
            const json::Dict& cmd = item.AsMap();
            if (cmd.at("type").AsString() == "Bus") {
                std::string name = cmd.at("name").AsString();
                bool is_round = cmd.at("is_roundtrip").AsBool();
                vector<const Stop*> route;
                for (const auto& s : cmd.at("stops").AsArray()) {
                    const Stop* st = tc.FindStop(s.AsString());
                    route.push_back(st);
                }
                if (!is_round && route.size() > 1) {
                    for (int i = static_cast<int>(route.size()) - 2; i >= 0; --i) {
                        route.push_back(route[i]);
                    }
                }
                tc.AddBus({ std::move(name), std::move(route), is_round });
            }
        }
    }

    svg::Color JsonReader::GetJsonColor(const json::Node& color) const {
        if (color.IsString()) {
            return color.AsString();
        }
        const json::Array& arr = color.AsArray();
        if (arr.size() == 3) {
            return svg::Rgb(arr[0].AsInt(), arr[1].AsInt(), arr[2].AsInt());
        } else if (arr.size() == 4) {
            return svg::Rgba(arr[0].AsInt(), arr[1].AsInt(), arr[2].AsInt(), arr[3].AsDouble());
        }
        throw out_of_range("Unknown color format");
    }

    void JsonReader::SetRendererData(MapRenderer& renderer) {
        const auto& root = document_json_.GetRoot().AsMap();
        if (!root.count("render_settings")) return;
        const auto& s = root.at("render_settings").AsMap();

        RenderSettings rs;

        rs.width = s.at("width").AsDouble();
        rs.height = s.at("height").AsDouble();
        rs.padding = s.at("padding").AsDouble();
        rs.line_width = s.at("line_width").AsDouble();
        rs.stop_radius = s.at("stop_radius").AsDouble();

        rs.bus_label_font_size = s.at("bus_label_font_size").AsInt();
        rs.bus_label_offset = {
            s.at("bus_label_offset").AsArray()[0].AsDouble(),
            s.at("bus_label_offset").AsArray()[1].AsDouble()
        };

        rs.stop_label_font_size = s.at("stop_label_font_size").AsInt();
        rs.stop_label_offset = {
            s.at("stop_label_offset").AsArray()[0].AsDouble(),
            s.at("stop_label_offset").AsArray()[1].AsDouble()
        };

        rs.underlayer_width = s.at("underlayer_width").AsDouble();
        rs.underlayer_color = GetJsonColor(s.at("underlayer_color"));


        for (const auto& c : s.at("color_palette").AsArray()) {
            rs.color_palette.push_back(GetJsonColor(c));
        }

        renderer.SetSettings(rs);
    }


    json::Node ProcessMapRequest(int id, RequestHandler& rh, const MapRenderer& map_rend) {
        auto objects_opt = rh.GetRenderingObjects();
        std::ostringstream svg_out;
        if (objects_opt.has_value()) {
            map_rend.RenderMap(std::move(objects_opt.value())).Render(svg_out);
        } else {
            svg::Document{}.Render(svg_out);
        }
        
        return json::Builder{}
            .StartDict()
                .Key("map").Value(svg_out.str())
                .Key("request_id").Value(id)
            .EndDict()
            .Build();
    }

    json::Node ProcessStopRequest(int id, const std::string& name, RequestHandler& rh) {
        auto stop_info_opt = rh.GetStopInfo(name);
        if (!stop_info_opt.has_value()) {
            return json::Builder{}
                .StartDict()
                    .Key("request_id").Value(id)
                    .Key("error_message").Value("not found")
                .EndDict()
                .Build();
        }
        const auto& si = stop_info_opt.value();
        std::vector<std::string> bus_names;
        bus_names.reserve(si.buses.size());
        for (const auto* b : si.buses) {
            if (b) bus_names.push_back(b->name);
        }
        std::sort(bus_names.begin(), bus_names.end());
        
        json::Builder builder;
        builder.StartDict()
               .Key("buses").StartArray();
        for (auto& bn : bus_names) {
            builder.Value(std::move(bn));
        }
        return builder.EndArray()
                      .Key("request_id").Value(id)
                      .EndDict()
                      .Build();
    }

    json::Node ProcessBusRequest(int id, const std::string& name, const transport_catalogue::TransportCatalogue& tc) {
        auto bus_info_opt = tc.GetBusInfo(name);
        if (!bus_info_opt.has_value()) {
            return json::Builder{}
                .StartDict()
                    .Key("request_id").Value(id)
                    .Key("error_message").Value("not found")
                .EndDict()
                .Build();
        }

        const auto& bi = bus_info_opt.value();
        return json::Builder{}
            .StartDict()
                .Key("curvature").Value(bi.curvature)
                .Key("request_id").Value(id)
                .Key("route_length").Value(bi.length_route)
                .Key("stop_count").Value(bi.num_stops)
                .Key("unique_stop_count").Value(bi.uniq_stops)
            .EndDict()
            .Build();
    }


    json::Node ProcessUnknownRequest(int id) {
        return json::Builder{}
            .StartDict()
                .Key("request_id").Value(id)
                .Key("error_message").Value("not found")
            .EndDict()
            .Build();
    }

    void JsonReader::OutputStatRequests(const transport_catalogue::TransportCatalogue& tc, 
                                        const MapRenderer& map_rend, 
                                        std::ostream& output) {
        const auto& root = document_json_.GetRoot().AsMap();
        RequestHandler rh(tc, map_rend);

        // prepare routing settings and router
        transport_router::RoutingSettings routing_settings;
        if (root.count("routing_settings")) {
            const auto& rs = root.at("routing_settings").AsMap();
            if (rs.count("bus_wait_time")) {
                routing_settings.bus_wait_time = rs.at("bus_wait_time").AsInt();
            }
            if (rs.count("bus_velocity")) {
                routing_settings.bus_velocity = rs.at("bus_velocity").AsDouble();
            }
        }

        transport_router::TransportRouter router(tc, routing_settings);

        if (!root.count("stat_requests")) {
             return;
        }

        const json::Array& arr = root.at("stat_requests").AsArray();
        json::Array results;
        results.reserve(arr.size());

        for (const auto& req_node : arr) {
            const json::Dict& cmd = req_node.AsMap();
            const int id = cmd.at("id").AsInt();
            const std::string type = cmd.at("type").AsString();

            if (type == "Map") {
                results.push_back(ProcessMapRequest(id, rh, map_rend));
            } else if (type == "Stop") {
                results.push_back(ProcessStopRequest(id, cmd.at("name").AsString(), rh));
            } else if (type == "Bus") {
                results.push_back(ProcessBusRequest(id, cmd.at("name").AsString(), tc));
            } else if (type == "Route") {
                // read from/to and ask transport router
                const std::string from = cmd.at("from").AsString();
                const std::string to = cmd.at("to").AsString();
                auto route_node_opt = router.FindRoute(from, to, id);
                if (route_node_opt.has_value()) {
                    results.push_back(route_node_opt.value());
                } else {
                    results.push_back(ProcessUnknownRequest(id));
                }
            } else {
                results.push_back(ProcessUnknownRequest(id));
            }
        }

        json::Print(json::Document(json::Node(std::move(results))), output);
    }

}
