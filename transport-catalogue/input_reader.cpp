#include "input_reader.h"

#include <algorithm>
#include <cassert>
#include <iterator>
#include <iostream>

/**
 * Парсит строку вида "10.123,  -30.1837" и возвращает пару координат (широта, долгота)
 */
Coordinates ParseCoordinates(std::string_view str) {
    static const double nan = std::nan("");

    auto not_space = str.find_first_not_of(' ');
    auto comma = str.find(',');

    if (comma == str.npos) {
        return {nan, nan};
    }

    auto not_space2 = str.find_first_not_of(' ', comma + 1);

    double lat = std::stod(std::string(str.substr(not_space, comma - not_space)));
    double lng = std::stod(std::string(str.substr(not_space2)));

    return {lat, lng};
}

/**
 * Удаляет пробелы в начале и конце строки
 */
std::string_view Trim(std::string_view string) {
    const auto start = string.find_first_not_of(' ');
    if (start == string.npos) {
        return {};
    }
    return string.substr(start, string.find_last_not_of(' ') + 1 - start);
}

/**
 * Разбивает строку string на n строк, с помощью указанного символа-разделителя delim
 */
std::vector<std::string_view> Split(std::string_view string, char delim) {
    std::vector<std::string_view> result;

    size_t pos = 0;
    while ((pos = string.find_first_not_of(' ', pos)) < string.length()) {
        auto delim_pos = string.find(delim, pos);
        if (delim_pos == string.npos) {
            delim_pos = string.size();
        }
        if (auto substr = Trim(string.substr(pos, delim_pos - pos)); !substr.empty()) {
            result.push_back(substr);
        }
        pos = delim_pos + 1;
    }

    return result;
}

/**
 * Парсит маршрут.
 * Для кольцевого маршрута (A>B>C>A) возвращает массив названий остановок [A,B,C,A]
 * Для некольцевого маршрута (A-B-C-D) возвращает массив названий остановок [A,B,C,D,C,B,A]
 */
std::vector<std::string_view> ParseRoute(std::string_view route) {
    if (route.find('>') != route.npos) {
        return Split(route, '>');
    }

    auto stops = Split(route, '-');
    std::vector<std::string_view> results(stops.begin(), stops.end());
    results.insert(results.end(), std::next(stops.rbegin()), stops.rend());

    return results;
}

static std::vector<info::Distance> ParseDistance(std::string_view line) {
    std::vector<info::Distance> result;

    size_t start = line.find_first_not_of(", ");
    if (start == std::string_view::npos) {
        return result;
    }
    line.remove_prefix(start);

    auto parts = Split(line, ',');
    for (auto part : parts) {
        part = Trim(part);
        if (part.empty()) continue;

        size_t m_pos = part.find('m');
        if (m_pos == std::string_view::npos) continue;

        auto num_sv = Trim(part.substr(0, m_pos));
        if (num_sv.empty()) continue;

        int distance = std::stoi(std::string(num_sv));

        size_t to_pos = part.find("to", m_pos);
        if (to_pos == std::string_view::npos) continue;

        auto stop_name_sv = Trim(part.substr(to_pos + 2));
        if (!stop_name_sv.empty()) {
            result.push_back({std::string(stop_name_sv), distance});
        }
    }

    return result;
}

CommandDescription ParseCommandDescription(std::string_view line) {
    auto colon_pos = line.find(':');
    if (colon_pos == line.npos) {
        return {};
    }

    auto space_pos = line.find(' ');
    if (space_pos >= colon_pos) {
        return {};
    }

    auto not_space = line.find_first_not_of(' ', space_pos);
    if (not_space >= colon_pos) {
        return {};
    }

    size_t first_pos = line.find(',');
    if (first_pos != std::string::npos) {
        size_t second_pos = line.find(',', first_pos + 1);
        if (second_pos != std::string::npos) {
            return {std::string(line.substr(0, space_pos)),
                    std::string(line.substr(not_space, colon_pos - not_space)),
                    std::string(line.substr(colon_pos + 1)),
                    ParseDistance(std::string(line.substr(second_pos + 2)))};
        }
    }

    return {std::string(line.substr(0, space_pos)),
            std::string(line.substr(not_space, colon_pos - not_space)),
            std::string(line.substr(colon_pos + 1)),
            {}};
}

void InputReader::ParseLine(std::string_view line) {
    auto command_description = ParseCommandDescription(line);
    if (command_description) {
        commands_.push_back(std::move(command_description));
    }
}

void InputReader::ApplyCommands(transport_catalogue::TransportCatalogue& catalogue) const {
    for (const CommandDescription& obj : commands_) {
        if (obj.command == "Stop") {
            catalogue.AddStop(obj.id, ParseCoordinates(obj.coordinates));
            catalogue.AddDistance(obj.id, const_cast<std::vector<info::Distance>&>(obj.distances));
        }
    }
    for (const CommandDescription& obj : commands_) {
        if (obj.command == "Bus") {
            catalogue.AddBus(obj.id, ParseRoute(obj.coordinates));
        }
    }

    catalogue.BuildDistanceIndex();
}

void InputReader::ReadBaseRequests(std::istream& in, transport_catalogue::TransportCatalogue& catalogue) {
    int base_request_count = 0;
    in >> base_request_count >> std::ws;
    for (int i = 0; i < base_request_count; ++i) {
        std::string line;
        std::getline(in, line);
        ParseLine(line);
    }
    ApplyCommands(catalogue);
}
