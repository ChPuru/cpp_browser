#ifndef HTML_PARSER_H
#define HTML_PARSER_H

#include "dom.h"
#include <string>
#include <vector>
#include <functional>
#include <unordered_set>

namespace HTML {
    class Parser {
    public:
        Parser(std::string input);
        // --- THIS IS THE PUBLIC ENTRY POINT ---
        std::vector<std::unique_ptr<DOM::Node>> parse_nodes();

    private:
        std::string m_input;
        size_t m_pos = 0;

        // --- THIS IS THE NEW PRIVATE, RECURSIVE HELPER ---
        std::vector<std::unique_ptr<DOM::Node>> parse_nodes_recursive(const std::string& parent_tag);

        char next_char();
        bool eof();
        bool starts_with(const std::string& s);
        char consume_char();
        std::string consume_while(std::function<bool(char)> test);
        void consume_whitespace();

        std::unique_ptr<DOM::Node> parse_node();
        std::unique_ptr<DOM::Node> parse_text();
        std::unique_ptr<DOM::Node> parse_element();
        std::string parse_tag_name();
        DOM::AttrMap parse_attributes();
        std::pair<std::string, std::string> parse_attr();
        std::string parse_attr_value();
    };
}

#endif // HTML_PARSER_H