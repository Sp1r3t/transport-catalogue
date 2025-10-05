#include "stat_reader.h"
#include <string>
#include <iostream>
#include <algorithm>

void ParseAndPrintStat(const transport_catalogue::TransportCatalogue& catalogue,
                       std::string_view request,
                       std::ostream& output)
{
    std::string str(request);
    auto space_pos = str.find(' ');
    if (space_pos == std::string::npos) {
        return;
    }

    std::string type = str.substr(0, space_pos);
    std::string name = str.substr(space_pos + 1);

    if (type == "Bus") {
        auto bus_info = catalogue.GetBusInfo(name);
        if (!bus_info.found) {
            output << "Bus " << bus_info.name << ": not found\n";
        } else {
            output << "Bus " << bus_info.name
                   << ": " << bus_info.stops_count << " stops on route, "
                   << bus_info.unique_stops_count << " unique stops, "
                   << bus_info.route_length << " route length\n";
        }
    } else if (type == "Stop") {
        auto stop_info = catalogue.GetStopInfo(name);
        if (!stop_info.found) {
            output << "Stop " << stop_info.name << ": not found\n";
        } else if (stop_info.buses.empty()) {
            output << "Stop " << stop_info.name << ": no buses\n";
        } else {
            output << "Stop " << stop_info.name << ": buses";
            std::sort(stop_info.buses.begin(), stop_info.buses.end());
            for (const auto& bus_name : stop_info.buses) {
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
