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

        // First expressions with starting characters: '[', '{' and '"'
        if ((expression = this->parseArray()) != nullptr)
            return expression;
        if ((expression = this->parseDictionary()) != nullptr)
            return expression;
        if ((expression = this->parseString()) != nullptr)
            return expression;

        // Then built-in identifiers: True, False, None
        if ((expression = this->parseBooleanNone()) != nullptr)
            return expression;

        // Then custom identifiers - dataclasses
        if ((expression = this->parseDataclass()) != nullptr)
            return expression;

        // Lastly - numerals (identifiers cannot start with a number)
        if ((expression = this->parseNumeral()) != nullptr)
            return expression;

        throw ParseException(this->in, this->idx, "unexpected character");
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
                throw ParseException(this->in, this->idx, "expecting string as a dictionary key");

            this->ws();
            if (this->peek() == EOF)
                throw ParseException(this->in, this->idx, "unexpected EOF while parsing dictionary");
            if (this->get() != ':')
                throw ParseException(this->in, this->idx - 1, "missing colon ':' after key in dictionary");

            auto valueNode = this->parseExpression();

            elems.emplace_back(stringNode->getValue(), std::move(valueNode));

            this->ws();
        }
        this->get();    // eat '}'

        return ast::NodeDictionary::create(elems);
    }

    std::shared_ptr<const ast::NodeDataclass> Parser::parseDataclass() {
        auto className = this->parseIdentifier();
        if (!className.has_value())
            return nullptr;

        this->ws();
        if (this->peek() != '(')
            return ast::NodeDataclass::create(*className, ast::NodeArray::create(), ast::NodeDictionary::create());

        this->get();   // eat '('
        this->ws();
        std::vector<std::shared_ptr<const ast::Node>> positionalArguments;
        std::vector<std::pair<std::string, std::shared_ptr<const ast::Node>>> keywordArguments;
        while (this->peek() != ')') {
            if (this->peek() == EOF)
                throw ParseException(this->in, this->idx, "unexpected EOF while parsing dataclass");

            if (!positionalArguments.empty() || !keywordArguments.empty()) {
                if (this->get() != ',')
                    throw ParseException(this->in, this->idx - 1, "missing comma ',' while parsing dataclass");
                this->ws();
            }

            this->parseArgument(positionalArguments, keywordArguments);

            this->ws();
        }
        this->get();    // eat ')'

        auto positionalNode = ast::NodeArray::create(std::move(positionalArguments));
        auto keywordNode = ast::NodeDictionary::create(keywordArguments);
        return ast::NodeDataclass::create(*className, std::move(positionalNode), std::move(keywordNode));
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
                    throw ParseException(this->in, this->idx - 2, "'" + std::string(1, escaped) + "' cannot follow backslash");
                }
            }

            string << static_cast<char>(c);
        }

        return ast::NodeString::create(string.str());
    }

    std::shared_ptr<const ast::Node> Parser::parseBooleanNone() {
        std::size_t savedIdx = this->idx;
        std::optional<std::string> name = this->parseIdentifier();
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

    std::optional<std::string> Parser::parseIdentifier() {
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

    void Parser::parseArgument(std::vector<std::shared_ptr<const ast::Node>> &positionalArguments,
                               std::vector<std::pair<std::string, std::shared_ptr<const ast::Node>>> &keywordArguments)
    {
        auto keywordArgument = this->parseKeywordArgument();
        if (keywordArgument.has_value()) {
            keywordArguments.push_back(std::move(*keywordArgument));
            return;
        }

        std::size_t savedIdx = this->idx;
        auto positionalArgument = this->parseExpression();
        if (!keywordArguments.empty())
            throw ParseException(this->in, savedIdx, "positional arguments cannot follow keyword arguments");
        positionalArguments.push_back(std::move(positionalArgument));
    }

    std::optional<std::pair<std::string, std::shared_ptr<const ast::Node>>> Parser::parseKeywordArgument() {
        std::size_t savedIdx = this->idx;

        auto argumentName = this->parseIdentifier();
        if (!argumentName.has_value())
            return std::nullopt;

        this->ws();
        if (this->peek() != '=') {
            this->idx = savedIdx;
            return std::nullopt;
        }

        this->get();   // eat '='
        this->ws();
        auto node = this->parseExpression();
        if (node == nullptr) {
            this->idx = savedIdx;
            return std::nullopt;
        }

        return std::make_pair(*argumentName, std::move(node));
    }
} // pyon