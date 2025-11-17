#include "json_reader.h"
#include "map_renderer.h"
#include "transport_catalogue.h"

#include <iostream>

int main() {
    transport_catalogue::TransportCatalogue tc;
    MapRenderer renderer;
    jsonreader::JsonReader reader;

    reader.ReadData(std::cin);

    reader.SetCatalogueData(tc);
    reader.SetRendererData(renderer);

    reader.OutputStatRequests(tc, renderer, std::cout);

    return 0;
}
