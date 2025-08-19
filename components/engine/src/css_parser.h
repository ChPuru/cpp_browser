#ifndef CSS_PARSER_H
#define CSS_PARSER_H

#include "css.h"
#include <string>
#include <functional> // <-- ADD THIS LINE

namespace CSS {
    class Parser {
    public:
        Parser(std::string input);
        Stylesheet parse_stylesheet();

    private:
        std::string m_input;
        size_t m_pos = 0;

        char next_char();
        bool eof();
        char consume_char();
        // This line now works because we included <functional>
        std::string consume_while(std::function<bool(char)> test);
        void consume_whitespace();

        Rule parse_rule();
        std::vector<Selector> parse_selectors();
        Selector parse_simple_selector();
        std::vector<Declaration> parse_declarations();
        Declaration parse_declaration();
    };
}

#endif // CSS_PARSER_H