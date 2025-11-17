#pragma once

#include <string>
#include <variant>
#include <vector>
#include <memory>
#include <ostream>

namespace svg {
    struct Rgb {
        int r, g, b;
    };

    struct Rgba {
        int r, g, b;
        double a;
    };

    using Color = std::variant<std::string, Rgb, Rgba>;

    struct Point {
        double x = 0;
        double y = 0;
    };

    enum class StrokeLineCap { BUTT, ROUND, SQUARE };
    enum class StrokeLineJoin { ARCS, BEVEL, MITER, MITER_CLIP, ROUND };

    class Object {
    public:
        virtual void Render(std::ostream& out) const = 0;
        virtual ~Object() = default;
    };

    class Polyline : public Object {
    public:
        Polyline& AddPoint(Point p);
        Polyline& SetStrokeWidth(double w);
        Polyline& SetStrokeLineCap(StrokeLineCap lc);
        Polyline& SetStrokeLineJoin(StrokeLineJoin lj);
        Polyline& SetStrokeColor(Color c);
        Polyline& SetFillColor(Color c);

        void Render(std::ostream& out) const override;

    private:
        std::vector<Point> pts_;
        double stroke_width_ = 1.0;
        StrokeLineCap linecap_ = StrokeLineCap::BUTT;
        StrokeLineJoin linejoin_ = StrokeLineJoin::MITER;
        Color stroke_color_ = "black";
        Color fill_color_ = "none";
    };

    class Text : public Object {
    public:
        Text& SetPosition(Point p);
        Text& SetOffset(Point p);
        Text& SetFontSize(int size);
        Text& SetFontFamily(const std::string& family);
        Text& SetFontWeight(const std::string& weight);
        Text& SetStrokeColor(Color c);
        Text& SetStrokeWidth(double w);
        Text& SetStrokeLineCap(StrokeLineCap lc);
        Text& SetStrokeLineJoin(StrokeLineJoin lj);
        Text& SetFillColor(Color c);
        Text& SetData(const std::string& d);

        void Render(std::ostream& out) const override;

    private:
        Point pos_;
        Point offset_{0,0};
        int font_size_ = 0;
        std::string font_family_;
        std::string font_weight_;
        double stroke_width_ = 0.0;
        StrokeLineCap linecap_ = StrokeLineCap::BUTT;
        StrokeLineJoin linejoin_ = StrokeLineJoin::MITER;
        Color stroke_color_ = "none";
        Color fill_color_ = "black";
        std::string data_;
    };

    class Circle : public Object {
    public:
        Circle& SetCenter(Point c);
        Circle& SetRadius(double r);
        Circle& SetFillColor(Color c);
        Circle& SetStrokeColor(Color c);
        Circle& SetStrokeWidth(double w);

        void Render(std::ostream& out) const override;

    private:
        Point center_;
        double r_ = 1.0;
        Color fill_color_ = "black";
        Color stroke_color_ = "none";
        double stroke_width_ = 0.0;
    };

    class Document {
    public:
        void Add(std::unique_ptr<Object> obj);
        void Render(std::ostream& out) const;

    private:
        std::vector<std::unique_ptr<Object>> objects_;
    };

    std::string RenderColor(const Color& c);

}