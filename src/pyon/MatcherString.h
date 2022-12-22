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
    private:
        struct Filter {
            std::function<bool(const std::string&)> predicate;
            std::string description;
        };

        std::vector<Filter> filters;
        std::function<Any(const std::string&)> mapping = [](const std::string &str) { return str; };

    public:
        MatcherString() = default;

        [[nodiscard]] MatcherString copy() const { return *this; }

        explicit MatcherString(const std::string &str);
        MatcherString(std::initializer_list<std::string> values);

        bool match(std::shared_ptr<const ast::Node> node, Any &result) const override;
        [[nodiscard]] std::string outline(std::size_t indent) const override;

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
    };
} // matcher

#endif //RAMPACK_MATCHERSTRING_H
