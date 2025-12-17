#include "json_builder.h"
#include <stdexcept>
#include <utility>

namespace json {

KeyItemContext DictItemContext::Key(std::string key) {
    return builder_.Key(std::move(key));
}

Builder& DictItemContext::EndDict() {
    return builder_.EndDict();
}

ArrayItemContext ArrayItemContext::Value(Node value) {
    builder_.Value(std::move(value));
    return ArrayItemContext(builder_);
}

DictItemContext ArrayItemContext::StartDict() {
    return builder_.StartDict();
}

ArrayItemContext ArrayItemContext::StartArray() {
    return builder_.StartArray();
}

Builder& ArrayItemContext::EndArray() {
    return builder_.EndArray();
}

DictItemContext KeyItemContext::Value(Node value) {
    builder_.Value(std::move(value));
    return DictItemContext(builder_);
}

DictItemContext KeyItemContext::StartDict() {
    return builder_.StartDict();
}

ArrayItemContext KeyItemContext::StartArray() {
    return builder_.StartArray();
}

Builder::Builder() = default;

KeyItemContext Builder::Key(std::string key) {
    if (nodes_stack_.empty() || !nodes_stack_.back()->IsMap()) {
        throw std::logic_error("Key() called outside of a map");
    }
    if (current_key_) {
        throw std::logic_error("Key() called while another key is already active");
    }
    current_key_ = std::move(key);
    return KeyItemContext(*this);
}

Builder& Builder::Value(Node value) {
    AddNode(std::move(value));
    return *this;
}

DictItemContext Builder::StartDict() {
    nodes_stack_.push_back(AddNode(Dict{}));
    return DictItemContext(*this);
}

ArrayItemContext Builder::StartArray() {
    nodes_stack_.push_back(AddNode(Array{}));
    return ArrayItemContext(*this);
}

Builder& Builder::EndDict() {
    if (nodes_stack_.empty() || !nodes_stack_.back()->IsMap()) {
        throw std::logic_error("EndDict() called without active dictionary");
    }
    nodes_stack_.pop_back();
    return *this;
}

Builder& Builder::EndArray() {
    if (nodes_stack_.empty() || !nodes_stack_.back()->IsArray()) {
        throw std::logic_error("EndArray() called without active array");
    }
    nodes_stack_.pop_back();
    return *this;
}

Node Builder::Build() {
    if (!nodes_stack_.empty()) {
        throw std::logic_error("Build() called with unfinished structure");
    }
    if (!root_initialized_) {
        throw std::logic_error("Build() called on empty builder");
    }
    return root_;
}

Node* Builder::AddNode(Node node) {
    if (nodes_stack_.empty()) {
        if (root_initialized_) {
            throw std::logic_error("Root already contains a value");
        }
        root_ = std::move(node);
        root_initialized_ = true;
        return &root_;
    } else {
        Node* current = nodes_stack_.back();
        
        if (current->IsArray()) {
            Array& arr = const_cast<Array&>(current->AsArray());
            arr.push_back(std::move(node));
            return &arr.back();
        } else if (current->IsMap()) {
            if (!current_key_) {
                throw std::logic_error("Value added to dictionary without a key");
            }
            Dict& dict = const_cast<Dict&>(current->AsMap());
            auto res = dict.emplace(std::move(*current_key_), std::move(node));
            current_key_ = std::nullopt;
            return &res.first->second;
        } else {
            throw std::logic_error("Attempt to add value to scalar node");
        }
    }
}

}