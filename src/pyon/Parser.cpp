//
// Created by pkua on 06.12.22.
//

#include "Parser.h"

namespace pyon {
    std::shared_ptr<const ast::Node> Parser::parse(const std::string &expression) {
        Parser parser(expression);
        auto rootNode = parser.parseExpression();
//        if (!this->in.eof()) {
//            this->in >> std::ws;
//            if (!this->in.eof())
//                throw ParseException(this->in.str(), this->idx(), "unexpected character");
//        }
        return rootNode;
    }

    std::shared_ptr<const ast::Node> Parser::parse(std::istream &expression) {
        std::ostringstream out;
        out << expression.rdbuf();
        return Parser::parse(out.str());
    }

    std::shared_ptr<const ast::Node> Parser::parseExpression() {
        this->in >> std::ws;
        if (this->in.eof())
            throw ParseException(this->in.str(), this->idx(), "unexpected EOF");

        std::shared_ptr<const ast::Node> expression;
        if ((expression = this->parseArray()) != nullptr)
            return expression;
        if ((expression = this->parseDictionary()) != nullptr)
            return expression;
        if ((expression = this->parseDataclass()) != nullptr)
            return expression;
        if ((expression = this->parseLiteral()) != nullptr)
            return expression;

        throw ParseException(this->in.str(), this->idx(), "unexpected character");
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
        return nullptr;
    }

    std::shared_ptr<const ast::NodeDictionary> Parser::parseDictionary() {
        return nullptr;
    }

    std::shared_ptr<const ast::NodeDataclass> Parser::parseDataclass() {
        return nullptr;
    }

    void Parser::backtrack(std::size_t savedIdx) {
        this->in.clear();
        this->in.seekg(static_cast<std::ios::off_type>(savedIdx), std::ios::beg);
    }

    std::shared_ptr<const ast::NodeString> Parser::parseString() {
        if (this->in.peek() != '"')
            return nullptr;
        this->in.get();

        std::ostringstream string;
        int c;
        while ((c = this->in.get()) != '"') {
            if (c == EOF)
                throw ParseException(this->in.str(), this->idx(), "unexpected EOF while parsing string");

            if (c == '"')
                break;

            if (c == '\\') {
                int escaped = this->in.get();
                if (escaped == EOF)
                    throw ParseException(this->in.str(), this->idx(), "unexpected EOF after backslash");

                if (escaped == '\\')
                    c = '\\';
                else if (escaped == '"')
                    c = '"';
                else if (escaped == 'n')
                    c = '\n';
                else if (escaped == 't')
                    c = '\t';
                else {
                    throw ParseException(this->in.str(), this->idx(),
                                         "'" + std::string(c, 1) + "' cannot follow backslash");
                }
            }

            string << static_cast<char>(c);
        }

        return ast::NodeString::create(string.str());
    }

    std::shared_ptr<const ast::Node> Parser::parseBooleanNone() {
        std::size_t savedIdx = this->idx();
        std::optional<std::string> name = this->parseName();
        if (!name.has_value()) {
            this->backtrack(savedIdx);
            return nullptr;
        }

        if (name == "True")
            return ast::NodeBoolean::create(true);
        else if (name == "False")
            return ast::NodeBoolean::create(false);
        else if (name == "None")
            return ast::NodeNone::create();

        this->backtrack(savedIdx);
        return nullptr;
    }

    std::shared_ptr<const ast::Node> Parser::parseNumeral() {
        std::size_t savedIdx = this->idx();

        auto float_ = this->parseFloat();
        std::size_t floatIdx = this->idx();
        this->backtrack(savedIdx);

        auto int_ = this->parseInt();
        std::size_t intIdx = this->idx();
        this->backtrack(savedIdx);

        if (float_ != nullptr) {
            if (floatIdx > intIdx) {
                this->in.seekg(static_cast<std::ios::off_type>(floatIdx), std::ios::beg);
                return float_;
            } else {
                this->in.seekg(static_cast<std::ios::off_type>(intIdx), std::ios::beg);
                return int_;
            }
        }
        return nullptr;
    }

    std::shared_ptr<const ast::NodeInt> Parser::parseInt() {
        long int i;
        this->in >> i;
        if (!this->in)
            return nullptr;

        return ast::NodeInt::create(i);
    }

    std::shared_ptr<const ast::NodeFloat> Parser::parseFloat() {
        double d;
        this->in >> d;
        if (!this->in)
            return nullptr;

        return ast::NodeFloat::create(d);
    }

    std::optional<std::string> Parser::parseName() {
        std::ostringstream name;

        auto isFirstNameChar = [](int c) { return std::isalpha(c) || c == '_'; };
        if (!isFirstNameChar(this->in.peek()))
            return std::nullopt;
        name << static_cast<char>(this->in.get());

        auto isNameChar = [](int c) { return std::isalnum(c) || c == '_'; };
        while (isNameChar(this->in.peek()))
            name << static_cast<char>(this->in.get());

        return name.str();
    }
} // pyon