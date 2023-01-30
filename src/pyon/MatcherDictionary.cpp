//
// Created by pkua on 11.12.22.
//

#include <sstream>
#include <optional>
#include <iomanip>

#include "MatcherDictionary.h"
#include "utils/Exceptions.h"
#include "utils/Utils.h"


namespace pyon::matcher {
    const Any &DictionaryData::at(const std::string &key) const {
        auto it = this->map.find(key);
        if (it == this->map.end()) {
            auto msg = "pyon::matcher::DictionaryData::at: There is no '" + key + "' in map";
            throw ast::DictionaryNoSuchKeyException(msg);
        }
        return it->second;
    }

    DictionaryData::DictionaryData(std::map<std::string, Any> map) {
        this->keys.reserve(map.size());
        auto keyRetriever = [](const auto &pair) { return pair.first; };
        std::transform(map.begin(), map.end(), std::back_inserter(this->keys), keyRetriever);
        std::swap(this->map, map);
    }

    std::string MatcherDictionary::implodeKeys(const std::vector<std::string> &keys) {
        std::ostringstream out;
        for (std::size_t i{}; i < keys.size() - 1; i++)
            out << quoted(keys[i]) << ", ";
        out << quoted(keys.back());
        return out.str();
    }

    void MatcherDictionary::addKeyMatchers(std::vector<std::string> &bulletPoints, size_t &linesAtLeast,
                                           std::size_t indent) const
    {
        if (this->keyMatchers.empty()) {
            this->addDefaultKeyMatcher(bulletPoints, linesAtLeast, indent);
        } else {
            linesAtLeast = 2;   // If there are any key specific matchers, we for sure have multiline output
            this->addAllKeyMatchers(bulletPoints, indent);
        }
    }

    void MatcherDictionary::addDefaultKeyMatcher(std::vector<std::string> &bulletPoints, std::size_t &linesAtLeast,
                                                 std::size_t indent) const
    {
        if (this->defaultMatcher != nullptr) {
            std::string defaultMatcherOutline = this->defaultMatcher->outline(indent + 2);
            defaultMatcherOutline = "with values: " + defaultMatcherOutline.substr(indent + 2);
            linesAtLeast++;
            if (isMultiline(defaultMatcherOutline))
                linesAtLeast++;

            bulletPoints.push_back(defaultMatcherOutline);
        }
    }

    void MatcherDictionary::addAllKeyMatchers(std::vector<std::string> &bulletPoints, std::size_t indent) const {
        std::string spaces(indent, ' ');
        std::vector<std::pair<std::string, std::string>> keyValues = this->describeKeyValues(indent);

        auto keyLength = [](const auto &keyValue1, const auto &keyValue2) {
            return keyValue1.first.length() < keyValue2.first.length();
        };
        std::size_t maxKeyLength = std::max_element(keyValues.begin(), keyValues.end(), keyLength)->first.length();

        std::ostringstream bulletOut;
        bulletOut << "with values at keys:";
        for (const auto &keyValue : keyValues) {
            bulletOut << std::endl << spaces << "  - ";
            bulletOut << std::setw(static_cast<int>(maxKeyLength)) << std::left << keyValue.first << " : ";
            bulletOut << keyValue.second;
        }
        bulletPoints.push_back(bulletOut.str());
    }

    std::vector<std::pair<std::string, std::string>> MatcherDictionary::describeKeyValues(std::size_t indent) const {
        std::vector<std::pair<std::string, std::string>> keyValues;

        for (const auto &keyMatcher : this->keyMatchers) {
            std::string keyOutline = keyMatcher.matcher->outline(indent + 4);
            keyOutline = keyOutline.substr(indent + 4);
            keyValues.emplace_back(keyMatcher.keyDescription, keyOutline);
        }

        if (this->defaultMatcher != nullptr) {
            std::string defaultMatcherOutline = this->defaultMatcher->outline(indent + 4);
            defaultMatcherOutline = defaultMatcherOutline.substr(indent + 4);
            keyValues.emplace_back("other keys", defaultMatcherOutline);
        }
        return keyValues;
    }

    void MatcherDictionary::addFilters(std::vector<std::string> &bulletPoints, std::size_t &linesAtLeast) const {
        linesAtLeast += this->filters.size();   // each filter adds one line
        for (const auto &filter : this->filters)
            bulletPoints.push_back(filter.description);
    }

