#ifndef CSS_H
#define CSS_H

#include <string>
#include <vector>
#include <map>
#include <variant>

namespace CSS {

    struct Color {
        uint8_t r = 0, g = 0, b = 0, a = 255;
    };

    // A Value can now be a keyword (string), a length (float), or a Color.
    using Value = std::variant<std::string, float, Color>;

    struct Selector {
        std::string tag_name;
        std::string id;
        std::vector<std::string> classes;
    };

    struct Declaration {
        std::string property;
        Value value; // Use the new Value type
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