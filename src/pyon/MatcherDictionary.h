//
// Created by pkua on 11.12.22.
//

#ifndef RAMPACK_MATCHERDICTIONARY_H
#define RAMPACK_MATCHERDICTIONARY_H

#include <vector>
#include <functional>
#include <map>
#include <memory>

#include "MatcherBase.h"
#include "NodeDictionary.h"


namespace pyon::matcher {
    class DictionaryData {
    private:
        std::map<std::string, Any> map;
        std::vector<std::string> keys;

    public:
        using iterator = decltype(map)::const_iterator;

        DictionaryData() = default;
        explicit DictionaryData(std::map<std::string, Any> map);

        [[nodiscard]] bool empty() const { return this->map.empty(); }
        [[nodiscard]] std::size_t size() const { return this->map.size(); }
        [[nodiscard]] const Any &at(const std::string &key) const;
        [[nodiscard]] const Any &operator[](const std::string &key) const { return this->at(key); };
        [[nodiscard]] bool hasKey(const std::string &key) const { return this->map.find(key) != this->map.end(); }
        [[nodiscard]] const std::vector<std::string> &getKeys() const { return this->keys; }
        [[nodiscard]] iterator begin() const { return this->map.begin(); }
        [[nodiscard]] iterator end() const { return this->map.end(); }

        template<typename T>
        [[nodiscard]] std::map<std::string, T> asStdMap() const {
            std::map<std::string, T> result;
            using Pair = std::pair<std::string, Any>;
            auto transformer = [](const Pair &elem) { return std::make_pair(elem.first, elem.second.as<T>()); };
            std::transform(this->begin(), this->end(), std::inserter(result, result.end()), transformer);
            return result;
        }
    };

    class MatcherDictionary : public MatcherBase {
    private:
        struct KeyMatcher {
            std::function<bool(const std::string &)> keyPredicate;
            std::shared_ptr<MatcherBase> matcher;
        };

        std::vector<std::function<bool(const DictionaryData&)>> filters;
        std::function<Any(const DictionaryData&)> mapping = [](const DictionaryData &dict) { return dict; };
        std::shared_ptr<MatcherBase> defaultMatcher;
        std::vector<KeyMatcher> keyMatchers;

    public:
        MatcherDictionary() = default;
        template <typename ConcreteMatcher>
        explicit MatcherDictionary(const ConcreteMatcher &matcher) {
            this->valuesMatch(matcher);
        }

        bool match(std::shared_ptr<const ast::Node> node, Any &result) const override;

        template <typename ConcreteMatcher>
        MatcherDictionary &valuesMatch(const ConcreteMatcher &matcher) {
            static_assert(std::is_base_of_v<MatcherBase, ConcreteMatcher>,
                          "ConcreteMatcher template parameter is not a matcher derived from MatcherBase");
            // Copy construct the matcher
            this->defaultMatcher = std::make_shared<ConcreteMatcher>(matcher);
            return *this;
        }

        template <typename ConcreteMatcher>
        MatcherDictionary &valueAtKeyMatches(const std::function<bool(const std::string&)> &keyPredicate,
                                             const ConcreteMatcher &matcher)
        {
            static_assert(std::is_base_of_v<MatcherBase, ConcreteMatcher>,
                          "ConcreteMatcher template parameter is not a matcher derived from MatcherBase");
            // Copy construct the matcher
            this->keyMatchers.push_back({keyPredicate, std::make_shared<ConcreteMatcher>(matcher)});
            return *this;
        }

        template <typename ConcreteMatcher>
        MatcherDictionary &valueAtKeyMatches(const std::string &key_, const ConcreteMatcher &matcher) {
            this->valueAtKeyMatches([key_](const std::string &key) { return key == key_; }, matcher);
            return *this;
        }

        template<typename T>
        MatcherDictionary &mapToStdMap() {
            this->mapping = [](const DictionaryData &dict) { return dict.template asStdMap<T>(); };
            return *this;
        }

        MatcherDictionary &mapTo(const std::function<Any(const DictionaryData&)> &mapping_);
        MatcherDictionary &mapToDefault();
        MatcherDictionary &filter(const std::function<bool(const DictionaryData&)> &filter);
        MatcherDictionary &hasKeys(const std::vector<std::string> &keys);
        MatcherDictionary &hasOnlyKeys(const std::vector<std::string> &keys);
        MatcherDictionary &hasNotKeys(const std::vector<std::string> &keys);
        MatcherDictionary &keysMatch(const std::function<bool(const std::string&)> &predicate);
        MatcherDictionary &empty();
        MatcherDictionary &size(std::size_t size);
        MatcherDictionary &sizeAtLeast(std::size_t size);
        MatcherDictionary &sizeAtMost(std::size_t size);
        MatcherDictionary &sizeInRange(std::size_t low, std::size_t high);
    };
} // matcher

#endif //RAMPACK_MATCHERDICTIONARY_H
