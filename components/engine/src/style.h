#ifndef STYLE_H
#define STYLE_H

#include "dom.h"
#include "css.h"
#include <map>
#include <string>
#include <vector>
#include <memory>

namespace Style {

    // --- THIS IS THE CORRECTED LINE ---
    using PropertyMap = std::map<std::string, CSS::Value>;

    struct StyledNode {
        const DOM::Node* node;
        PropertyMap specified_values;
        std::vector<std::unique_ptr<StyledNode>> children;
    };

    std::unique_ptr<StyledNode> style_tree(const DOM::Node* root, const CSS::Stylesheet& stylesheet);
}

#endif // STYLE_H