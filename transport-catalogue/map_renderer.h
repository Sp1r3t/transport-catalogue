#pragma once

#include "svg.h"
#include "geo.h"

#include <cstdlib>
#include <iostream>
#include <optional>
#include <map>
#include <vector>
#include <string>
#include <utility>
#include <variant>
#include <algorithm>
#include <cmath>

struct RenderSettings {
    double width = 600;
    double height = 400;
    double padding = 50;
    double line_width = 14;
    double stop_radius = 5;
    int bus_label_font_size = 20;
    svg::Point bus_label_offset{7, 15};
    int stop_label_font_size = 12;
    svg::Point stop_label_offset{7, -3};
    double underlayer_width = 3.0;
    svg::Color underlayer_color = std::string("white");
    std::vector<svg::Color> color_palette;
};

inline const double EPSILON = 1e-6;
inline bool IsZero(double value) {
    return std::abs(value) < EPSILON;
}

class SphereProjector {
public:
    template <typename PointInputIt>
    SphereProjector(PointInputIt points_begin, PointInputIt points_end,
                    double max_width, double max_height, double padding)
        : padding_(padding)
    {
        if (points_begin == points_end) {
            return;
        }

        const auto [left_it, right_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });

        min_lon_ = left_it->lng;
        const double max_lon = right_it->lng;

        const auto [bottom_it, top_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
            
        const double min_lat = bottom_it->lat;
        max_lat_ = top_it->lat;

        std::optional<double> width_zoom;
        if (!IsZero(max_lon - min_lon_)) {
            width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
        }

        std::optional<double> height_zoom;
        if (!IsZero(max_lat_ - min_lat)) {
            height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
        }

        if (width_zoom && height_zoom) {
            zoom_coeff_ = std::min(*width_zoom, *height_zoom);
        } else if (width_zoom) {
            zoom_coeff_ = *width_zoom;
        } else if (height_zoom) {
            zoom_coeff_ = *height_zoom;
        }
    }

    svg::Point operator()(geo::Coordinates coords) const {
        return { (coords.lng - min_lon_) * zoom_coeff_ + padding_, (max_lat_ - coords.lat) * zoom_coeff_ + padding_};
    }

private:
    double padding_;
    double min_lon_ = 0;
    double max_lat_ = 0;
    double zoom_coeff_ = 0;
};

class MapRenderer {
public:
    using RenderingObjects = std::optional<std::pair<
        std::map<std::string, svg::Point>,
        std::map<std::string, std::pair<std::vector<svg::Point>, bool>>
    >>;

    MapRenderer() = default;
    explicit MapRenderer(RenderSettings settings) : settings_(std::move(settings)) {}

    MapRenderer& SetWidth(double w);
    MapRenderer& SetHeight(double h);
    MapRenderer& SetPadding(double p);
    MapRenderer& SetLineWidth(double lw);
    MapRenderer& SetStopRadius(double r);
    MapRenderer& SetBusLabelFontSize(int s);
    MapRenderer& SetBusLabelOffset(svg::Point p);
    MapRenderer& SetStopLabelFontSize(int s);
    MapRenderer& SetStopLabelOffset(svg::Point p);
    MapRenderer& SetUnderlayerWidth(double w);
    MapRenderer& SetUnderlayerColor(const svg::Color& c);
    MapRenderer& SetColorPalette(const std::vector<svg::Color>& pal);

    void AddBusLabel(std::vector<std::unique_ptr<svg::Object>>& out, const std::string& name, const svg::Point& pos, const svg::Color& color) const;


    double GetWidth() const;
    double GetHeight() const;
    double GetPadding() const;

    svg::Document RenderMap(RenderingObjects&& objects) const;

    void DrawBusRoute(const std::vector<svg::Point>& route, const svg::Color& color, std::vector<svg::Polyline>& out) const;
    void DrawBusName(const std::string& bus_name, const svg::Point& pos, const svg::Color& color, std::vector<svg::Text>& out) const;
    void DrawStopPoint(const svg::Point& pt, std::vector<svg::Circle>& out) const;
    void DrawStopName(const std::string& stop_name, const svg::Point& pos, std::vector<svg::Text>& out) const;

private:
    RenderSettings settings_;
};
