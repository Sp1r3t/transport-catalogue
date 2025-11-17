#pragma once

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "geo.h"

#include <optional>
#include <string>
#include <vector>

struct StopInfo {
    std::string name;
    std::vector<const transport_catalogue::Bus*> buses;
};

struct BusInfo {
    std::string name;
    int num_stops = 0;
    int uniq_stops = 0;
    int length_route = 0;
    double curvature = 0.0;
};

class RequestHandler {
public:
    using RenderingObjects = MapRenderer::RenderingObjects;

    RequestHandler(const transport_catalogue::TransportCatalogue& tc, const MapRenderer& mr)
        : tc_(tc), mr_(mr) {}

    std::optional<StopInfo> GetStopInfo(std::string_view stop_name) const;
    std::optional<BusInfo> GetBusInfo(std::string_view bus_name) const;

    RenderingObjects GetRenderingObjects() const;

    double ComputeDistance(const geo::Coordinates& a, const geo::Coordinates& b) const;

private:
    const transport_catalogue::TransportCatalogue& tc_;
    const MapRenderer& mr_;
};
