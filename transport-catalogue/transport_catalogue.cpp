#include "transport_catalogue.h"

using namespace std;
using namespace transport_catalogue;

void TransportCatalogue::AddStop(Stop stop) {
    stops_.push_back(move(stop));
    const auto& inserted = stops_.back();
    stopname_to_stop_[inserted.name] = &inserted;
    passing_buses_.try_emplace(inserted.name, set<Bus*>{});
}

void TransportCatalogue::AddBus(Bus bus) {
    buses_.push_back(move(bus));
    auto& stored = buses_.back();
    busname_to_bus_[stored.name] = &stored;
    for (const auto* s : stored.route) {
        if (s) {
            passing_buses_[s->name].insert(&stored);
        }
    }
}

const Stop* TransportCatalogue::FindStop(string_view name) const {
    auto it = stopname_to_stop_.find(string(name));
    return it != stopname_to_stop_.end() ? it->second : nullptr;
}

const Bus* TransportCatalogue::FindBus(string_view name) const {
    auto it = busname_to_bus_.find(string(name));
    return it != busname_to_bus_.end() ? it->second : nullptr;
}

void TransportCatalogue::AddLength(pair<string_view, string_view> stops, int length) {
    const Stop* a = FindStop(stops.first);
    const Stop* b = FindStop(stops.second);
    if (!a || !b) return;
    distance_between_stops_[{a, b}] = length;
}

int TransportCatalogue::GetLength(string_view from, string_view to) const {
    const Stop* a = FindStop(from);
    const Stop* b = FindStop(to);
    if (!a || !b) return 0;

    auto it = distance_between_stops_.find({a, b});
    if (it != distance_between_stops_.end()) return it->second;

    auto it2 = distance_between_stops_.find({b, a});
    if (it2 != distance_between_stops_.end()) return it2->second;

    return 0;
}

const deque<Stop>& TransportCatalogue::GetStops() const {
    return stops_;
}

const deque<Bus>& TransportCatalogue::GetBuses() const {
    return buses_;
}

set<Bus*> TransportCatalogue::GetPassingBuses(string_view stop_name) const {
    auto it = passing_buses_.find(string(stop_name));
    return it != passing_buses_.end() ? it->second : set<Bus*>{};
}

const TransportCatalogue::BusNameToBusMap& TransportCatalogue::GetBusesToFind() const {
    return busname_to_bus_;
}
