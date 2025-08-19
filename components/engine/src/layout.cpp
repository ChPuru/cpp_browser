#include "layout.h"
#include <string>
#include <iostream>
#include <algorithm>

namespace Layout {

    float get_px_value(const Style::StyledNode* node, const std::string& prop_name) {
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
                    if (*val == "block") {
                        box->box_type = BoxType::Block;
                    } else {
                        box->box_type = BoxType::Inline;
                    }
                } else {
                    box->box_type = BoxType::Inline;
                }
            } else {
                box->box_type = BoxType::Inline;
            }
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
        box->dimensions.width = containing_block.width - total_horizontal_space;

        Dimensions content_box;
        content_box.x = box->dimensions.x + box->dimensions.padding.left;
        content_box.y = box->dimensions.y + box->dimensions.padding.top;
        content_box.width = box->dimensions.width - box->dimensions.padding.left - box->dimensions.padding.right;
        
        float children_height = 0.0f;
        for (auto& child : box->children) {
            Dimensions child_cb = content_box;
            child_cb.y += children_height;

            if (child->box_type == BoxType::Anonymous) {
                child->dimensions.x = child_cb.x;
                child->dimensions.y = child_cb.y;
                child->dimensions.width = child_cb.width;
                child->dimensions.height = get_px_value(child->styled_node, "font-size") > 0 ? get_px_value(child->styled_node, "font-size") : 20.0f;
                children_height += child->dimensions.height;
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

    std::unique_ptr<LayoutBox> layout_tree(const Style::StyledNode& root, Dimensions viewport) {
        auto root_box = build_layout_box(&root);
        layout_block(root_box.get(), viewport);
        return root_box;
    }
}