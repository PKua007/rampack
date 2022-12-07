//
// Created by pkua on 06.12.22.
//

#include "Parser.h"

#include <iostream>


namespace pyon {
    std::shared_ptr<const ast::Node> Parser::parse(const std::string &expression) {
        Parser parser(expression);
        auto rootNode = parser.parseExpression();
        parser.throwIfAnythingLeft();
        return rootNode;
    }

    std::shared_ptr<const ast::Node> Parser::parse(std::istream &expression) {
        std::ostringstream out;
        out << expression.rdbuf();
        return Parser::parse(out.str());
    }

    std::shared_ptr<const ast::Node> Parser::parseExpression() {
        this->ws();
        if (this->eof())
            throw ParseException(this->in, this->idx, "unexpected EOF");

        std::shared_ptr<const ast::Node> expression;
        if ((expression = this->parseArray()) != nullptr)
            return expression;
        if ((expression = this->parseDictionary()) != nullptr)
            return expression;
        if ((expression = this->parseDataclass()) != nullptr)
            return expression;
        if ((expression = this->parseLiteral()) != nullptr)
            return expression;

        throw ParseException(this->in, this->idx, "unexpected character");
    }

    std::shared_ptr<const ast::Node> Parser::parseLiteral() {
        std::shared_ptr<const ast::Node> literal;
        if ((literal = this->parseString()) != nullptr)
            return literal;
        if ((literal = this->parseBooleanNone()) != nullptr)
            return literal;
        if ((literal = this->parseNumeral()) != nullptr)
            return literal;
        return nullptr;
    }

    std::shared_ptr<const ast::NodeArray> Parser::parseArray() {
        if (this->peek() != '[')
            return nullptr;

        std::vector<std::shared_ptr<const ast::Node>> elems;
        this->get();  // eat '['
        this->ws();
        while (this->peek() != ']') {
            if (this->peek() == EOF)
                throw ParseException(this->in, this->idx, "unexpected EOF while parsing array");

            if (!elems.empty()) {
                if (this->get() != ',')
                    throw ParseException(this->in, this->idx - 1, "missing comma ',' while parsing array");
                this->ws();
            }

            elems.push_back(this->parseExpression());
            this->ws();
        }
        this->get();    // eat ']'

        return ast::NodeArray::create(std::move(elems));
    }

    std::shared_ptr<const ast::NodeDictionary> Parser::parseDictionary() {
        if (this->peek() != '{')
            return nullptr;

        std::vector<std::pair<std::string, std::shared_ptr<const ast::Node>>> elems;
        this->get();  // eat '{'
        this->ws();
        while (this->peek() != '}') {
            if (this->peek() == EOF)
                throw ParseException(this->in, this->idx, "unexpected EOF while parsing dictionary");

            if (!elems.empty()) {
                if (this->get() != ',')
                    throw ParseException(this->in, this->idx - 1, "missing comma ',' while parsing dictionary");
                this->ws();
            }

            auto stringNode = this->parseString();
            if (stringNode == nullptr)
                throw ParseException(this->in, this->idx, "dictionary key should be string");

            this->ws();
            if (this->peek() == EOF)
                throw ParseException(this->in, this->idx, "unexpected EOF while parsing dictionary");
            if (this->get() != ':')
                throw ParseException(this->in, this->idx - 1, "missing colon ':' after key in dictionary");

            auto valueNode = this->parseExpression();

            elems.emplace_back(stringNode->getValue(), std::move(valueNode));
        }
        this->get();    // eat '}'

        return ast::NodeDictionary::create(elems);
    }

    std::shared_ptr<const ast::NodeDataclass> Parser::parseDataclass() {
        return nullptr;
    }

    std::shared_ptr<const ast::NodeString> Parser::parseString() {
        if (this->peek() != '"')
            return nullptr;
        this->get();

        std::ostringstream string;
        int c;
        while ((c = this->get()) != '"') {
            if (c == EOF)
                throw ParseException(this->in, this->idx, "unexpected EOF while parsing string");

            if (c == '"')
                break;

            if (c == '\\') {
                int escaped = this->get();
                if (escaped == EOF)
                    throw ParseException(this->in, this->idx, "unexpected EOF after backslash");

                if (escaped == '\\')
                    c = '\\';
                else if (escaped == '"')
                    c = '"';
                else if (escaped == 'n')
                    c = '\n';
                else if (escaped == 't')
                    c = '\t';
                else {
                    throw ParseException(this->in, this->idx, "'" + std::string(c, 1) + "' cannot follow backslash");
                }
            }

            string << static_cast<char>(c);
        }

        return ast::NodeString::create(string.str());
    }

    std::shared_ptr<const ast::Node> Parser::parseBooleanNone() {
        std::size_t savedIdx = this->idx;
        std::optional<std::string> name = this->parseName();
        if (!name.has_value()) {
            this->idx = savedIdx;
            return nullptr;
        }

        if (name == "True")
            return ast::NodeBoolean::create(true);
        else if (name == "False")
            return ast::NodeBoolean::create(false);
        else if (name == "None")
            return ast::NodeNone::create();

        this->idx = savedIdx;
        return nullptr;
    }

    std::shared_ptr<const ast::Node> Parser::parseNumeral() {
        std::size_t savedIdx = this->idx;

        auto float_ = this->parseFloat();
        std::size_t floatIdx = this->idx;
        this->idx = savedIdx;

        auto int_ = this->parseInt();
        std::size_t intIdx = this->idx;
        this->idx = savedIdx;

        if (float_ != nullptr) {
            if (floatIdx > intIdx) {
                this->idx = floatIdx;
                return float_;
            } else {
                this->idx = intIdx;
                return int_;
            }
        }
        return nullptr;
    }

    std::shared_ptr<const ast::NodeInt> Parser::parseInt() {
        const char *start = this->in.c_str() + this->idx;
        char *end;
        long int i = std::strtol(start, &end, 0);

        if (start == end)
            return nullptr;
        if (errno == ERANGE)
            throw ParseException(this->in, this->idx, "integer out of range");

        this->idx += (end - start);
        return ast::NodeInt::create(i);
    }

    std::shared_ptr<const ast::NodeFloat> Parser::parseFloat() {
        const char *start = this->in.c_str() + this->idx;
        char *end;
        double d = std::strtod(start, &end);

        if (start == end)
            return nullptr;
        if (errno == ERANGE)
            throw ParseException(this->in, this->idx, "float out of range");

        this->idx += (end - start);
        return ast::NodeFloat::create(d);
    }

    std::optional<std::string> Parser::parseName() {
        std::ostringstream name;

        auto isFirstNameChar = [](int c) { return std::isalpha(c) || c == '_'; };
        if (!isFirstNameChar(this->peek()))
            return std::nullopt;
        name << static_cast<char>(this->get());

        auto isNameChar = [](int c) { return std::isalnum(c) || c == '_'; };
        while (isNameChar(this->peek()))
            name << static_cast<char>(this->get());

        return name.str();
    }

    void Parser::throwIfAnythingLeft() {
        this->ws();
        if (!this->eof())
            throw ParseException(this->in, this->idx, "unexpected character");
    }

    int Parser::peek() const {
        if (this->eof())
            return EOF;
        else
            return this->in[this->idx];
    }

    int Parser::get() {
        if (this->eof())
            return EOF;
        return this->in[this->idx++];
    }

    void Parser::ws() {
        while (std::isspace(this->peek()))
            this->get();
    }
} // pyon