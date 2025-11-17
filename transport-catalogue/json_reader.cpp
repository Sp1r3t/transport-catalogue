#include "json_reader.h"

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
                vector<const transport_catalogue::Stop*> route;
                for (const auto& s : cmd.at("stops").AsArray()) {
                    const transport_catalogue::Stop* st = tc.FindStop(s.AsString());
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

    void JsonReader::SetRendererData(MapRenderer& map_rend) {
        const auto& root = document_json_.GetRoot().AsMap();
        if (!root.count("render_settings")) return;
        const json::Dict& rs = root.at("render_settings").AsMap();

        map_rend.SetWidth(rs.at("width").AsDouble())
            .SetHeight(rs.at("height").AsDouble())
            .SetPadding(rs.at("padding").AsDouble())
            .SetLineWidth(rs.at("line_width").AsDouble())
            .SetStopRadius(rs.at("stop_radius").AsDouble())
            .SetBusLabelFontSize(rs.at("bus_label_font_size").AsInt())
            .SetBusLabelOffset({ rs.at("bus_label_offset").AsArray()[0].AsDouble(),
                                rs.at("bus_label_offset").AsArray()[1].AsDouble() })
            .SetStopLabelFontSize(rs.at("stop_label_font_size").AsInt())
            .SetStopLabelOffset({ rs.at("stop_label_offset").AsArray()[0].AsDouble(),
                                rs.at("stop_label_offset").AsArray()[1].AsDouble() })
            .SetUnderlayerWidth(rs.at("underlayer_width").AsDouble());

        if (rs.count("underlayer_color")) {
            map_rend.SetUnderlayerColor(GetJsonColor(rs.at("underlayer_color")));
        }

        vector<svg::Color> palette;
        if (rs.count("color_palette")) {
            for (const auto& c : rs.at("color_palette").AsArray()) {
                palette.push_back(GetJsonColor(c));
            }
        } else if (rs.count("colors")) {
            for (const auto& c : rs.at("colors").AsArray()) {
                palette.push_back(GetJsonColor(c));
            }
        }
        map_rend.SetColorPalette(palette);
    }

    void JsonReader::OutputStatRequests(const transport_catalogue::TransportCatalogue& tc,
                                    const MapRenderer& map_rend,
                                    std::ostream& output) {
        const auto& root = document_json_.GetRoot().AsMap();

        RequestHandler rh(tc, map_rend);

        if (!root.count("stat_requests")) {
            auto objects_opt = rh.GetRenderingObjects();
            if (objects_opt.has_value()) {
                map_rend.RenderMap(std::move(objects_opt.value())).Render(output);
            } else {
                svg::Document{}.Render(output);
            }
            return;
        }

        const json::Array& arr = root.at("stat_requests").AsArray();
        json::Array results;
        results.reserve(arr.size());

        for (const auto& req_node : arr) {
            const json::Dict& cmd = req_node.AsMap();
            const int id = cmd.at("id").AsInt();
            const string type = cmd.at("type").AsString();

            if (type == "Map") {
                auto objects_opt = rh.GetRenderingObjects();
                ostringstream svg_out;
                if (objects_opt.has_value()) {
                    map_rend.RenderMap(std::move(objects_opt.value())).Render(svg_out);
                } else {
                    svg::Document{}.Render(svg_out);
                }
                std::string svg_str = svg_out.str();
                json::Dict res;
                res["map"] = json::Node(std::move(svg_str));
                res["request_id"] = json::Node(id);
                results.push_back(json::Node(std::move(res)));
                continue;
            }

            if (type == "Stop") {
                const string name = cmd.at("name").AsString();
                auto stop_info_opt = rh.GetStopInfo(name);
                if (!stop_info_opt.has_value()) {
                    json::Dict err;
                    err["request_id"] = json::Node(id);
                    std::string em = "not found";
                    err["error_message"] = json::Node(std::move(em));
                    results.push_back(json::Node(std::move(err)));
                } else {
                    const auto& si = stop_info_opt.value();
                    vector<string> bus_names;
                    bus_names.reserve(si.buses.size());
                    for (const auto* b : si.buses) {
                        if (b) bus_names.push_back(b->name);
                    }
                    sort(bus_names.begin(), bus_names.end());
                    json::Array buses_json;
                    buses_json.reserve(bus_names.size());
                    for (auto& bn : bus_names) {
                        buses_json.push_back(json::Node(std::move(bn)));
                    }
                    json::Dict res;
                    res["buses"] = json::Node(std::move(buses_json));
                    res["request_id"] = json::Node(id);
                    results.push_back(json::Node(std::move(res)));
                }
                continue;
            }

            if (type == "Bus") {
                const string name = cmd.at("name").AsString();
                auto bus_info_opt = rh.GetBusInfo(name);
                if (!bus_info_opt.has_value()) {
                    json::Dict err;
                    err["request_id"] = json::Node(id);
                    std::string em = "not found";
                    err["error_message"] = json::Node(std::move(em));
                    results.push_back(json::Node(std::move(err)));
                } else {
                    const auto& bi = bus_info_opt.value();
                    json::Dict res;
                    res["stop_count"] = json::Node(bi.num_stops);
                    res["unique_stop_count"] = json::Node(bi.uniq_stops);
                    res["route_length"] = json::Node(bi.length_route);
                    res["curvature"] = json::Node(bi.curvature);
                    res["request_id"] = json::Node(id);
                    results.push_back(json::Node(std::move(res)));
                }
                continue;
            }

            json::Dict err;
            err["request_id"] = json::Node(id);
            std::string em = "not found";
            err["error_message"] = json::Node(std::move(em));
            results.push_back(json::Node(std::move(err)));
        }

        json::Document out_doc(json::Node(std::move(results)));
        json::Print(out_doc, output);
    }

}