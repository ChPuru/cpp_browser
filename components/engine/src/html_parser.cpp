#include "html_parser.h"
#include <cassert>
#include <algorithm>

namespace HTML {

    Parser::Parser(std::string input) : m_input(std::move(input)) {}

    char Parser::next_char() { return m_input[m_pos]; }
    bool Parser::eof() { return m_pos >= m_input.length(); }
    bool Parser::starts_with(const std::string& s) { return m_input.substr(m_pos, s.length()) == s; }
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

    std::vector<std::unique_ptr<DOM::Node>> Parser::parse_nodes() {
        std::vector<std::unique_ptr<DOM::Node>> nodes;
        while (true) {
            consume_whitespace();
            if (eof() || starts_with("</")) {
                break;
            }
            nodes.push_back(parse_node());
        }
        return nodes;
    }

    std::unique_ptr<DOM::Node> Parser::parse_node() {
        if (next_char() == '<') {
            return parse_element();
        } else {
            return parse_text();
        }
    }

    std::unique_ptr<DOM::Node> Parser::parse_text() {
        return DOM::create_text_node(consume_while([](char c) { return c != '<'; }));
    }

    std::unique_ptr<DOM::Node> Parser::parse_element() {
        assert(consume_char() == '<');
        std::string tag_name = parse_tag_name();
        std::transform(tag_name.begin(), tag_name.end(), tag_name.begin(), ::tolower);
        DOM::AttrMap attrs = parse_attributes();
        assert(consume_char() == '>');

        auto children = parse_nodes();

        if (starts_with("</")) {
            assert(consume_char() == '<');
            assert(consume_char() == '/');
            std::string closing_tag = parse_tag_name();
            std::transform(closing_tag.begin(), closing_tag.end(), closing_tag.begin(), ::tolower);
            // We don't assert equality for robustness, but we consume the tag.
            assert(consume_char() == '>');
        }

        return DOM::create_element_node(tag_name, std::move(attrs), std::move(children));
    }

    std::string Parser::parse_tag_name() {
        return consume_while([](char c) { return isalnum(c); });
    }

    // --- THIS IS THE UPGRADED FUNCTION ---
    DOM::AttrMap Parser::parse_attributes() {
        DOM::AttrMap attributes;
        while (true) {
            consume_whitespace();
            if (next_char() == '>') {
                break;
            }
            std::string name = parse_tag_name();
            std::string value = ""; // Default to empty string for boolean attributes

            consume_whitespace();
            if (!eof() && next_char() == '=') {
                consume_char(); // consume '='
                value = parse_attr_value();
            }
            attributes[name] = value;
        }
        return attributes;
    }

    std::string Parser::parse_attr_value() {
        consume_whitespace();
        char open_quote = next_char();
        if (open_quote == '"' || open_quote == '\'') {
            consume_char();
            std::string value = consume_while([open_quote](char c) { return c != open_quote; });
            assert(consume_char() == open_quote);
            return value;
        } else {
            // Handle unquoted attributes
            return consume_while([](char c) { return !isspace(c) && c != '>'; });
        }
    }
}