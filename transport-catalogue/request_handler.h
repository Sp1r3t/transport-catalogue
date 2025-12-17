#pragma once
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"
#include <optional>
#include <string>
#include <string_view>

class RequestHandler {
public:
    RequestHandler(const transport_catalogue::TransportCatalogue& db, const MapRenderer& renderer)
        : tc_(db), mr_(renderer) {
    }
    
    void SetRoutingSettings(transport_router::RoutingSettings settings) {
        router_ = std::make_unique<transport_router::TransportRouter>(tc_, settings);
    }

    std::optional<StopInfo> GetStopInfo(std::string_view stop_name) const;

    using RenderingObjects = std::optional<std::pair<std::map<std::string, svg::Point>, std::map<std::string, std::pair<std::vector<svg::Point>, bool>>>>;
    RenderingObjects GetRenderingObjects() const;
    double ComputeDistance(const geo::Coordinates& a, const geo::Coordinates& b) const;

    std::optional<json::Node> GetRoute(const std::string& from, const std::string& to, int id) const;

private:
    const transport_catalogue::TransportCatalogue& tc_;
    const MapRenderer& mr_;
    std::unique_ptr<transport_router::TransportRouter> router_;
};

