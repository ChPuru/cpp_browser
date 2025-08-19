#ifndef DOM_H
#define DOM_H

#include <string>
#include <vector>
#include <map>
#include <memory>

namespace DOM {

    struct Node; // Forward declaration

    using AttrMap = std::map<std::string, std::string>;

    struct ElementData {
        std::string tag_name;
        AttrMap attributes;
    };

    enum class NodeType {
        Element,
        Text
    };

    struct Node {
        NodeType type;
        ElementData element_data;
        std::string text_data;
        std::vector<std::unique_ptr<Node>> children;
    };

    // Helper functions
    std::unique_ptr<Node> create_text_node(const std::string& data);
    std::unique_ptr<Node> create_element_node(const std::string& name, AttrMap attrs, std::vector<std::unique_ptr<Node>> children);
}

#endif // DOM_H