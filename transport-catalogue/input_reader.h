#pragma once
#include <string>
#include <string_view>
#include <vector>
#include <istream>

#include "geo.h"
#include "transport_catalogue.h"

struct CommandDescription {
    explicit operator bool() const {
        return !command.empty();
    }

    bool operator!() const {
        return !operator bool();
    }

    std::string command;
    std::string id;
    std::string description;
};

class InputReader {
public:
    /**
     * Парсит строку в структуру CommandDescription и сохраняет результат в commands_
     */
    void ParseLine(std::string_view line);

    /**
     * Наполняет данными транспортный справочник, используя команды из commands_
     */
    void ApplyCommands(transport_catalogue::TransportCatalogue& catalogue) const;

    void ReadBaseRequests(std::istream& in, transport_catalogue::TransportCatalogue& catalogue);

private:
    std::vector<CommandDescription> commands_;
};
