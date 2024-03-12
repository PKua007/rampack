//
// Created by pkua on 10.12.22.
//

#ifndef RAMPACK_MATCHERSTRING_H
#define RAMPACK_MATCHERSTRING_H

#include <functional>
#include <vector>
#include <string>

#include "MatcherBase.h"
#include "NodeLiteral.h"


namespace pyon::matcher {
    class MatcherString : public MatcherBase {
    public:
        struct Filter {
            std::function<bool(const std::string&)> predicate;
            std::string description;
        };

    private:
        std::vector<Filter> filters;
        std::function<Any(const std::string&)> mapping = [](const std::string &str) { return str; };

        [[nodiscard]] std::string generateUnmatchedReport(const std::string &reason) const;

    public:
        MatcherString() = default;

        [[nodiscard]] MatcherString copy() const { return *this; }

        explicit MatcherString(const std::string &str);
        MatcherString(std::initializer_list<std::string> values);

        MatchReport match(std::shared_ptr<const ast::Node> node, Any &result) const override;
        [[nodiscard]] bool matchNodeType(ast::Node::Type type) const override { return type == ast::Node::STRING; }
        [[nodiscard]] std::string outline(std::size_t indent) const override;
        [[nodiscard]] std::string synopsis() const override { return "String"; }

        template<typename T>
        MatcherString &mapTo() {
            this->mapping = [](const std::string &str) { return static_cast<T>(str); };
            return *this;
        }

        MatcherString &mapTo(const std::function<Any(const std::string&)> &mapping_);
        MatcherString &filter(const std::function<bool(const std::string&)> &filter);
        MatcherString &describe(const std::string &description);
        MatcherString &equals(const std::string &expected);
        MatcherString &anyOf(const std::vector<std::string> &values);
        MatcherString &startsWith(const std::string &prefix);
        MatcherString &endsWith(const std::string &suffix);
        MatcherString &contains(const std::string &fragment);
        MatcherString &length(std::size_t len);
        MatcherString &empty();
        MatcherString &nonEmpty();
        MatcherString &containsOnlyCharacters(const std::string &chars);
        MatcherString &containsOnlyCharacters(const std::function<bool(char)> &charPredicate);
        MatcherString &uniqueCharacters();
        MatcherString &lowercase();
        MatcherString &uppercase();
        MatcherString &numeric();
        MatcherString &alpha();
        MatcherString &alphanumeric();

        [[nodiscard]] const std::vector<Filter> &getFilters() const { return this->filters; }
        [[nodiscard]] const std::function<Any(const std::string&)> &getMapping() const { return this->mapping; }
    };
} // matcher

#endif //RAMPACK_MATCHERSTRING_H
