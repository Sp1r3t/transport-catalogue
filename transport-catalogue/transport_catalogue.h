#pragma once

#include "geo.h"

#include <string_view>
#include <vector>
#include <deque>
#include <string>
#include <algorithm>
#include <unordered_map>

namespace info {
    struct BusStop {
        std::string name;
        Coordinates coordinates;
        std::vector<std::string> buses;
    };

    struct BusInfo {
        std::string name;
        std::vector<std::string> stops;
    };
}

namespace transport_catalogue {
    class TransportCatalogue {
    public:
        void AddStop(std::string_view name, Coordinates coordinates);
        info::BusStop* FindStop(std::string_view name) const;
        void AddBus(std::string_view name, const std::vector<std::string_view>& stops);
        info::BusInfo* FindBus(std::string_view name) const;
        std::string GetBusInfo(std::string_view request_name) const;
        std::string GetStopInfo(std::string_view request_name) const;
    private:
        std::deque<info::BusStop> stops_;
        std::deque<info::BusInfo> buses_;
        std::unordered_map<std::string_view, info::BusStop*> stopname_to_stop_;
        std::unordered_map<std::string_view, info::BusInfo*> busname_to_bus_;
    };
}