#pragma once

#include "json.h"
#include "map_renderer.h"
#include "transport_catalogue.h"
#include "request_handler.h"

#include <iostream>

namespace jsonreader {

class JsonReader {
public:
    json::Document ReadData(std::istream& input);

    void SetCatalogueData(transport_catalogue::TransportCatalogue& tc);

    void SetRendererData(MapRenderer& map_rend);

    void OutputStatRequests(const transport_catalogue::TransportCatalogue& tc, const MapRenderer& map_rend, std::ostream& output);

private:
    svg::Color GetJsonColor(const json::Node& color) const;

    json::Document document_json_;
};

}