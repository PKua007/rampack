//
// Created by pkua on 10.12.22.
//

#include <algorithm>

#include "MatcherString.h"
#include "utils/Utils.h"


namespace pyon::matcher {
    MatcherString::MatcherString(const std::string &str) {
        this->equals(str);
    }

    MatcherString::MatcherString(std::initializer_list<std::string> values) {
        this->anyOf(values);
    }

    bool MatcherString::match(std::shared_ptr<const ast::Node> node, Any &result) const {
        if (node->getType() != ast::Node::STRING)
            return false;

        const std::string &str = node->as<ast::NodeString>()->getValue();
        for (const auto &filter: this->filters)
            if (!filter(str))
                return false;

        result = this->mapping(str);
        return true;
    }

    MatcherString &MatcherString::mapTo(const std::function<Any(const std::string &)> &mapping_) {
        this->mapping = mapping_;
        return *this;
    }

    MatcherString &MatcherString::filter(const std::function<bool(const std::string &)> &filter) {
        this->filters.push_back(filter);
        return *this;
    }

    MatcherString &MatcherString::equals(const std::string &expected) {
        this->filters.emplace_back([expected](const std::string &str) { return str == expected; });
        return *this;
    }

    MatcherString &MatcherString::anyOf(const std::vector<std::string> &values) {
        auto filter = [values](const std::string &str) {
            return std::find(values.begin(), values.end(), str) != values.end();
        };
        this->filters.emplace_back(filter);
        return *this;
    }

    MatcherString &MatcherString::startsWith(const std::string &prefix) {
        this->filters.emplace_back([prefix](const std::string &str) { return ::startsWith(str, prefix); });
        return *this;
    }

    MatcherString &MatcherString::endsWith(const std::string &suffix) {
        this->filters.emplace_back([suffix](const std::string &str) { return ::endsWith(str, suffix); });
        return *this;
    }

    MatcherString &MatcherString::contains(const std::string &fragment) {
        auto filter = [fragment](const std::string &str) {
            return str.find(fragment) != std::string::npos;
        };
        this->filters.emplace_back(filter);
        return *this;
    }

    MatcherString &MatcherString::length(std::size_t len) {
        this->filters.emplace_back([len](const std::string &str) { return str.length() == len; });
        return *this;
    }

    MatcherString &MatcherString::empty() {
        this->filters.emplace_back([](const std::string &str) { return str.empty(); });
        return *this;
    }

    MatcherString &MatcherString::nonEmpty() {
        this->filters.emplace_back([](const std::string &str) { return !str.empty(); });
        return *this;
    }

    MatcherString &MatcherString::containsOnlyCharacters(const std::string &chars) {
        auto isCharAllowed = [chars](char c) { return chars.find(c) != std::string::npos; };
        return this->containsOnlyCharacters(isCharAllowed);
    }

    MatcherString &MatcherString::containsOnlyCharacters(const std::function<bool(char)> &charPredicate) {
        auto filter = [charPredicate](const std::string &str) {
            return std::all_of(str.begin(), str.end(), charPredicate);
        };
        this->filters.emplace_back(filter);
        return *this;
    }

    MatcherString &MatcherString::uniqueCharacters() {
        auto filter = [](const std::string &str) {
            auto strCopy = str;
            std::sort(strCopy.begin(), strCopy.end());
            return std::unique(strCopy.begin(), strCopy.end()) == strCopy.end();
        };
        this->filters.emplace_back(filter);
        return *this;
    }

    MatcherString &MatcherString::lowercase() {
        return this->containsOnlyCharacters([](char c) { return std::islower(c); });
    }

    MatcherString &MatcherString::uppercase() {
        return this->containsOnlyCharacters([](char c) { return std::isupper(c); });
    }

    MatcherString &MatcherString::numeric() {
        return this->containsOnlyCharacters([](char c) { return std::isdigit(c); });
    }

    MatcherString &MatcherString::alpha() {
        return this->containsOnlyCharacters([](char c) { return std::isalpha(c); });
    }

    MatcherString &MatcherString::alphanumeric() {
        return this->containsOnlyCharacters([](char c) { return std::isalnum(c); });
    }
} // matcher