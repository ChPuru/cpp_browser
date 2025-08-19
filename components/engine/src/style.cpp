#include "style.h"
#include <algorithm>
#include <vector>
#include <sstream>

namespace Style {

    bool selector_matches(const DOM::ElementData& elem, const CSS::Selector& selector) {
        if (!selector.tag_name.empty() && selector.tag_name != elem.tag_name) {
            return false;
        }
        if (!selector.id.empty()) {
            auto it = elem.attributes.find("id");
            if (it == elem.attributes.end() || it->second != selector.id) {
                return false;
            }
        }
        if (!selector.classes.empty()) {
            auto it = elem.attributes.find("class");
            if (it == elem.attributes.end()) return false;
            
            std::vector<std::string> elem_classes;
            std::string current_class;
            std::istringstream iss(it->second);
            while (iss >> current_class) {
                elem_classes.push_back(current_class);
            }

            for (const auto& sel_class : selector.classes) {
                if (std::find(elem_classes.begin(), elem_classes.end(), sel_class) == elem_classes.end()) {
                    return false;
                }
            }
        }
        return true;
    }

    PropertyMap specified_values(const DOM::ElementData& elem, const CSS::Stylesheet& stylesheet) {
        PropertyMap values;
        for (const auto& rule : stylesheet.rules) {
            for (const auto& selector : rule.selectors) {
                if (selector_matches(elem, selector)) {
                    for (const auto& decl : rule.declarations) {
                        values[decl.property] = decl.value;
                    }
                }
            }
        }
        return values;
    }

    // --- THIS FUNCTION IS NOW CORRECT ---
    std::unique_ptr<StyledNode> style_tree(const DOM::Node* root, const CSS::Stylesheet& stylesheet) {
        auto styled_node = std::make_unique<StyledNode>();
        styled_node->node = root;

        if (root->type == DOM::NodeType::Element) {
            styled_node->specified_values = specified_values(root->element_data, stylesheet);
        }

        for (const auto& child : root->children) {
            auto styled_child = style_tree(child.get(), stylesheet);

            // --- THE INHERITANCE FIX ---
            // If the child is a text node, it needs to inherit color from its parent.
            if (styled_child->node->type == DOM::NodeType::Text) {
                if (styled_child->specified_values.find("color") == styled_child->specified_values.end()) {
                    auto parent_color = styled_node->specified_values.find("color");
                    if (parent_color != styled_node->specified_values.end()) {
                        styled_child->specified_values["color"] = parent_color->second;
                    }
                }
            }
            
            styled_node->children.push_back(std::move(styled_child));
        }

        return styled_node;
    }
}