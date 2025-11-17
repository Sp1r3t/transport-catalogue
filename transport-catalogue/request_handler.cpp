#include "request_handler.h"
#include "map_renderer.h"
#include "transport_catalogue.h"

#include <set>
#include <algorithm>
#include <cmath>
#include <unordered_set>

using namespace std;

double RequestHandler::ComputeDistance(const geo::Coordinates& a, const geo::Coordinates& b) const {
    return geo::ComputeDistance(a, b);
}

optional<StopInfo> RequestHandler::GetStopInfo(string_view stop_name) const {
    const auto* stop = tc_.FindStop(stop_name);
    if (!stop) return nullopt;

    StopInfo info;
    info.name = stop->name;

    auto buses = tc_.GetPassingBuses(stop->name);
    info.buses.reserve(buses.size());
    for (const auto* b : buses) info.buses.push_back(b);

    return info;
}

optional<BusInfo> RequestHandler::GetBusInfo(string_view bus_name) const {
    const auto* bus = tc_.FindBus(bus_name);
    if (!bus) return nullopt;

    BusInfo info;
    info.name = bus->name;
    info.num_stops = static_cast<int>(bus->route.size());

    set<string> unique_stops;
    double geo_length = 0.0;
    int real_length = 0;

    for (size_t i = 0; i + 1 < bus->route.size(); ++i) {
        const auto* a = bus->route[i];
        const auto* b = bus->route[i + 1];
        if (!a || !b) continue;

        unique_stops.insert(a->name);
        geo_length += geo::ComputeDistance(a->coord, b->coord);
        real_length += tc_.GetLength(a->name, b->name);
    }

    if (!bus->route.empty())
        unique_stops.insert(bus->route.back()->name);

    info.uniq_stops = static_cast<int>(unique_stops.size());
    info.length_route = real_length;
    info.curvature =
        geo_length > 0.0 ? static_cast<double>(real_length) / geo_length : 0.0;

    return info;
}

RequestHandler::RenderingObjects RequestHandler::GetRenderingObjects() const {
    unordered_set<string> used_stop_names;
    for (const auto& bus : tc_.GetBuses()) {
        if (bus.route.size() < 2) continue;
        for (const auto* s : bus.route) {
            if (s) used_stop_names.insert(s->name);
        }
    }

    if (used_stop_names.empty()) return nullopt;

    vector<geo::Coordinates> coords;
    coords.reserve(used_stop_names.size());
    for (const auto& stop : tc_.GetStops()) {
        if (used_stop_names.count(stop.name)) {
            coords.push_back(stop.coord);
        }
    }
    if (coords.empty()) return nullopt;

    SphereProjector proj(coords.begin(), coords.end(), mr_.GetWidth(), mr_.GetHeight(), mr_.GetPadding());

    map<string, svg::Point> stops_to_draw;
    for (const auto& stop : tc_.GetStops()) {
        if (used_stop_names.count(stop.name)) {
            stops_to_draw[stop.name] = proj(stop.coord);
        }
    }

    map<string, pair<vector<svg::Point>, bool>> buses_to_draw;
    for (const auto& bus : tc_.GetBuses()) {
        if (bus.route.empty()) continue;

        vector<svg::Point> pts;
        pts.reserve(bus.route.size());
        for (const auto* s : bus.route) {
            if (!s) continue;
            auto it = stops_to_draw.find(s->name);
            if (it != stops_to_draw.end()) {
                pts.push_back(it->second);
            }
        }
        if (pts.size() < 2) continue;

        buses_to_draw[bus.name] = { move(pts), bus.is_round_trip };
    }

    return make_optional(make_pair(move(stops_to_draw), move(buses_to_draw)));
}