    std::string MatcherDictionary::doPrintOutline(const std::vector<std::string> &bulletPoints,
                                                  std::size_t linesAtLeast, std::size_t indent) const
    {
        std::ostringstream out;
        std::string spaces(indent, ' ');

        if (linesAtLeast == 0) {
            // No specification
            Assert(bulletPoints.empty());
            out << spaces << "Dictionary";
        } else if (linesAtLeast == 1) {
            // Specification can be shown inline with "Dictionary"
            Assert(bulletPoints.size() == 1);
            out << spaces << "Dictionary, " << bulletPoints.back();
        } else {
            // Specification is split between multiple lines
            out << spaces << "Dictionary:";
            for (const auto &bulletPoint : bulletPoints)
                out << std::endl << spaces << "- " << bulletPoint;
        }

        return out.str();
    }

    std::string MatcherDictionary::generateDictionaryUnmatchedReport(const std::string &reason) const {
        std::ostringstream out;
        out << "Matching Dictionary failed:" << std::endl;
        out << "✖ " << reason << std::endl;
        out << "✓ Expected format: " << this->synopsis();
        return out.str();
    }

    std::string MatcherDictionary::generateUnmetConditionReport(const std::string &condition) const {
        std::ostringstream out;
        out << "Matching Dictionary failed:" << std::endl;
        out << "✖ Condition not satisfied: " << condition;
        return out.str();
    }

    std::string MatcherDictionary::generateKeyUnmatchedReport(const std::string &key, const std::string &reason) const {
        std::ostringstream out;
        out << "Matching Dictionary failed: Matching key \"" << key << "\" failed:" << std::endl;
        out << "✖ " << replaceAll(reason, "\n", "\n  ");
        return out.str();
    }

    MatchReport MatcherDictionary::matchKeys(const ast::NodeDictionary &nodeDict, DictionaryData &dictData) const {
        std::map<std::string, Any> dictDataMap;
        for (const auto &nodeDictElem: nodeDict) {
            Any elemResult;
            MatchReport matchReport = this->matchKey(nodeDictElem.first, nodeDictElem.second, elemResult);
            if (!matchReport)
                return matchReport.getReason();
            dictDataMap[nodeDictElem.first] = elemResult;
        }

        dictData = DictionaryData(std::move(dictDataMap));
        return true;
    }

    MatchReport MatcherDictionary::matchKey(const std::string &key, const std::shared_ptr<const ast::Node> &value,
                                            Any &elemResult) const
    {
        for (const auto &keyMatcher : this->keyMatchers) {
            if (keyMatcher.keyPredicate(key)) {
                auto matchReport = keyMatcher.matcher->match(value, elemResult);
                if (!matchReport)
                    return this->generateKeyUnmatchedReport(key, matchReport.getReason());
                return true;
            }
        }

        if (this->defaultMatcher == nullptr) {
            elemResult = value;
            return true;
        }

        auto matchReport = this->defaultMatcher->match(value, elemResult);
        if (!matchReport)
            return this->generateKeyUnmatchedReport(key, matchReport.getReason());
        return true;
    }

    MatchReport MatcherDictionary::match(std::shared_ptr<const ast::Node> node, Any &result) const {
        if (node->getType() != ast::Node::DICTIONARY)
            return this->generateDictionaryUnmatchedReport("Got incorrect node type: " + node->getNodeName());

        const auto &nodeDict = node->as<ast::NodeDictionary>();
        DictionaryData dictData;
        auto matchReport = this->matchKeys(*nodeDict, dictData);
        if (!matchReport)
            return matchReport;

        for (const auto &filter: this->filters)
            if (!filter.predicate(dictData))
                return this->generateUnmetConditionReport( filter.description);

        result = this->mapping(dictData);
        return true;
    }

    std::string MatcherDictionary::outline(std::size_t indent) const {
        std::size_t linesAtLeast{};     // How much lines has the description (for 2+ lines the number may be not exact)
        std::vector<std::string> bulletPoints;
        this->addKeyMatchers(bulletPoints, linesAtLeast, indent);
        this->addFilters(bulletPoints, linesAtLeast);
        return this->doPrintOutline(bulletPoints, linesAtLeast, indent);
    }

    MatcherDictionary &MatcherDictionary::describeKey(const std::string &description) {
        Expects(!this->keyMatchers.empty());
        this->keyMatchers.back().keyDescription = description;
        return *this;
    }

    MatcherDictionary &MatcherDictionary::mapTo(const std::function<Any(const DictionaryData &)> &mapping_) {
        this->mapping = mapping_;
        return *this;
    }

