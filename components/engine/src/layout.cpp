#include "layout.h"
#include <string>
#include <iostream>
#include <algorithm>
#include <numeric>
#include <cmath> // For std::ceil

namespace Layout {

    std::unique_ptr<LayoutBox> build_layout_box(const Style::StyledNode* styled_node);

    template<typename T>
    T get_value(const Style::StyledNode* node, const std::string& prop_name) {
        if (!node) return T{};
        auto it = node->specified_values.find(prop_name);
        if (it != node->specified_values.end()) {
            if (const T* val = std::get_if<T>(&it->second)) {
                return *val;
            }
        }
        return T{};
    }

    float get_px_value(const Style::StyledNode* node, const std::string& prop_name) {
        return get_value<float>(node, prop_name);
    }

    void layout_block(LayoutBox* box, Dimensions containing_block);
    void layout_flex(LayoutBox* box, Dimensions containing_block);

    std::unique_ptr<LayoutBox> layout_tree(const Style::StyledNode& root, Dimensions viewport) {
        auto root_box = build_layout_box(&root);
        if (root_box->box_type == Layout::BoxType::Flex) {
            layout_flex(root_box.get(), viewport);
        } else {
            layout_block(root_box.get(), viewport);
        }
        return root_box;
    }

    std::unique_ptr<LayoutBox> build_layout_box(const Style::StyledNode* styled_node) {
        auto box = std::make_unique<LayoutBox>();
        box->styled_node = styled_node;

        if (styled_node->node->type == DOM::NodeType::Text) {
            box->box_type = Layout::BoxType::Anonymous;
        } else {
            auto display = get_value<CSS::Display>(styled_node, "display");
            if (display == CSS::Display::Flex) box->box_type = Layout::BoxType::Flex;
            else if (display == CSS::Display::Block) box->box_type = Layout::BoxType::Block;
            else box->box_type = Layout::BoxType::Inline;
        }

        for (const auto& child : styled_node->children) {
            if (child->node->type == DOM::NodeType::Element || 
               (child->node->type == DOM::NodeType::Text && child->node->text_data.find_first_not_of(" \t\n\r") != std::string::npos)) {
                box->children.push_back(build_layout_box(child.get()));
            }
        }
        return box;
    }

    void layout_flex(LayoutBox* box, Dimensions containing_block) {
        box->dimensions.x = containing_block.x;
        box->dimensions.y = containing_block.y;
        box->dimensions.width = containing_block.width;
        
        for (auto& child : box->children) {
            layout_block(child.get(), containing_block);
        }

        float total_children_width = 0.0f;
        for (const auto& child : box->children) {
            total_children_width += child->dimensions.width;
        }

        float remaining_space = box->dimensions.width - total_children_width;
        float spacing = 0.0f;
        float offset = 0.0f;

        auto justify = get_value<CSS::JustifyContent>(box->styled_node, "justify-content");
        if (justify == CSS::JustifyContent::FlexEnd) offset = remaining_space;
        else if (justify == CSS::JustifyContent::Center) offset = remaining_space / 2.0f;
        else if (justify == CSS::JustifyContent::SpaceBetween) {
            if (box->children.size() > 1) spacing = remaining_space / (box->children.size() - 1);
        } else if (justify == CSS::JustifyContent::SpaceAround) {
            if (!box->children.empty()) {
                spacing = remaining_space / box->children.size();
                offset = spacing / 2.0f;
            }
        }

        float current_x = box->dimensions.x + offset;
        float max_child_height = 0.0f;

        for (auto& child : box->children) {
            child->dimensions.x = current_x;
            child->dimensions.y = box->dimensions.y;
            current_x += child->dimensions.width + spacing;
            max_child_height = std::max(max_child_height, child->dimensions.height);
        }

        box->dimensions.height = get_px_value(box->styled_node, "height") > 0 ? get_px_value(box->styled_node, "height") : max_child_height;
    }

    void layout_block(LayoutBox* box, Dimensions containing_block) {
        box->dimensions.margin.top = get_px_value(box->styled_node, "margin-top");
        box->dimensions.margin.bottom = get_px_value(box->styled_node, "margin-bottom");
        box->dimensions.margin.left = get_px_value(box->styled_node, "margin-left");
        box->dimensions.margin.right = get_px_value(box->styled_node, "margin-right");
        box->dimensions.padding.top = get_px_value(box->styled_node, "padding-top");
        box->dimensions.padding.bottom = get_px_value(box->styled_node, "padding-bottom");
        box->dimensions.padding.left = get_px_value(box->styled_node, "padding-left");
        box->dimensions.padding.right = get_px_value(box->styled_node, "padding-right");

        box->dimensions.x = containing_block.x + box->dimensions.margin.left;
        box->dimensions.y = containing_block.y;
        
        float total_horizontal_space = box->dimensions.padding.left + box->dimensions.padding.right +
                                       box->dimensions.margin.left + box->dimensions.margin.right;
        box->dimensions.width = get_px_value(box->styled_node, "width") > 0 ? get_px_value(box->styled_node, "width") : containing_block.width - total_horizontal_space;

        Dimensions content_box;
        content_box.x = box->dimensions.x + box->dimensions.padding.left;
        content_box.y = box->dimensions.y + box->dimensions.padding.top;
        content_box.width = box->dimensions.width - box->dimensions.padding.left - box->dimensions.padding.right;
        
        float children_height = 0.0f;
        for (auto& child : box->children) {
            Dimensions child_cb = content_box;
            child_cb.y += children_height;

            if (child->box_type == Layout::BoxType::Anonymous) {
                child->dimensions.x = child_cb.x;
                child->dimensions.y = child_cb.y;
                child->dimensions.width = child_cb.width;
                
                float font_size = get_px_value(child->styled_node, "font-size") > 0 ? get_px_value(child->styled_node, "font-size") : 16.0f;
                float chars_per_line = child->dimensions.width / (font_size * 0.6f);
                if (chars_per_line > 0) {
                    float num_lines = std::ceil(child->styled_node->node->text_data.length() / chars_per_line);
                    child->dimensions.height = num_lines * font_size * 1.2f;
                } else {
                    child->dimensions.height = font_size * 1.2f;
                }

                children_height += child->dimensions.height;
            } else if (child->box_type == Layout::BoxType::Flex) {
                layout_flex(child.get(), child_cb);
                children_height += child->dimensions.margin.top + child->dimensions.height + child->dimensions.margin.bottom;
            } else {
                layout_block(child.get(), child_cb);
                children_height += child->dimensions.margin.top + child->dimensions.height + child->dimensions.margin.bottom;
            }
        }

        float explicit_height = get_px_value(box->styled_node, "height");
        if (explicit_height > 0.0f) {
            box->dimensions.height = explicit_height;
        } else {
            box->dimensions.height = children_height + box->dimensions.padding.top + box->dimensions.padding.bottom;
        }
    }
}