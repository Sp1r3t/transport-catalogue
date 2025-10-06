#include "transport_catalogue.h"

#include <unordered_set>
#include <set>
#include <cmath>
#include <iostream>

namespace transport_catalogue {
    using TC = TransportCatalogue;

    void TC::AddStop(std::string_view name, Coordinates coordinates) {
        stops_.push_back({ std::string(name), coordinates , {}});
        stopname_to_stop_[stops_.back().name] = &stops_.back();
    }

    const info::BusStop* TC::FindStop(std::string_view name) const {
        auto it = stopname_to_stop_.find(name);
        if (it != stopname_to_stop_.end()) {
            return it->second;
        }
        return nullptr;
    }
    //Это перегрузка для FindStop и она специально возращает не конcтанту, 
    //так как в строке 41 я делаю insert и const мне этому помешает.
    info::BusStop* TC::FindStop(std::string_view name) {
        auto it = stopname_to_stop_.find(name);
        if (it != stopname_to_stop_.end()) {
            return it->second;
        }
        return nullptr;
    }

    void TC::AddBus(std::string_view name, const std::vector<std::string_view>& stop_names) {
        buses_.push_back({ std::string(name), {} });
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
        for (size_t i = 0; i + 1 < bus->stops.size(); ++i) {
            const auto* a = FindStop(bus->stops[i]);
            const auto* b = FindStop(bus->stops[i + 1]);
            if (a && b) {
                route_length += ComputeDistance(a->coordinates, b->coordinates);
            }
        }
        stat.route_length = route_length;

        return stat;
    }


    std::optional<StopStat> TC::GetStopInfo(std::string_view request_name) const {
        const info::BusStop* stop = FindStop(request_name);
        if (!stop) {
            return std::nullopt;
        }

        StopStat stat;
        stat.name = stop->name;
        stat.buses = &stop->buses;
        return stat;
    }
}