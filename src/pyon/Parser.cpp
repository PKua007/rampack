//
// Created by pkua on 06.12.22.
//

#include "Parser.h"

#include <cmath>
#include <algorithm>


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

        // First, expressions with starting characters: '[', '{' and '"'
        if ((expression = this->parseArray()) != nullptr)
            return expression;
        if ((expression = this->parseDictionary()) != nullptr)
            return expression;
        if ((expression = this->parseString()) != nullptr)
            return expression;

        // Then, built-in identifiers: True, False, None
        if ((expression = this->parseBooleanNone()) != nullptr)
            return expression;

        // Then, custom identifiers - dataclasses
        if ((expression = this->parseDataclass()) != nullptr)
            return expression;

        // Last, numerals (identifiers cannot start with a number)
        if ((expression = this->parseNumeral()) != nullptr)
            return expression;

        throw ParseException(this->in, this->idx, "unexpected character");
    }

    std::shared_ptr<const ast::NodeArray> Parser::parseArray() {
        if (this->peek() != '[')
            return nullptr;

        std::vector<std::shared_ptr<const ast::Node>> elems;
        this->eat();  // eat '['
        this->ws();
        while (this->peek() != ']') {
            if (this->peek() == EOF)
                throw ParseException(this->in, this->idx, "unexpected EOF while parsing array");

            // First element isn't prepended by comma
            if (!elems.empty()) {
                if (this->eat() != ',')
                    throw ParseException(this->in, this->idx - 1, "missing comma ',' while parsing array");
                this->ws();
            }

            elems.push_back(this->parseExpression());

            this->ws();
        }
        this->eat();    // eat ']'

        return ast::NodeArray::create(std::move(elems));
    }

    std::shared_ptr<const ast::NodeDictionary> Parser::parseDictionary() {
        if (this->peek() != '{')
            return nullptr;

        std::vector<std::pair<std::string, std::shared_ptr<const ast::Node>>> elems;
        this->eat();  // eat '{'
        this->ws();
        while (this->peek() != '}') {
            if (this->peek() == EOF)
                throw ParseException(this->in, this->idx, "unexpected EOF while parsing dictionary");

            // First element isn't prepended by comma
            if (!elems.empty()) {
                if (this->eat() != ',')
                    throw ParseException(this->in, this->idx - 1, "missing comma ',' while parsing dictionary");
                this->ws();
            }

            std::size_t savedIdx = this->idx;
            auto newEntry = this->parseDictionaryEntry();
            auto duplicateKey = [&newEntry](const auto &entry) { return newEntry.first == entry.first; };
            if (std::find_if(elems.begin(), elems.end(), duplicateKey) != elems.end())
                throw ParseException(this->in, savedIdx, "duplicate dictionary key: " + newEntry.first);
            elems.push_back(newEntry);

            this->ws();
        }
        this->eat();    // eat '}'

        return ast::NodeDictionary::create(elems);
    }

    std::shared_ptr<const ast::NodeDataclass> Parser::parseDataclass() {
        auto className = this->parseIdentifier();
        if (!className.has_value())
            return nullptr;

        this->ws();
        // '()' can be skipped for dataclass with no arguments
        if (this->peek() != '(')
            return ast::NodeDataclass::create(*className, ast::NodeArray::create(), ast::NodeDictionary::create());

        this->eat();   // eat '('
        this->ws();
        std::vector<std::shared_ptr<const ast::Node>> positionalArguments;
        std::vector<std::pair<std::string, std::shared_ptr<const ast::Node>>> keywordArguments;
        while (this->peek() != ')') {
            if (this->peek() == EOF)
                throw ParseException(this->in, this->idx, "unexpected EOF while parsing dataclass");

            if (!positionalArguments.empty() || !keywordArguments.empty()) {
                if (this->eat() != ',')
                    throw ParseException(this->in, this->idx - 1, "missing comma ',' while parsing dataclass");
                this->ws();
            }

            this->parseArgument(positionalArguments, keywordArguments);

            this->ws();
        }
        this->eat();    // eat ')'

        auto positionalNode = ast::NodeArray::create(std::move(positionalArguments));
        auto keywordNode = ast::NodeDictionary::create(keywordArguments);
        return ast::NodeDataclass::create(*className, std::move(positionalNode), std::move(keywordNode));
    }

    std::shared_ptr<const ast::NodeString> Parser::parseString() {
        if (this->peek() != '"')
            return nullptr;
        this->eat();

        std::ostringstream string;
        int c;
        while ((c = this->eat()) != '"') {
            if (c == EOF)
                throw ParseException(this->in, this->idx, "unexpected EOF while parsing string");

            if (c == '"')
                break;

            if (c == '\\') {
                int escaped = this->eat();
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
                    throw ParseException(this->in, this->idx - 2,
                                         std::string("'") + static_cast<char>(escaped) + "' cannot follow backslash");
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

        // If it couldn't parse as float, so it couldn't as int
        if (float_ == nullptr)
            return nullptr;

        // How we tell if it is int or float?
        // If float parsing ate more characters, then it is float.
        // But if float and int ate the same, it is int.
        if (floatIdx > intIdx) {
            this->idx = floatIdx;
            return float_;
        } else {
            this->idx = intIdx;
            return int_;
        }
    }

    std::shared_ptr<const ast::NodeInt> Parser::parseInt() {
        // Add missing binary auto-detect to std::stdtol
        int base = 0;
        if (this->autodetectedBinary())
            base = 2;

        const char *startPtr = this->in.c_str() + this->idx;
        char *endPtr;
        errno = 0;
        long int i = std::strtol(startPtr, &endPtr, base);

        if (startPtr == endPtr)
            return nullptr;
        if (errno == ERANGE)
            throw ParseException(this->in, this->idx, "integer out of range");

        std::size_t eatenChars = endPtr - startPtr;
        this->idx += eatenChars;
        return ast::NodeInt::create(i);
    }

    std::shared_ptr<const ast::NodeFloat> Parser::parseFloat() {
        const char *startPtr = this->in.c_str() + this->idx;
        char *endPtr;
        errno = 0;
        double d = std::strtod(startPtr, &endPtr);

        if (startPtr == endPtr)
            return nullptr;
        if (errno == ERANGE)
            throw ParseException(this->in, this->idx, "float out of range");
        
        if (std::isnan(d) || std::isinf(d))
            throw ParseException(this->in, this->idx, "NaN/inf is not supported");

        std::size_t eatenChars = endPtr - startPtr;
        this->idx += eatenChars;
        return ast::NodeFloat::create(d);
    }

    std::optional<std::string> Parser::parseIdentifier() {
        std::ostringstream name;

        auto isFirstNameChar = [](int c) { return std::isalpha(c) || c == '_'; };
        if (!isFirstNameChar(this->peek()))
            return std::nullopt;
        name << static_cast<char>(this->eat());

        auto isNameChar = [](int c) { return std::isalnum(c) || c == '_'; };
        while (isNameChar(this->peek()))
            name << static_cast<char>(this->eat());

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

    int Parser::eat() {
        if (this->eof())
            return EOF;
        return this->in[this->idx++];
    }

    void Parser::ws() {
        while (std::isspace(this->peek()))
            this->eat();
    }

    void Parser::parseArgument(std::vector<std::shared_ptr<const ast::Node>> &positionalArguments,
                               std::vector<std::pair<std::string, std::shared_ptr<const ast::Node>>> &keywordArguments)
    {
        std::size_t savedIdx = this->idx;
        auto keywordArgument = this->parseKeywordArgument();
        if (keywordArgument.has_value()) {
            auto duplicateName = [&keywordArgument](const auto &arg) {
                return keywordArgument->first == arg.first;
            };
            if (std::find_if(keywordArguments.begin(), keywordArguments.end(), duplicateName)
                             != keywordArguments.end())
            {
                throw ParseException(this->in, savedIdx, "duplicate keyword argument: " + keywordArgument->first);
            }
            keywordArguments.push_back(std::move(*keywordArgument));
            return;
        }

        savedIdx = this->idx;
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

        this->eat();   // eat '='
        this->ws();
        auto node = this->parseExpression();
        if (node == nullptr) {
            this->idx = savedIdx;
            return std::nullopt;
        }

        return std::make_pair(*argumentName, std::move(node));
    }

    std::pair<std::string, std::shared_ptr<const ast::Node>> Parser::parseDictionaryEntry() {
        auto stringNode = this->parseString();
        if (stringNode == nullptr)
            throw ParseException(this->in, this->idx, "expecting string as a dictionary key");

        this->ws();
        if (this->peek() == EOF)
            throw ParseException(this->in, this->idx, "unexpected EOF while parsing dictionary");
        if (this->eat() != ':')
            throw ParseException(this->in, this->idx - 1, "missing colon ':' after key in dictionary");

        auto valueNode = this->parseExpression();

        return std::make_pair(stringNode->getValue(), std::move(valueNode));
    }

    bool Parser::autodetectedBinary() {
        // Two characters for "0b" and the third one for at least one digit
        if (this->idx + 3 > this->in.length())
            return false;

        if (this->in.substr(this->idx, 2) != "0b")
            return false;

        this->idx += 2;
        return true;
    }
} // pyon