#pragma once

#include "geo.h"

#include <string>
#include <string_view>
#include <deque>
#include <vector>
#include <unordered_map>
#include <map>
#include <set>

namespace info {
    struct Stop {
        std::string name;
        geo::Coordinates coord;
    };

    struct Bus {
        std::string name;
        std::vector<const Stop*> route;
        bool is_round_trip = false;
    };
}

namespace transport_catalogue {

    using Stop = info::Stop;
    using Bus = info::Bus;

    struct StringViewHasher {
        using is_transparent = void;
        size_t operator()(std::string_view sv) const noexcept {
            return std::hash<std::string_view>{}(sv);
        }
        size_t operator()(const std::string& s) const noexcept {
            return std::hash<std::string>{}(s);
        }
    };

    struct StopPtrPairHasher {
        size_t operator()(const std::pair<const Stop*, const Stop*>& p) const noexcept {
            auto h1 = reinterpret_cast<size_t>(p.first);
            auto h2 = reinterpret_cast<size_t>(p.second);
            return h1 * 17 + h2 * 17 * 17;
        }
    };

    class TransportCatalogue {
    public:
        void AddStop(Stop stop);
        void AddBus(Bus bus);

        const Stop* FindStop(std::string_view name) const;
        const Bus* FindBus(std::string_view name) const;

        void AddLength(std::pair<std::string_view, std::string_view> stops, int length);
        int GetLength(std::string_view from, std::string_view to) const;

        const std::deque<Stop>& GetStops() const;
        const std::deque<Bus>& GetBuses() const;

        std::set<Bus*> GetPassingBuses(std::string_view stop_name) const;

        using BusNameToBusMap = std::unordered_map<std::string, const Bus*, StringViewHasher>;
        const BusNameToBusMap& GetBusesToFind() const;

    private:
        std::deque<Stop> stops_;
        std::deque<Bus> buses_;

        std::unordered_map<std::string, const Stop*, StringViewHasher> stopname_to_stop_;
        BusNameToBusMap busname_to_bus_;

        std::unordered_map<std::pair<const Stop*, const Stop*>, int, StopPtrPairHasher> distance_between_stops_;
        std::unordered_map<std::string, std::set<Bus*>, StringViewHasher> passing_buses_;
    };
}