#ifndef CSS_H
#define CSS_H

#include <string>
#include <vector>
#include <map>
#include <variant>

namespace CSS {

    struct Color { uint8_t r = 0, g = 0, b = 0, a = 255; };

    // --- NEW: Enums for specific CSS properties ---
    enum class Display { Block, Inline, Flex, None };
    enum class FlexDirection { Row, Column };
    enum class JustifyContent { FlexStart, FlexEnd, Center, SpaceBetween, SpaceAround };

    // Value can now hold our new enum types
    using Value = std::variant<std::string, float, Color, Display, FlexDirection, JustifyContent>;

    struct Selector {
        std::string tag_name;
        std::string id;
        std::vector<std::string> classes;
    };

    struct Declaration {
        std::string property;
        Value value;
    };

    struct Rule {
        std::vector<Selector> selectors;
        std::vector<Declaration> declarations;
    };

    struct Stylesheet {
        std::vector<Rule> rules;
    };
}

#endif // CSS_H