    MatcherDictionary &MatcherDictionary::mapToDefault() {
        this->mapping = [](const DictionaryData &dict) { return dict; };
        return *this;
    }

    MatcherDictionary &MatcherDictionary::filter(const std::function<bool(const DictionaryData &)> &filter) {
        this->filters.push_back({filter, "<undefined filter>"});
        return *this;
    }

    MatcherDictionary &MatcherDictionary::describe(const std::string &description) {
        Expects(!this->filters.empty());
        this->filters.back().description = description;
        return *this;
    }

    MatcherDictionary &MatcherDictionary::keysMatch(const std::function<bool(const std::string &)> &predicate) {
        auto keysMatcher = [predicate](const DictionaryData &dict) {
            const auto &keys = dict.getKeys();
            return std::all_of(keys.begin(), keys.end(), predicate);
        };
        this->filter(keysMatcher);
        this->describe("keys match <undefined predicate>");
        return *this;
    }

    MatcherDictionary &MatcherDictionary::hasKeys(const std::vector<std::string> &keys) {
        auto keysMatcher = [keys](const DictionaryData &dict) {
            auto isPresent = [&dict](const std::string &key) { return dict.hasKey(key); };
            return std::all_of(keys.begin(), keys.end(), isPresent);
        };
        this->filter(keysMatcher);
        this->describe("mandatory keys: " + MatcherDictionary::implodeKeys(keys));
        return *this;
    }

    MatcherDictionary &MatcherDictionary::hasNotKeys(const std::vector<std::string> &keys) {
        auto keysMatcher = [keys](const DictionaryData &dict) {
            auto isNotPresent = [&dict](const std::string &key) { return !dict.hasKey(key); };
            return std::all_of(keys.begin(), keys.end(), isNotPresent);
        };
        this->filter(keysMatcher);
        this->describe("forbidden keys: " + MatcherDictionary::implodeKeys(keys));
        return *this;
    }

    MatcherDictionary &MatcherDictionary::hasOnlyKeys(const std::vector<std::string> &keys) {
        auto areKeysAllowed = [keys](const DictionaryData &dict) {
            auto isKeyAllowed = [keys](const std::string &key) {
                return std::find(keys.begin(), keys.end(), key) != keys.end();
            };
            const auto &dictKeys = dict.getKeys();
            return std::all_of(dictKeys.begin(), dictKeys.end(), isKeyAllowed);
        };
        this->filter(areKeysAllowed);
        this->describe("allowed keys: " + MatcherDictionary::implodeKeys(keys));
        return *this;
    }

    MatcherDictionary &MatcherDictionary::empty() {
        this->filter([](const DictionaryData &dict) { return dict.empty(); });
        this->describe("empty");
        return *this;
    }

    MatcherDictionary &MatcherDictionary::nonEmpty() {
        this->filter([](const DictionaryData &dict) { return !dict.empty(); });
        this->describe("non-empty");
        return *this;
    }

    MatcherDictionary &MatcherDictionary::size(std::size_t size) {
        this->filter([size](const DictionaryData &dict) { return dict.size() == size; });
        this->describe("with size = " + std::to_string(size));
        return *this;
    }

    MatcherDictionary &MatcherDictionary::sizeAtLeast(std::size_t size) {
        this->filter([size](const DictionaryData &dict) { return dict.size() >= size; });
        this->describe("with size >= " + std::to_string(size));
        return *this;
    }

    MatcherDictionary &MatcherDictionary::sizeAtMost(std::size_t size) {
        this->filter([size](const DictionaryData &dict) { return dict.size() <= size; });
        this->describe("with size <= " + std::to_string(size));
        return *this;
    }

    MatcherDictionary &MatcherDictionary::sizeInRange(std::size_t low, std::size_t high) {
        Expects(low <= high);
        auto isSizeInRange = [low, high](const DictionaryData &dict) {
            return dict.size() >= low && dict.size() <= high;
        };
        this->filter(isSizeInRange);
        this->describe("with size in range [" + std::to_string(low) + ", " + std::to_string(high) + "]");
        return *this;
    }

    std::string MatcherDictionary::synopsis() const {
        if (this->defaultMatcher == nullptr && this->keyMatchers.empty())
            return "Dictionary[String -> any expression]";
        else if (this->defaultMatcher != nullptr && this->keyMatchers.empty())
            return "Dictionary[String -> " + this->defaultMatcher->synopsis() + "]";
        else
            return "Dictionary[String -> various]";
    }
} // matcher