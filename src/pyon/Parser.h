//
// Created by pkua on 06.12.22.
//

#ifndef RAMPACK_PARSER_H
#define RAMPACK_PARSER_H

#include <sstream>
#include <optional>

#include "PyonException.h"
#include "AST.h"


namespace pyon {
    class ParseException : public PyonException {
    private:
        static std::string constructMsg(const std::string &in, std::size_t pos, std::string const &msg) {
            if (pos >= in.length())
                return "pyon parse exception: " + msg + "\n\n\"" + in + ">>>>\"";
            return "pyon parse exception: " + msg + "\n\n\"" + in.substr(0, pos) + ">>>>" + in.substr(pos) + "\"";
        }

    public:
        ParseException(const std::string &in, std::size_t pos, const std::string &msg)
            : PyonException(ParseException::constructMsg(in, pos, msg))
        { }
    };

    class Parser {
    private:
        std::istringstream in;

        explicit Parser(const std::string &in) : in{in} { }

        [[nodiscard]] std::size_t idx() { return this->in.tellg(); }
        void backtrack(std::size_t savedIdx);

        std::shared_ptr<const ast::Node> parseExpression();
        std::shared_ptr<const ast::Node> parseLiteral();
        std::shared_ptr<const ast::NodeArray> parseArray();
        std::shared_ptr<const ast::NodeDictionary> parseDictionary();
        std::shared_ptr<const ast::NodeDataclass> parseDataclass();
        std::shared_ptr<const ast::NodeString> parseString();
        std::shared_ptr<const ast::Node> parseBooleanNone();
        std::shared_ptr<const ast::Node> parseNumeral();
        std::shared_ptr<const ast::NodeInt> parseInt();
        std::shared_ptr<const ast::NodeFloat> parseFloat();
        std::optional<std::string> parseName();

    public:
        static std::shared_ptr<const ast::Node> parse(const std::string &expression);
        static std::shared_ptr<const ast::Node> parse(std::istream &expression);
    };
} // pyon

#endif //RAMPACK_PARSER_H
