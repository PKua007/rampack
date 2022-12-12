//
// Created by pkua on 11.12.22.
//

#include "MatcherDictionary.h"
#include "utils/Assertions.h"


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

    bool MatcherDictionary::match(std::shared_ptr<const ast::Node> node, Any &result) const {
        if (node->getType() != ast::Node::DICTIONARY)
            return false;

        const auto &nodeDict = node->as<ast::NodeDictionary>();
        std::map<std::string, Any> dictDataMap;
        auto hintIt = dictDataMap.end();
        for (const auto &nodeDictElem: *nodeDict) {
            Any elemResult;

            bool matched = false;
            for (const auto &keyMatcher : this->keyMatchers) {
                if (keyMatcher.keyPredicate(nodeDictElem.first)) {
                    if (!keyMatcher.matcher->match(nodeDictElem.second, elemResult))
                        return false;
                    hintIt = dictDataMap.emplace_hint(hintIt, nodeDictElem.first, elemResult);
                    matched = true;
                    break;
                }
            }
            if (matched)
                continue;


            if (this->defaultMatcher != nullptr) {
                if (!this->defaultMatcher->match(nodeDictElem.second, elemResult))
                    return false;
                hintIt = dictDataMap.emplace_hint(hintIt, nodeDictElem.first, elemResult);
                continue;
            }

            hintIt = dictDataMap.emplace_hint(hintIt, nodeDictElem.first, nodeDictElem.second);
        }
        DictionaryData dictData(std::move(dictDataMap));

        for (const auto &filter: this->filters)
            if (!filter(dictData))
                return false;

        result = this->mapping(dictData);
        return true;
    }

    MatcherDictionary &MatcherDictionary::mapTo(const std::function<Any(const DictionaryData &)> &mapping_) {
        this->mapping = mapping_;
        return *this;
    }

    MatcherDictionary &MatcherDictionary::filter(const std::function<bool(const DictionaryData &)> &filter) {
        this->filters.push_back(filter);
        return *this;
    }

    MatcherDictionary &MatcherDictionary::keysMatch(const std::function<bool(const std::string &)> &predicate) {
        auto keysMatcher = [predicate](const DictionaryData &dict) {
            const auto &keys = dict.getKeys();
            return std::all_of(keys.begin(), keys.end(), predicate);
        };
        this->filters.emplace_back(keysMatcher);
        return *this;
    }

    MatcherDictionary &MatcherDictionary::hasKeys(const std::vector<std::string> &keys) {
        auto keysMatcher = [keys](const DictionaryData &dict) {
            auto isPresent = [&dict](const std::string &key) { return dict.hasKey(key); };
            return std::all_of(keys.begin(), keys.end(), isPresent);
        };
        this->filters.emplace_back(keysMatcher);
        return *this;
    }

    MatcherDictionary &MatcherDictionary::hasNotKeys(const std::vector<std::string> &keys) {
        auto keysMatcher = [keys](const DictionaryData &dict) {
            auto isNotPresent = [&dict](const std::string &key) { return !dict.hasKey(key); };
            return std::all_of(keys.begin(), keys.end(), isNotPresent);
        };
        this->filters.emplace_back(keysMatcher);
        return *this;
    }

    MatcherDictionary &MatcherDictionary::hasOnlyKeys(std::vector<std::string> keys) {
        std::sort(keys.begin(), keys.end());
        keys.erase(std::unique(keys.begin(), keys.end()), keys.end());
        this->filters.emplace_back([keys](const DictionaryData &dict) { return dict.getKeys() == keys; });
        return *this;
    }

    MatcherDictionary &MatcherDictionary::empty() {
        this->filters.emplace_back([](const DictionaryData &dict) { return dict.empty(); });
        return *this;
    }

    MatcherDictionary &MatcherDictionary::size(std::size_t size) {
        this->filters.emplace_back([size](const DictionaryData &dict) { return dict.size() == size; });
        return *this;
    }

    MatcherDictionary &MatcherDictionary::sizeAtLeast(std::size_t size) {
        this->filters.emplace_back([size](const DictionaryData &dict) { return dict.size() >= size; });
        return *this;
    }

    MatcherDictionary &MatcherDictionary::sizeAtMost(std::size_t size) {
        this->filters.emplace_back([size](const DictionaryData &dict) { return dict.size() <= size; });
        return *this;
    }

    MatcherDictionary &MatcherDictionary::sizeInRange(std::size_t low, std::size_t high) {
        Expects(low <= high);
        auto isSizeInRange = [low, high](const DictionaryData &dict) {
            return dict.size() >= low && dict.size() <= high;
        };
        this->filters.emplace_back(isSizeInRange);
        return *this;
    }
} // matcher