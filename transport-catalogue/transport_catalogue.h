#pragma once

#include "geo.h"

#include <string_view>
#include <vector>
#include <deque>
#include <string>
#include <algorithm>
#include <optional>
#include <set>
#include <unordered_map>
#include <unordered_set>

namespace info {

    struct BusStop {
        std::string name;
        Coordinates coordinates;
        std::set<std::string> buses;
    };

    struct BusInfo {
        std::string name;
        std::vector<std::string> stops;
        const std::set<std::string>* buses;
    };

    struct Distance {
        std::string name;
        int length;
    };

    struct StopPairHasher {
        std::size_t operator()(const std::pair<const BusStop*, const BusStop*>& p) const noexcept {
            auto h1 = std::hash<const void*>{}(p.first);
            auto h2 = std::hash<const void*>{}(p.second);
            return h1 * 31 + h2 * 17;
        }
    };
}

namespace transport_catalogue {
    struct BusStat {
        std::string name;
        size_t stops_count = 0;
        size_t unique_stops_count = 0;
        double route_length = 0.0;
        double curvature = 0.0;
    };

    class TransportCatalogue {
    public:
        void AddStop(std::string_view name, Coordinates coordinates);
        void AddDistance(std::string_view name1, std::string_view name2, int distance);
        const info::BusStop* FindStop(std::string_view name) const;
        info::BusStop* FindStop(std::string_view name);
        void AddBus(std::string_view name, const std::vector<std::string_view>& stops);
        const info::BusInfo* FindBus(std::string_view name) const;
        std::optional <BusStat> GetBusInfo(std::string_view request_name) const;
        std::optional<info::BusInfo> GetStopInfo(std::string_view request_name) const;
        void BuildDistanceIndex();
    private:
        std::deque<info::BusStop> stops_;
        std::deque<info::BusInfo> buses_;
        std::deque<info::Distance> distance_;
        std::deque<std::tuple<std::string, std::string, int>> pending_distances_;
        std::unordered_map<std::pair<const info::BusStop*, const info::BusStop*>, int, info::StopPairHasher> distance_between_stops_;
        std::unordered_map<std::string_view, info::BusStop*> stopname_to_stop_;
        std::unordered_map<std::string_view, info::BusInfo*> busname_to_bus_;
    };
}