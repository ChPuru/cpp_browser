#include "css_parser.h"
#include <cassert>
#include <algorithm>
#include <iostream>

namespace CSS {

    Parser::Parser(std::string input) : m_input(std::move(input)) {}

    char Parser::next_char() { return m_input[m_pos]; }
    bool Parser::eof() { return m_pos >= m_input.length(); }
    char Parser::consume_char() { return m_input[m_pos++]; }

    std::string Parser::consume_while(std::function<bool(char)> test) {
        std::string result;
        while (!eof() && test(next_char())) {
            result += consume_char();
        }
        return result;
    }

    void Parser::consume_whitespace() {
        consume_while(isspace);
    }

    Stylesheet Parser::parse_stylesheet() {
        Stylesheet sheet;
        while (!eof()) {
            consume_whitespace();
            if (eof()) break;
            sheet.rules.push_back(parse_rule());
        }
        return sheet;
    }

    Rule Parser::parse_rule() {
        Rule rule;
        rule.selectors = parse_selectors();
        rule.declarations = parse_declarations();
        return rule;
    }

    std::vector<Selector> Parser::parse_selectors() {
        std::vector<Selector> selectors;
        while (true) {
            selectors.push_back(parse_simple_selector());
            consume_whitespace();
            if (eof() || next_char() == '{') {
                break;
            }
            assert(consume_char() == ',');
            consume_whitespace();
        }
        return selectors;
    }

    Selector Parser::parse_simple_selector() {
        Selector selector;
        while (!eof() && next_char() != ',' && next_char() != '{' && !isspace(next_char())) {
            if (next_char() == '#') {
                consume_char();
                selector.id = consume_while([](char c) { return isalnum(c) || c == '-'; });
            } else if (next_char() == '.') {
                consume_char();
                selector.classes.push_back(consume_while([](char c) { return isalnum(c) || c == '-'; }));
            } else {
                selector.tag_name = consume_while([](char c) { return isalnum(c); });
            }
        }
        return selector;
    }

    std::vector<Declaration> Parser::parse_declarations() {
        assert(consume_char() == '{');
        std::vector<Declaration> declarations;
        while (true) {
            consume_whitespace();
            if (next_char() == '}') {
                consume_char();
                break;
            }
            declarations.push_back(parse_declaration());
        }
        return declarations;
    }

    // Helper to parse a hex string into a Color
    Color parse_hex_color(const std::string& hex_str) {
        Color color;
        if (hex_str.length() == 7 && hex_str[0] == '#') {
            try {
                color.r = std::stoul(hex_str.substr(1, 2), nullptr, 16);
                color.g = std::stoul(hex_str.substr(3, 2), nullptr, 16);
                color.b = std::stoul(hex_str.substr(5, 2), nullptr, 16);
            } catch (...) { /* invalid hex, return black */ }
        }
        return color;
    }

    Declaration Parser::parse_declaration() {
        Declaration decl;
        decl.property = consume_while([](char c) { return c != ':'; });
        decl.property.erase(0, decl.property.find_first_not_of(" \t\n\r"));
        decl.property.erase(decl.property.find_last_not_of(" \t\n\r") + 1);
        std::transform(decl.property.begin(), decl.property.end(), decl.property.begin(), ::tolower);

        assert(consume_char() == ':');
        consume_whitespace();
        std::string value_str = consume_while([](char c) { return c != ';'; });
        value_str.erase(0, value_str.find_first_not_of(" \t\n\r"));
        value_str.erase(value_str.find_last_not_of(" \t\n\r") + 1);

        // --- NEW PARSING LOGIC ---
        if (value_str.length() > 0 && value_str[0] == '#') {
            decl.value = parse_hex_color(value_str);
        } else if (value_str.length() > 2 && value_str.substr(value_str.length() - 2) == "px") {
            try {
                decl.value = std::stof(value_str.substr(0, value_str.length() - 2));
            } catch (...) {
                decl.value = value_str; // Fallback to string
            }
        } else {
            decl.value = value_str;
        }

        assert(consume_char() == ';');
        return decl;
    }
}