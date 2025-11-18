#pragma once
#include "geo.h"

#include <string>
#include <vector>

struct Stop {
    std::string name;
    geo::Coordinates coord;
};

struct Bus {
    std::string name;
    std::vector<const Stop*> route;
    bool is_round_trip = false;
};

struct StopInfo {
    std::string name;
    std::vector<const Bus*> buses;
};

struct BusInfo {
    std::string name;
    int num_stops = 0;
    int uniq_stops = 0;
    int length_route = 0;
    double curvature = 0.0;
};