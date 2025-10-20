#include "transport_catalogue.h"

#include <unordered_set>
#include <set>
#include <cmath>
#include <iostream>

namespace transport_catalogue {
    using TC = TransportCatalogue;

    void TC::AddStop(std::string_view name, Coordinates coordinates) {
        stops_.push_back({ std::string(name), coordinates, {} });
        info::BusStop* stop_ptr = &stops_.back();
        stopname_to_stop_[stop_ptr->name] = stop_ptr;
    }

    void TC::AddDistance(std::string_view name1, std::string_view name2, int distance){
        pending_distances_.push_back({ std::string(name1), std::string(name2), distance });
    }

    const info::BusStop* TC::FindStop(std::string_view name) const {
        auto it = stopname_to_stop_.find(name);
        if (it != stopname_to_stop_.end()) {
            return it->second;
        }
        return nullptr;
    }
    
    info::BusStop* TC::FindStop(std::string_view name) {
        auto it = stopname_to_stop_.find(name);
        if (it != stopname_to_stop_.end()) {
            return it->second;
        }
        return nullptr;
    }

    void TC::AddBus(std::string_view name, const std::vector<std::string_view>& stop_names) {
        buses_.push_back({ std::string(name), {} , {}});
        info::BusInfo& bus = buses_.back();

        bus.stops.reserve(stop_names.size());

        for (auto sv_name : stop_names) {
            bus.stops.emplace_back(sv_name);
            if (info::BusStop* stop = FindStop(sv_name)) {
                stop->buses.insert(bus.name);
            }
        }

        busname_to_bus_[bus.name] = &bus;
    }


    const info::BusInfo* TC::FindBus(std::string_view name) const {
        auto it = busname_to_bus_.find(name);
        if (it != busname_to_bus_.end()) {
            return it->second;
        }
        return nullptr;
    }

    std::optional<BusStat> TC::GetBusInfo(std::string_view request_name) const {
        const info::BusInfo* bus = FindBus(request_name);
        if (!bus) {
            return std::nullopt;
        }

        BusStat stat;
        stat.name = std::string(request_name);
        stat.stops_count = bus->stops.size();

        std::unordered_set<std::string_view> unique(bus->stops.begin(), bus->stops.end());
        stat.unique_stops_count = unique.size();

        double route_length = 0.0;
        double geography_length = 0.0;
        for (size_t i = 0; i + 1 < bus->stops.size(); ++i) {
            const auto* a = FindStop(bus->stops[i]);
            const auto* b = FindStop(bus->stops[i + 1]);
            if (a && b) {
                route_length += ComputeDistance(a->coordinates, b->coordinates);
                auto key = std::make_pair(a, b);
                auto it = distance_between_stops_.find(key);
                if (it != distance_between_stops_.end()) {
                    geography_length += it->second;
                } else {
                    auto it2 = distance_between_stops_.find({b, a});
                    if (it2 != distance_between_stops_.end()) {
                        geography_length += it2->second;
                    }
                }
            }
        }
        stat.route_length = geography_length;
        stat.curvature = geography_length / route_length;

        return stat;
    }


    std::optional<info::BusInfo> TC::GetStopInfo(std::string_view request_name) const {
        const info::BusStop* stop = FindStop(request_name);
        if (!stop) {
            return std::nullopt;
        }

        info::BusInfo stat;
        stat.name = stop->name;
        stat.buses = &stop->buses;
        return stat;
    }

    void TC::BuildDistanceIndex() {
        for (const auto& [from_name, to_name, dist] : pending_distances_) {
            info::BusStop* from = FindStop(from_name);
            info::BusStop* to = FindStop(to_name);
            if (from && to) {
                distance_between_stops_[{from, to}] = dist;
            }
        }
    }
}