#include "dom.h"

namespace DOM {
    std::unique_ptr<Node> create_text_node(const std::string& data) {
        auto node = std::make_unique<Node>();
        node->type = NodeType::Text;
        node->text_data = data;
        return node;
    }

    std::unique_ptr<Node> create_element_node(const std::string& name, AttrMap attrs, std::vector<std::unique_ptr<Node>> children) {
        auto node = std::make_unique<Node>();
        node->type = NodeType::Element;
        node->element_data.tag_name = name;
        node->element_data.attributes = std::move(attrs);
        node->children = std::move(children);
        return node;
    }
}