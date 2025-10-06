#include "stat_reader.h"

#include <string>
#include <string_view>
#include <iostream>
#include <algorithm>
#include <iomanip>

void ParseAndPrintStat( const transport_catalogue::TransportCatalogue& catalogue, 
    std::string_view request, std::ostream& output) {

    auto space_pos = request.find(' ');
    if (space_pos == std::string_view::npos) return;

    std::string_view type = request.substr(0, space_pos);
    std::string_view name_sv = request.substr(space_pos + 1);
    std::string name{ name_sv };

    if (type == "Bus") {
        auto bus_info_opt = catalogue.GetBusInfo(name);
        if (!bus_info_opt) {
            output << "Bus " << name << ": not found\n";
            return;
        }
        const auto& bus_info = *bus_info_opt;

        output << "Bus " << bus_info.name
            << ": " << bus_info.stops_count << " stops on route, "
            << bus_info.unique_stops_count << " unique stops, "
            << std::setprecision(6) << bus_info.route_length << " route length\n";
    }
    else if (type == "Stop") {
        auto stop_info_opt = catalogue.GetStopInfo(name);
        if (!stop_info_opt) {
            output << "Stop " << name << ": not found\n";
            return;
        }
        const auto& stop_info = *stop_info_opt;

        if (stop_info.buses->empty()) {
            output << "Stop " << stop_info.name << ": no buses\n";
        }
        else {
            output << "Stop " << stop_info.name << ": buses";
            std::vector<std::string> buses(stop_info.buses->begin(), stop_info.buses->end());
            std::sort(buses.begin(), buses.end());
            for (const auto& bus_name : buses) {
                output << " " << bus_name;
            }
            output << "\n";
        }
    }
}


void ProcessStatRequests(std::istream& in, std::ostream& out,
    const transport_catalogue::TransportCatalogue& catalogue) {
    int query_count = 0;
    in >> query_count;
    std::string dummy;
    std::getline(in, dummy);

    for (int i = 0; i < query_count; ++i) {
        std::string line;
        std::getline(in, line);
        if (!line.empty()) {
            ParseAndPrintStat(catalogue, line, out);
        }
    }
}
