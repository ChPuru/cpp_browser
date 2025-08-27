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
        while (!eof()) {
            if (isspace(next_char())) {
                consume_char();
            } else if (m_input.substr(m_pos, 2) == "/*") {
                consume_char(); consume_char();
                while (!eof() && m_input.substr(m_pos, 2) != "*/") {
                    consume_char();
                }
                consume_char(); consume_char();
            } else {
                break;
            }
        }
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
            char next = next_char();
            if (next == '{') {
                // Start of declarations, we're done.
                break;
            } else if (next == ',') {
                // Another selector in the list, consume the comma and continue.
                consume_char();
                consume_whitespace();
            } else if (eof()) {
                // Should not happen, but break just in case.
                break;
            }
            // If it's not a '{' or a ',', it's a descendant selector (e.g. "nav p").
            // We just continue the loop to parse the next part.
        }
        return selectors;
    }

    Selector Parser::parse_simple_selector() {
        Selector selector;
        // This function now correctly handles a single part of a selector, like '#nav' or 'p'
        while (!eof() && !isspace(next_char()) && next_char() != ',' && next_char() != '{') {
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

    Color parse_hex_color(const std::string& hex_str) {
        Color color;
        if (hex_str.length() == 7 && hex_str[0] == '#') {
            try {
                color.r = std::stoul(hex_str.substr(1, 2), nullptr, 16);
                color.g = std::stoul(hex_str.substr(3, 2), nullptr, 16);
                color.b = std::stoul(hex_str.substr(5, 2), nullptr, 16);
            } catch (...) {}
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

        if (decl.property == "display") {
            if (value_str == "flex") decl.value = Display::Flex;
            else if (value_str == "none") decl.value = Display::None;
            else decl.value = Display::Block;
        } else if (decl.property == "flex-direction") {
            if (value_str == "column") decl.value = FlexDirection::Column;
            else decl.value = FlexDirection::Row;
        } else if (decl.property == "justify-content") {
            if (value_str == "flex-end") decl.value = JustifyContent::FlexEnd;
            else if (value_str == "center") decl.value = JustifyContent::Center;
            else if (value_str == "space-between") decl.value = JustifyContent::SpaceBetween;
            else if (value_str == "space-around") decl.value = JustifyContent::SpaceAround;
            else decl.value = JustifyContent::FlexStart;
        } else if (value_str.length() > 0 && value_str[0] == '#') {
            decl.value = parse_hex_color(value_str);
        } else if (value_str.length() > 2 && value_str.substr(value_str.length() - 2) == "px") {
            try {
                decl.value = std::stof(value_str.substr(0, value_str.length() - 2));
            } catch (...) { decl.value = value_str; }
        } else {
            decl.value = value_str;
        }

        assert(consume_char() == ';');
        return decl;
    }
}