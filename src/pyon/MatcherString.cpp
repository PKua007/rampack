//
// Created by pkua on 10.12.22.
//

#include <algorithm>
#include <iomanip>
#include <sstream>

#include "MatcherString.h"
#include "utils/Utils.h"


namespace pyon::matcher {
    std::string MatcherString::quoted(const std::string &string) {
        std::ostringstream out;
        out << std::quoted(string);
        return out.str();
    }

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
            if (!filter.predicate(str))
                return false;

        result = this->mapping(str);
        return true;
    }

    std::string MatcherString::outline(std::size_t indent) const {
        std::string spaces(indent, ' ');
        std::ostringstream out;
        out << spaces << "String";
        if (this->filters.size() == 1) {
            out << ", " << this->filters.front().description;
        } else if (this->filters.size() > 1) {
            out << ":";
            for (const auto &filter : this->filters)
                out << std::endl << spaces << "- " <<
                    filter.description;
        }
        return out.str();
    }

    MatcherString &MatcherString::mapTo(const std::function<Any(const std::string &)> &mapping_) {
        this->mapping = mapping_;
        return *this;
    }

    MatcherString &MatcherString::filter(const std::function<bool(const std::string &)> &filter) {
        this->filters.push_back({filter, "<undefined filter>"});
        return *this;
    }

    MatcherString &MatcherString::describe(const std::string &description) {
        Expects(!this->filters.empty());
        this->filters.back().description = description;
        return *this;
    }

    MatcherString &MatcherString::equals(const std::string &expected) {
        this->filter([expected](const std::string &str) { return str == expected; });
        this->describe("= " + MatcherString::quoted(expected));
        return *this;
    }

    MatcherString &MatcherString::anyOf(const std::vector<std::string> &values) {
        auto filter = [values](const std::string &str) {
            return std::find(values.begin(), values.end(), str) != values.end();
        };
        this->filter(filter);

        std::ostringstream out;
        out << "one of: ";
        for (std::size_t i{}; i < values.size() - 1; i++)
            out << MatcherString::quoted(values[i]) << ", ";
        out << MatcherString::quoted(values.back());
        this->describe(out.str());

        return *this;
    }

    MatcherString &MatcherString::startsWith(const std::string &prefix) {
        this->filter([prefix](const std::string &str) { return ::startsWith(str, prefix); });
        this->describe("starting with " + MatcherString::quoted(prefix));
        return *this;
    }

    MatcherString &MatcherString::endsWith(const std::string &suffix) {
        this->filter([suffix](const std::string &str) { return ::endsWith(str, suffix); });
        this->describe("ending with " + MatcherString::quoted(suffix));
        return *this;
    }

    MatcherString &MatcherString::contains(const std::string &fragment) {
        auto filter = [fragment](const std::string &str) {
            return str.find(fragment) != std::string::npos;
        };
        this->filter(filter);
        this->describe("containing " + MatcherString::quoted(fragment));
        return *this;
    }

    MatcherString &MatcherString::length(std::size_t len) {
        this->filter([len](const std::string &str) { return str.length() == len; });
        this->describe("with length = " + std::to_string(len));
        return *this;
    }

    MatcherString &MatcherString::empty() {
        this->filter([](const std::string &str) { return str.empty(); });
        this->describe("empty");
        return *this;
    }

    MatcherString &MatcherString::nonEmpty() {
        this->filter([](const std::string &str) { return !str.empty(); });
        this->describe("non-empty");
        return *this;
    }

    MatcherString &MatcherString::containsOnlyCharacters(const std::string &chars) {
        auto isCharAllowed = [chars](char c) { return chars.find(c) != std::string::npos; };
        this->containsOnlyCharacters(isCharAllowed);
        this->describe("with only characters: " + MatcherString::quoted(chars));
        return *this;
    }

    MatcherString &MatcherString::containsOnlyCharacters(const std::function<bool(char)> &charPredicate) {
        auto filter = [charPredicate](const std::string &str) {
            return std::all_of(str.begin(), str.end(), charPredicate);
        };
        this->filter(filter);
        this->describe("with only characters: <undefined predicate>");
        return *this;
    }

    MatcherString &MatcherString::uniqueCharacters() {
        auto filter = [](const std::string &str) {
            auto strCopy = str;
            std::sort(strCopy.begin(), strCopy.end());
            return std::unique(strCopy.begin(), strCopy.end()) == strCopy.end();
        };
        this->filter(filter);
        this->describe("with unique characters");
        return *this;
    }

    MatcherString &MatcherString::lowercase() {
        this->containsOnlyCharacters([](char c) { return std::islower(c); });
        this->describe("lowercase");
        return *this;
    }

    MatcherString &MatcherString::uppercase() {
        this->containsOnlyCharacters([](char c) { return std::isupper(c); });
        this->describe("uppercase");
        return *this;
    }

    MatcherString &MatcherString::numeric() {
        this->containsOnlyCharacters([](char c) { return std::isdigit(c); });
        this->describe("with only numbers");
        return *this;
    }

    MatcherString &MatcherString::alpha() {
        this->containsOnlyCharacters([](char c) { return std::isalpha(c); });
        this->describe("with only letters");
        return *this;
    }

    MatcherString &MatcherString::alphanumeric() {
        this->containsOnlyCharacters([](char c) { return std::isalnum(c); });
        this->describe("with only numbers and letters");
        return *this;
    }
} // matcher