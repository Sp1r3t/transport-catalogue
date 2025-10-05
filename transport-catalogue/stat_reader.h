#pragma once

#include <iosfwd>
#include <string_view>
#include <string>

#include "transport_catalogue.h"

void ParseAndPrintStat(const transport_catalogue::TransportCatalogue&  transport_catalogue, std::string_view request, std::ostream& output);

void ProcessStatRequests(std::istream& in, std::ostream& out, const transport_catalogue::TransportCatalogue& transport_catalogue);