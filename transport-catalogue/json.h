#pragma once

#include <iostream>
#include <map>
#include <string>
#include <variant>
#include <vector>
#include <stdexcept>

namespace json {

class Node;
using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;

class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

class Node {
public:
    using JsonType = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;

    Node();
    Node(std::nullptr_t);
    Node(Array arr);
    Node(Dict map);
    Node(double value);
    Node(bool value);
    Node(int value);
    Node(const std::string& value);
    Node(std::string&& value);
    Node(const char* value);

    bool IsInt() const;
    bool IsDouble() const;
    bool IsPureDouble() const;
    bool IsBool() const;
    bool IsString() const;
    bool IsNull() const;
    bool IsArray() const;
    bool IsMap() const;

    const JsonType& GetValue() const;

    int AsInt() const;
    bool AsBool() const;
    double AsDouble() const;
    const std::string& AsString() const;
    const Array& AsArray() const;
    const Dict& AsMap() const;

    bool operator==(const Node& node) const;
    bool operator!=(const Node& node) const;

private:
    JsonType type_;
};

class Document {
public:
    Document() = default;
    explicit Document(Node root);

    const Node& GetRoot() const;

    bool operator==(const Document& other) const;
    bool operator!=(const Document& other) const;

private:
    Node root_;
};

struct PrintNode {
    std::ostream& out;

    void operator()(std::nullptr_t) const;
    void operator()(const Array& array) const;
    void operator()(const Dict& dict) const;
    void operator()(bool bool_value) const;
    void operator()(int int_value) const;
    void operator()(double dbl_value) const;
    void operator()(const std::string& str) const;
};

Document Load(std::istream& input);
void Print(const Document& doc, std::ostream& output);

std::ostream& operator<<(std::ostream& out, const Node::JsonType& value);

}
