#include "map_renderer.h"

#include <set>

using namespace std;

MapRenderer& MapRenderer::SetWidth(double w) {
    settings_.width = w;
    return *this;
}

MapRenderer& MapRenderer::SetHeight(double h) {
    settings_.height = h;
    return *this;
}

MapRenderer& MapRenderer::SetPadding(double p) {
    settings_.padding = p;
    return *this;
}

MapRenderer& MapRenderer::SetLineWidth(double lw) {
    settings_.line_width = lw;
    return *this;
}

MapRenderer& MapRenderer::SetStopRadius(double r) {
    settings_.stop_radius = r;
    return *this;
}

MapRenderer& MapRenderer::SetBusLabelFontSize(int s) {
    settings_.bus_label_font_size = s;
    return *this;
}

MapRenderer& MapRenderer::SetBusLabelOffset(svg::Point p) {
    settings_.bus_label_offset = p;
    return *this;
}

MapRenderer& MapRenderer::SetStopLabelFontSize(int s) {
    settings_.stop_label_font_size = s;
    return *this;
}

MapRenderer& MapRenderer::SetStopLabelOffset(svg::Point p) {
    settings_.stop_label_offset = p;
    return *this;
}

MapRenderer& MapRenderer::SetUnderlayerWidth(double w) {
    settings_.underlayer_width = w;
    return *this;
}

MapRenderer& MapRenderer::SetUnderlayerColor(const svg::Color& c) {
    settings_.underlayer_color = c;
    return *this;
}

MapRenderer& MapRenderer::SetColorPalette(const std::vector<svg::Color>& pal) {
    settings_.color_palette = pal;
    return *this;
}

double MapRenderer::GetWidth() const {
    return settings_.width;
}

double MapRenderer::GetHeight() const {
    return settings_.height;
}

double MapRenderer::GetPadding() const {
    return settings_.padding;
}

svg::Document MapRenderer::RenderMap(RenderingObjects&& objects) const {
    svg::Document doc;
    if (!objects.has_value()) return doc;

    const auto& [stops_to_draw, buses_to_draw] = *objects;

    vector<string> bus_names;
    bus_names.reserve(buses_to_draw.size());
    for (const auto& [name, _] : buses_to_draw) {
        bus_names.push_back(name);
    }
    sort(bus_names.begin(), bus_names.end());

    size_t color_index = 0;

    vector<unique_ptr<svg::Object>> polylines;
    vector<unique_ptr<svg::Object>> bus_labels;
    vector<unique_ptr<svg::Object>> stop_circles;
    vector<unique_ptr<svg::Object>> stop_labels;

    for (const string& bus_name : bus_names) {
        const auto& [points, is_roundtrip] = buses_to_draw.at(bus_name);
        if (points.empty()) continue;

        const svg::Color& color = settings_.color_palette[color_index % settings_.color_palette.size()];

        svg::Polyline pl;
        for (const svg::Point& pt : points) {
            pl.AddPoint(pt);
        }
        pl.SetFillColor("none")
          .SetStrokeColor(color)
          .SetStrokeWidth(settings_.line_width)
          .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
          .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

        polylines.push_back(make_unique<svg::Polyline>(std::move(pl)));

        AddBusLabel(bus_labels, bus_name, points.front(), color);

        if (!is_roundtrip) {
            size_t mid = (points.size() + 1) / 2;
            const svg::Point& end = points[mid - 1];
            const svg::Point& start = points.front();
            if (end.x != start.x || end.y != start.y) {
                AddBusLabel(bus_labels, bus_name, end, color);
            }
        }

        ++color_index;
    }

    for (const auto& [stop_name, pt] : stops_to_draw) {
        svg::Circle circle;
        circle.SetCenter(pt)
              .SetRadius(settings_.stop_radius)
              .SetFillColor("white");
        stop_circles.push_back(make_unique<svg::Circle>(std::move(circle)));

        svg::Text under;
        under.SetPosition(pt)
             .SetOffset(settings_.stop_label_offset)
             .SetFontSize(settings_.stop_label_font_size)
             .SetFontFamily("Verdana")
             .SetData(stop_name)
             .SetFillColor(settings_.underlayer_color)
             .SetStrokeColor(settings_.underlayer_color)
             .SetStrokeWidth(settings_.underlayer_width)
             .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
             .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

        svg::Text label;
        label.SetPosition(pt)
             .SetOffset(settings_.stop_label_offset)
             .SetFontSize(settings_.stop_label_font_size)
             .SetFontFamily("Verdana")
             .SetData(stop_name)
             .SetFillColor("black");

        stop_labels.push_back(make_unique<svg::Text>(std::move(under)));
        stop_labels.push_back(make_unique<svg::Text>(std::move(label)));
    }

    for (auto& obj : polylines) {
        doc.Add(std::move(obj));
    }

    for (auto& obj : bus_labels) {
        doc.Add(std::move(obj));
    }

    for (auto& obj : stop_circles) {
        doc.Add(std::move(obj));
    }

    for (auto& obj : stop_labels) {
        doc.Add(std::move(obj));
    }
    return doc;
}

void MapRenderer::AddBusLabel(vector<unique_ptr<svg::Object>>& out, const string& name, const svg::Point& pos, const svg::Color& color) const {
    svg::Text under;
    under.SetPosition(pos)
         .SetOffset(settings_.bus_label_offset)
         .SetFontSize(settings_.bus_label_font_size)
         .SetFontFamily("Verdana")
         .SetFontWeight("bold")
         .SetData(name)
         .SetFillColor(settings_.underlayer_color)
         .SetStrokeColor(settings_.underlayer_color)
         .SetStrokeWidth(settings_.underlayer_width)
         .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
         .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

    svg::Text label;
    label.SetPosition(pos)
         .SetOffset(settings_.bus_label_offset)
         .SetFontSize(settings_.bus_label_font_size)
         .SetFontFamily("Verdana")
         .SetFontWeight("bold")
         .SetData(name)
         .SetFillColor(color);

    out.push_back(make_unique<svg::Text>(std::move(under)));
    out.push_back(make_unique<svg::Text>(std::move(label)));
}