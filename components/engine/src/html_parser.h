#ifndef HTML_PARSER_H
#define HTML_PARSER_H

#include "dom.h"
#include <string>
#include <vector>
#include <functional> // <-- ADD THIS LINE

namespace HTML {
    class Parser {
    public:
        Parser(std::string input);
        std::vector<std::unique_ptr<DOM::Node>> parse_nodes();

    private:
        std::string m_input;
        size_t m_pos = 0;

        char next_char();
        bool eof();
        bool starts_with(const std::string& s);
        char consume_char();
        // This line now works because we included <functional>
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