#pragma once

#include "json.h"
#include <string>
#include <vector>
#include <optional>

namespace json {

class Builder;
class DictItemContext;
class ArrayItemContext;
class KeyItemContext;

class BaseContext {
public:
    BaseContext(Builder& b) : builder_(b) {}
protected:
    Builder& builder_;
};

class DictItemContext : public BaseContext {
public:
    DictItemContext(Builder& b) : BaseContext(b) {}
    
    KeyItemContext Key(std::string key);
    Builder& EndDict();
};

class ArrayItemContext : public BaseContext {
public:
    ArrayItemContext(Builder& b) : BaseContext(b) {}
    
    ArrayItemContext Value(Node value);
    DictItemContext StartDict();
    ArrayItemContext StartArray();
    Builder& EndArray();
};

class KeyItemContext : public BaseContext {
public:
    KeyItemContext(Builder& b) : BaseContext(b) {}
    
    DictItemContext Value(Node value);
    DictItemContext StartDict();
    ArrayItemContext StartArray();
};

class Builder {
public:
    Builder();
    
    KeyItemContext Key(std::string key);
    Builder& Value(Node value);
    DictItemContext StartDict();
    ArrayItemContext StartArray();
    Builder& EndDict();
    Builder& EndArray();
    Node Build();

private:
    Node root_;
    std::vector<Node*> nodes_stack_;
    std::optional<std::string> current_key_;
    bool root_initialized_ = false;
    
    Node* AddNode(Node node);
};

} // namespace json