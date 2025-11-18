#pragma once

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "geo.h"

#include <optional>
#include <string>
#include <vector>

class RequestHandler {
public:
    using RenderingObjects = MapRenderer::RenderingObjects;

    RequestHandler(const transport_catalogue::TransportCatalogue& tc, const MapRenderer& mr)
        : tc_(tc), mr_(mr) {}

    std::optional<StopInfo> GetStopInfo(std::string_view stop_name) const;

    RenderingObjects GetRenderingObjects() const;

    double ComputeDistance(const geo::Coordinates& a, const geo::Coordinates& b) const;

private:
    const transport_catalogue::TransportCatalogue& tc_;
    const MapRenderer& mr_;
};
