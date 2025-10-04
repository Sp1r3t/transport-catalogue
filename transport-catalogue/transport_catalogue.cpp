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

    info::BusStop* TC::FindStop(std::string_view name) const {
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
                if (std::find(stop->buses.begin(), stop->buses.end(), std::string(bus.name)) == stop->buses.end()) {
                    stop->buses.push_back(bus.name);
                }
            }
        }

        busname_to_bus_[bus.name] = &bus;
    }


    info::BusInfo* TC::FindBus(std::string_view name) const {
        auto it = busname_to_bus_.find(name);
        if (it != busname_to_bus_.end()) {
            return it->second;
        }
        const std::string_view prefix = "Bus ";
        if (name.size() > prefix.size() && name.substr(0, prefix.size()) == prefix) {
            auto without = name.substr(prefix.size());
            auto it = busname_to_bus_.find(without);
            if (it != busname_to_bus_.end()) {
                return it->second;
            }
        }
        return nullptr;
    }

    std::string TC::GetBusInfo(std::string_view request_name) const {
        info::BusInfo* bus = FindBus(request_name);
        if (!bus) {
            return "Bus " + std::string(request_name) + ": not found";
        }

        const auto& stops_vec = bus->stops;
        size_t total_stops = stops_vec.size();

        std::unordered_set<std::string_view> unique_stops(stops_vec.begin(), stops_vec.end());

        double route_length = 0.0;
        if (total_stops >= 2) {
            for (size_t i = 0; i + 1 < total_stops; ++i) {
                const info::BusStop* a = FindStop(stops_vec[i]);
                const info::BusStop* b = FindStop(stops_vec[i + 1]);
                if (a && b) {
                    route_length += ComputeDistance(a->coordinates, b->coordinates);
                }
            }
        }

        return "Bus " + std::string(request_name) + ": " +
            std::to_string(total_stops) + " stops on route, " +
            std::to_string(unique_stops.size()) + " unique stops, " +
            std::to_string(route_length) + " route length";
    }

    std::string TC::GetStopInfo(std::string_view request_name) const {

        info::BusStop* stop = FindStop(request_name);
        if (!stop) {
            return "Stop " + std::string(request_name) + ": not found";
        }

        if (stop->buses.empty()) {
            return "Stop " + std::string(request_name) + ": no buses";
        }

        std::set<std::string> sorted_buses(stop->buses.begin(), stop->buses.end());

        std::string result = "Stop " + std::string(request_name) + ": buses";
        for (const auto& bus : sorted_buses) {
            result += " " + bus;
        }
        return result;
    }
}