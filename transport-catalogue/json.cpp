#include "json.h"

#include <variant>
#include <optional>
#include <string>
#include <istream>
#include <cctype>

using namespace std;

namespace json {

namespace {

    using Number = variant<int, double>;

    Node LoadNode(istream& input);

    Node LoadString(istream& input) {
        string result;
        char c;
        while (input.get(c)) {
            if (c == '"') break;
            if (c == '\\') {
                if (!input.get(c)) throw ParsingError("Bad escape");
                switch (c) {
                    case 'n': result.push_back('\n'); break;
                    case 'r': result.push_back('\r'); break;
                    case 't': result.push_back('\t'); break;
                    case '"': result.push_back('"'); break;
                    case '\\': result.push_back('\\'); break;
                    default: throw ParsingError("Unknown escape");
                }
            } else {
                result.push_back(c);
            }
        }
        return Node(result);
    }

    Number ParseNumber(istream& input) {
        string s;
        if (input.peek() == '-') s.push_back(static_cast<char>(input.get()));
        if (!isdigit(input.peek())) throw ParsingError("Number expected");
        if (input.peek() == '0') {
            s.push_back(static_cast<char>(input.get()));
        } else {
            while (isdigit(input.peek())) s.push_back(static_cast<char>(input.get()));
        }
        bool is_int = true;
        if (input.peek() == '.') {
            is_int = false;
            s.push_back(static_cast<char>(input.get()));
            while (isdigit(input.peek())) s.push_back(static_cast<char>(input.get()));
        }
        if (input.peek() == 'e' || input.peek() == 'E') {
            is_int = false;
            s.push_back(static_cast<char>(input.get()));
            if (input.peek() == '+' || input.peek() == '-') s.push_back(static_cast<char>(input.get()));
            while (isdigit(input.peek())) s.push_back(static_cast<char>(input.get()));
        }

        try {
            if (is_int) return stoi(s);
            return stod(s);
        } catch (...) {
            throw ParsingError("Invalid number: " + s);
        }
    }

    Node LoadArray(istream& input) {
        Array arr;
        char c;
        while (input >> c && c != ']') {
            if (c != ',') input.putback(c);
            arr.push_back(LoadNode(input));
        }
        return Node(move(arr));
    }

    Node LoadDict(istream& input) {
        Dict dict;
        char c;
        while (input >> c && c != '}') {
            if (c == ',') input >> c;
            if (c != '"') throw ParsingError("Key expected");
            Node key_node = LoadString(input);
            string key = key_node.AsString();
            input >> c;
            dict.emplace(move(key), LoadNode(input));
        }
        return Node(move(dict));
    }

    Node LoadBoolOrNull(istream& input) {
        string word;
        while (isalpha(input.peek())) word.push_back(static_cast<char>(input.get()));
        if (word == "true") return Node(true);
        if (word == "false") return Node(false);
        if (word == "null") return Node(nullptr);
        throw ParsingError("Unknown literal: " + word);
    }

    Node LoadNode(istream& input) {
        while (isspace(input.peek())) input.get();
        char c;
        input.get(c);
        if (c == '[') return LoadArray(input);
        if (c == '{') return LoadDict(input);
        if (c == '"') return LoadString(input);
        if (isdigit(c) || c == '-') {
            input.putback(c);
            auto num = ParseNumber(input);
            if (holds_alternative<int>(num)) return Node(get<int>(num));
            return Node(get<double>(num));
        }
        input.putback(c);
        return LoadBoolOrNull(input);
    }
}

void PrintNode::operator()(std::nullptr_t) const { out << "null"; }

void PrintNode::operator()(const Array& arr) const {
    out << "[";
    bool first = true;
    for (const auto& v : arr) {
        if (!first) out << ",";
        first = false;
        visit(PrintNode{out}, v.GetValue());
    }
    out << "]";
}

void PrintNode::operator()(const Dict& dict) const {
    out << "{";
    bool first = true;
    for (const auto& [key, value] : dict) {
        if (!first) out << ",";
        first = false;
        out << "\"" << key << "\":";
        visit(PrintNode{out}, value.GetValue());
    }
    out << "}";
}

void PrintNode::operator()(bool b) const {
    out << (b ? "true" : "false");
}

void PrintNode::operator()(int v) const {
    out << v;
}

void PrintNode::operator()(double v) const {
    out << v;
}

void PrintNode::operator()(const std::string& s) const {
    out << "\"";
    for (char c : s) {
        switch (c) {
            case '"': out << "\\\""; break;
            case '\\': out << "\\\\"; break;
            case '\n': out << "\\n"; break;
            case '\t': out << "\\t"; break;
            case '\r': out << "\\r"; break;
            default: out << c;
        }
    }
    out << "\"";
}

Node::Node() : type_(nullptr) {}
Node::Node(std::nullptr_t) : type_(nullptr) {}
Node::Node(Array arr) : type_(move(arr)) {}
Node::Node(Dict map) : type_(move(map)) {}
Node::Node(double value) : type_(value) {}
Node::Node(bool value) : type_(value) {}
Node::Node(int value) : type_(value) {}
Node::Node(const string& value) : type_(value) {}
Node::Node(string&& value) : type_(move(value)) {}
Node::Node(const char* value) : type_(string(value)) {}

bool Node::IsInt() const { return holds_alternative<int>(type_); }
bool Node::IsDouble() const { return IsPureDouble() || IsInt(); }
bool Node::IsPureDouble() const { return holds_alternative<double>(type_); }
bool Node::IsBool() const { return holds_alternative<bool>(type_); }
bool Node::IsString() const { return holds_alternative<string>(type_); }
bool Node::IsNull() const { return holds_alternative<nullptr_t>(type_); }
bool Node::IsArray() const { return holds_alternative<Array>(type_); }
bool Node::IsMap() const { return holds_alternative<Dict>(type_); }

const Node::JsonType& Node::GetValue() const { return type_; }

int Node::AsInt() const { return get<int>(type_); }
bool Node::AsBool() const { return get<bool>(type_); }
double Node::AsDouble() const { return IsInt() ? static_cast<double>(get<int>(type_)) : get<double>(type_); }
const string& Node::AsString() const { return get<string>(type_); }
const Array& Node::AsArray() const { return get<Array>(type_); }
const Dict& Node::AsMap() const { return get<Dict>(type_); }

bool Node::operator==(const Node& other) const { return type_ == other.type_; }
bool Node::operator!=(const Node& other) const { return !(*this == other); }

Document::Document(Node root) : root_(move(root)) {}
const Node& Document::GetRoot() const { return root_; }
bool Document::operator==(const Document& other) const { return root_ == other.root_; }
bool Document::operator!=(const Document& other) const { return !(*this == other); }

Document Load(istream& input) { return Document{LoadNode(input)}; }

ostream& operator<<(ostream& out, const Node::JsonType& value) {
    visit(PrintNode{out}, value);
    return out;
}

void Print(const Document& doc, ostream& output) {
    visit(PrintNode{output}, doc.GetRoot().GetValue());
}

} // namespace json
