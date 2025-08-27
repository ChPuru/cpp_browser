#ifndef LAYOUT_H
#define LAYOUT_H

#include "style.h"
#include <vector>
#include <memory>

namespace Layout {

    struct EdgeSizes {
        float top = 0.0f, right = 0.0f, bottom = 0.0f, left = 0.0f;
    };

    struct Dimensions {
        float x = 0.0f, y = 0.0f, width = 0.0f, height = 0.0f;
        EdgeSizes padding;
        EdgeSizes border;
        EdgeSizes margin;
    };

    enum class BoxType { Block, Inline, Anonymous, Flex };

    struct LayoutBox {
        Dimensions dimensions;
        BoxType box_type;
        const Style::StyledNode* styled_node = nullptr;
        std::vector<std::unique_ptr<LayoutBox>> children;
    };

    std::unique_ptr<LayoutBox> layout_tree(const Style::StyledNode& root, Dimensions viewport);
}

#endif // LAYOUT_H