#include "layout.h"
#include <string>
#include <iostream>
#include <algorithm> // For std::max

namespace Layout {

    // Helper function to get a float value from a styled property
    float get_float_value(const Style::StyledNode* node, const std::string& prop_name) {
        if (!node) return 0.0f;
        auto it = node->specified_values.find(prop_name);
        if (it != node->specified_values.end()) {
            if (const float* val = std::get_if<float>(&it->second)) {
                return *val;
            }
        }
        return 0.0f;
    }

    std::unique_ptr<LayoutBox> build_layout_box(const Style::StyledNode* styled_node) {
        auto box = std::make_unique<LayoutBox>();
        box->styled_node = styled_node;

        if (styled_node->node->type == DOM::NodeType::Text) {
            box->box_type = BoxType::Anonymous;
        } else {
            auto it = styled_node->specified_values.find("display");
            if (it != styled_node->specified_values.end()) {
                if (auto val = std::get_if<std::string>(&it->second)) {
                    if (*val == "block") box->box_type = BoxType::Block;
                    else box->box_type = BoxType::Inline;
                } else { box->box_type = BoxType::Inline; }
            } else { box->box_type = BoxType::Inline; }
        }

        for (const auto& child : styled_node->children) {
            if (child->node->type == DOM::NodeType::Element) {
                 box->children.push_back(build_layout_box(child.get()));
            } else if (child->node->type == DOM::NodeType::Text) {
                const std::string& text = child->node->text_data;
                if (text.find_first_not_of(" \t\n\r") != std::string::npos) {
                    auto text_box = std::make_unique<LayoutBox>();
                    text_box->styled_node = child.get();
                    text_box->box_type = BoxType::Anonymous;
                    box->children.push_back(std::move(text_box));
                }
            }
        }
        return box;
    }

    void layout_block(LayoutBox* box, Dimensions containing_block) {
        // --- 1. Calculate box properties from CSS ---
        box->dimensions.margin.top = get_float_value(box->styled_node, "margin-top");
        box->dimensions.margin.bottom = get_float_value(box->styled_node, "margin-bottom");
        box->dimensions.margin.left = get_float_value(box->styled_node, "margin-left");
        box->dimensions.margin.right = get_float_value(box->styled_node, "margin-right");

        box->dimensions.padding.top = get_float_value(box->styled_node, "padding-top");
        box->dimensions.padding.bottom = get_float_value(box->styled_node, "padding-bottom");
        box->dimensions.padding.left = get_float_value(box->styled_node, "padding-left");
        box->dimensions.padding.right = get_float_value(box->styled_node, "padding-right");

        box->dimensions.border.top = get_float_value(box->styled_node, "border-top-width");
        box->dimensions.border.bottom = get_float_value(box->styled_node, "border-bottom-width");
        box->dimensions.border.left = get_float_value(box->styled_node, "border-left-width");
        box->dimensions.border.right = get_float_value(box->styled_node, "border-right-width");

        // --- 2. Calculate position of the box's outer edge ---
        box->dimensions.x = containing_block.x + box->dimensions.margin.left;
        box->dimensions.y = containing_block.y + box->dimensions.margin.top;

        // --- 3. Calculate the width of the box ---
        float total_horizontal_space = box->dimensions.padding.left + box->dimensions.padding.right +
                                       box->dimensions.border.left + box->dimensions.border.right +
                                       box->dimensions.margin.left + box->dimensions.margin.right;
        box->dimensions.width = containing_block.width - total_horizontal_space;

        // --- 4. Calculate the position and size of the content box for children ---
        Dimensions content_box;
        content_box.x = box->dimensions.x + box->dimensions.border.left + box->dimensions.padding.left;
        content_box.y = box->dimensions.y + box->dimensions.border.top + box->dimensions.padding.top;
        content_box.width = box->dimensions.width - box->dimensions.border.left - box->dimensions.border.right -
                            box->dimensions.padding.left - box->dimensions.padding.right;
        
        float children_height = 0.0f;
        for (auto& child : box->children) {
            if (child->box_type == BoxType::Anonymous) {
                child->dimensions.x = content_box.x;
                child->dimensions.y = content_box.y + children_height;
                child->dimensions.width = content_box.width;
                child->dimensions.height = 20.0f; // Fixed line height
                children_height += child->dimensions.height;
            } else {
                Dimensions child_cb = content_box;
                child_cb.y += children_height;
                layout_block(child.get(), child_cb);
                children_height += child->dimensions.margin.top + child->dimensions.height + child->dimensions.margin.bottom;
            }
        }

        // --- 5. Calculate the final height of the box ---
        float explicit_height = get_float_value(box->styled_node, "height");
        if (explicit_height > 0.0f) {
            box->dimensions.height = explicit_height;
        } else {
            box->dimensions.height = children_height;
        }
        
        // The final height of the *outer box* must include its own padding and border
        box->dimensions.height += box->dimensions.padding.top + box->dimensions.padding.bottom +
                                  box->dimensions.border.top + box->dimensions.border.bottom;
    }

    std::unique_ptr<LayoutBox> layout_tree(const Style::StyledNode& root, Dimensions viewport) {
        auto root_box = build_layout_box(&root);
        layout_block(root_box.get(), viewport);
        return root_box;
    }
}