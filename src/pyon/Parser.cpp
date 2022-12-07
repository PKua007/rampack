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
        if ((literal = this->parseBoolean()) != nullptr)
            return literal;
        if ((literal = this->parseNone()) != nullptr)
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
        return nullptr;
    }

    std::shared_ptr<const ast::NodeBoolean> Parser::parseBoolean() {
        return nullptr;
    }

    std::shared_ptr<const ast::NodeNone> Parser::parseNone() {
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
} // pyon