#include "stat_reader.h"

#include <iostream>

void ParseAndPrintStat(const transport_catalogue::TransportCatalogue& tansport_catalogue, std::string_view request, std::ostream& output) {
    std::string str = std::string(request);
    auto space_pos = str.find(' ');
    if (str.find("Stop") != std::string::npos) {
        std::string new_str = std::string(str.substr(space_pos + 1));
        std::string_view sv_new_str = new_str;
        output << tansport_catalogue.GetStopInfo(sv_new_str) << std::endl;
    }
    if (str.find("Bus") != std::string::npos) {
        std::string new_str = std::string(str.substr(space_pos + 1));
        std::string_view sv_new_str = new_str;
        output << tansport_catalogue.GetBusInfo(sv_new_str) << std::endl;
    }

}