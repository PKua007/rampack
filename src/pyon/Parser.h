//
// Created by pkua on 06.12.22.
//

#ifndef RAMPACK_PARSER_H
#define RAMPACK_PARSER_H

#include <sstream>
#include <optional>
#include <utility>

#include "PyonException.h"
#include "AST.h"


namespace pyon {
    class ParseException : public PyonException {
    private:
        static std::string constructMsg(const std::string &in, std::size_t pos, std::string const &msg) {
            if (pos >= in.length())
                return "pyon parse exception: " + msg + "\n\n\"\n" + in + ">>>>>>\n\"";
            return "pyon parse exception: " + msg + "\n\n\"\n" + in.substr(0, pos) + ">>>>>>" + in.substr(pos) + "\n\"";
        }

    public:
        ParseException(const std::string &in, std::size_t pos, const std::string &msg)
            : PyonException(ParseException::constructMsg(in, pos, msg))
        { }
    };

    class Parser {
    private:
        std::string in;
        std::size_t idx{};

        explicit Parser(std::string in) : in{std::move(in)} { }

        void ws();
        [[nodiscard]] bool eof() const { return this->idx >= this->in.length(); }
        [[nodiscard]] int peek() const;
        int eat();
        void eatComment();
        void throwIfAnythingLeft();

        std::shared_ptr<const ast::Node> parseExpression();
        std::shared_ptr<const ast::NodeArray> parseArray();
        std::shared_ptr<const ast::NodeDictionary> parseDictionary();
        std::shared_ptr<const ast::NodeDataclass> parseDataclass();
        std::shared_ptr<const ast::NodeString> parseString();
        std::shared_ptr<const ast::Node> parseBooleanNone();
        std::shared_ptr<const ast::Node> parseNumeral();
        std::shared_ptr<const ast::NodeInt> parseInt();
        std::shared_ptr<const ast::NodeFloat> parseFloat();
        std::optional<std::string> parseIdentifier();

        std::pair<std::string, std::shared_ptr<const ast::Node>>
        parseDictionaryEntry();

        void parseArgument(std::vector<std::shared_ptr<const ast::Node>> &positionalArguments,
                           std::vector<std::pair<std::string, std::shared_ptr<const ast::Node>>> &keywordArguments);

        std::optional<std::pair<std::string, std::shared_ptr<const ast::Node>>>
        parseKeywordArgument();

        bool autodetectedBinary();

    public:
        static std::shared_ptr<const ast::Node> parse(const std::string &expression);
        static std::shared_ptr<const ast::Node> parse(std::istream &expression);
    };
} // pyon

#endif //RAMPACK_PARSER_H
