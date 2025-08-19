#include "html_parser.h"
#include <cassert>
#include <algorithm>

namespace HTML {

    Parser::Parser(std::string input) : m_input(std::move(input)) {}

    char Parser::next_char() {
        return m_input[m_pos];
    }

    bool Parser::eof() {
        return m_pos >= m_input.length();
    }

    bool Parser::starts_with(const std::string& s) {
        return m_input.substr(m_pos, s.length()) == s;
    }

    char Parser::consume_char() {
        return m_input[m_pos++];
    }

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
        }
        else {
            return parse_text();
        }
    }

    std::unique_ptr<DOM::Node> Parser::parse_text() {
        return DOM::create_text_node(consume_while([](char c) { return c != '<'; }));
    }

    std::unique_ptr<DOM::Node> Parser::parse_element() {
        assert(consume_char() == '<');
        std::string tag_name = parse_tag_name();
        DOM::AttrMap attrs = parse_attributes();
        assert(consume_char() == '>');

        auto children = parse_nodes();

        assert(consume_char() == '<');
        assert(consume_char() == '/');
        assert(parse_tag_name() == tag_name);
        assert(consume_char() == '>');

        return DOM::create_element_node(tag_name, std::move(attrs), std::move(children));
    }

    std::string Parser::parse_tag_name() {
        return consume_while([](char c) { return isalnum(c); });
    }

    DOM::AttrMap Parser::parse_attributes() {
        DOM::AttrMap attributes;
        while (true) {
            consume_whitespace();
            if (next_char() == '>') {
                break;
            }
            auto [name, value] = parse_attr();
            attributes[name] = value;
        }
        return attributes;
    }

    std::pair<std::string, std::string> Parser::parse_attr() {
        std::string name = parse_tag_name();
        assert(consume_char() == '=');
        std::string value = parse_attr_value();
        return { name, value };
    }

    std::string Parser::parse_attr_value() {
        char open_quote = consume_char();
        assert(open_quote == '"' || open_quote == '\'');
        std::string value = consume_while([open_quote](char c) { return c != open_quote; });
        assert(consume_char() == open_quote);
        return value;
    }
}