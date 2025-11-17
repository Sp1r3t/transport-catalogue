#include "svg.h"
#include <sstream>

namespace svg {
    std::string RenderColor(const Color& c) {
        if (std::holds_alternative<std::string>(c)) {
            return std::get<std::string>(c);
        } else if (std::holds_alternative<Rgb>(c)) {
            const auto& v = std::get<Rgb>(c);
            return "rgb(" + std::to_string(v.r) + "," + std::to_string(v.g) + "," +
                std::to_string(v.b) + ")";
        } else {
            const auto& v = std::get<Rgba>(c);
            std::ostringstream ss;
            ss << "rgba(" << v.r << "," << v.g << "," << v.b << "," << v.a << ")";
            return ss.str();
        }
    }

    void Document::Add(std::unique_ptr<Object> obj) {
        objects_.push_back(std::move(obj));
    }

    void Document::Render(std::ostream& out) const {
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n";
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n";
        for (const auto& o : objects_) {
            o->Render(out);
        }
        out << "</svg>\n";
    }

    static std::string LineCapToStr(StrokeLineCap lc) {
        switch (lc) {
            case StrokeLineCap::BUTT: return "butt";
            case StrokeLineCap::ROUND: return "round";
            case StrokeLineCap::SQUARE: return "square";
        }
        return "butt";
    }

    static std::string LineJoinToStr(StrokeLineJoin lj) {
        switch (lj) {
            case StrokeLineJoin::ARCS: return "arcs";
            case StrokeLineJoin::BEVEL: return "bevel";
            case StrokeLineJoin::MITER: return "miter";
            case StrokeLineJoin::MITER_CLIP: return "miter-clip";
            case StrokeLineJoin::ROUND: return "round";
        }
        return "miter";
    }

    Polyline& Polyline::AddPoint(Point p) { 
        pts_.push_back(p);
        return *this;

    }

    Polyline& Polyline::SetStrokeWidth(double w) { 
        stroke_width_ = w; 
        return *this;
    }

    Polyline& Polyline::SetStrokeLineCap(StrokeLineCap lc) { 
        linecap_ = lc;
        return *this; 
    }

    Polyline& Polyline::SetStrokeLineJoin(StrokeLineJoin lj) { 
        linejoin_ = lj;
        return *this; 
    }

    Polyline& Polyline::SetStrokeColor(Color c) { 
        stroke_color_ = c;
        return *this; 
    }

    Polyline& Polyline::SetFillColor(Color c) { 
        fill_color_ = c;
        return *this; 
    }
    

    void Polyline::Render(std::ostream& out) const {
        out << "  <polyline points=\"";
        bool first = true;
        for (const auto& p : pts_) {
            if (!first) out << " ";
            first = false;
            out << p.x << "," << p.y;
        }
        out << "\" fill=\"" << RenderColor(fill_color_) << "\""
            << " stroke=\"" << RenderColor(stroke_color_) << "\""
            << " stroke-width=\"" << stroke_width_ << "\""
            << " stroke-linecap=\"" << LineCapToStr(linecap_) << "\""
            << " stroke-linejoin=\"" << LineJoinToStr(linejoin_) << "\""
            << "/>\n";
    }

    void Text::Render(std::ostream& out) const {
        out << "  <text";

        out << " fill=\"" << RenderColor(fill_color_) << "\"";

        if (stroke_width_ > 0.0) {
            out << " stroke=\"" << RenderColor(stroke_color_) << "\""
                << " stroke-width=\"" << stroke_width_ << "\""
                << " stroke-linecap=\"" << LineCapToStr(linecap_) << "\""
                << " stroke-linejoin=\"" << LineJoinToStr(linejoin_) << "\"";
        }

        out << " x=\"" << pos_.x << "\"";
        out << " y=\"" << pos_.y << "\"";

        if (offset_.x != 0 || offset_.y != 0) {
            out << " dx=\"" << offset_.x << "\""
                << " dy=\"" << offset_.y << "\"";
        }

        if (font_size_ > 0) {
            out << " font-size=\"" << font_size_ << "\"";
        }

        if (!font_family_.empty()) {
            out << " font-family=\"" << font_family_ << "\"";
        }

        if (!font_weight_.empty()) {
            out << " font-weight=\"" << font_weight_ << "\"";
        }

        out << ">";

        for (char c : data_) {
            switch (c) {
                case '<':  out << "&lt;"; break;
                case '>':  out << "&gt;"; break;
                case '&':  out << "&amp;"; break;
                case '"':  out << "&quot;"; break;
                case '\'': out << "&apos;"; break;
                default: out << c;
            }
        }

        out << "</text>\n";
    }

    Text& Text::SetPosition(Point p) {
        pos_ = p;
        return *this;
    }

    Text& Text::SetOffset(Point p) {
        offset_ = p;
        return *this;
    }

    Text& Text::SetFontSize(int size) {
        font_size_ = size;
        return *this;
    }

    Text& Text::SetFontFamily(const std::string& family) {
        font_family_ = family;
        return *this;
    }

    Text& Text::SetFontWeight(const std::string& weight) {
        font_weight_ = weight;
        return *this;
    }

    Text& Text::SetStrokeColor(Color c) {
        stroke_color_ = c;
        return *this;
    }

    Text& Text::SetStrokeWidth(double w) {
        stroke_width_ = w;
        return *this;
    }

    Text& Text::SetStrokeLineCap(StrokeLineCap lc) {
        linecap_ = lc;
        return *this;
    }

    Text& Text::SetStrokeLineJoin(StrokeLineJoin lj) {
        linejoin_ = lj;
        return *this;
    }

    Text& Text::SetFillColor(Color c) {
        fill_color_ = c;
        return *this;
    }

    Text& Text::SetData(const std::string& d) {
        data_ = d;
        return *this;
    }

    void Circle::Render(std::ostream& out) const {
        out << "  <circle cx=\"" << center_.x << "\" cy=\"" << center_.y
            << "\" r=\"" << r_ << "\" fill=\"" << RenderColor(fill_color_) << "\"";

        if (stroke_width_ > 0.0) {
            out << " stroke=\"" << RenderColor(stroke_color_) << "\""
                << " stroke-width=\"" << stroke_width_ << "\"";
        }

        out << "/>\n";
    }

    Circle& Circle::SetCenter(Point c) {
        center_ = c;
        return *this;
    }

    Circle& Circle::SetRadius(double r) {
        r_ = r;
        return *this;
    }

    Circle& Circle::SetFillColor(Color c) {
        fill_color_ = c;
        return *this;
    }

    Circle& Circle::SetStrokeColor(Color c) {
        stroke_color_ = c;
        return *this;
    }

    Circle& Circle::SetStrokeWidth(double w) {
        stroke_width_ = w;
        return *this;
    }

}