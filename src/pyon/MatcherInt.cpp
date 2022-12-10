//
// Created by pkua on 10.12.22.
//

#include "MatcherInt.h"
#include "utils/Assertions.h"


namespace pyon::matcher {
    bool MatcherInt::match(std::shared_ptr<const ast::Node> node, Any &result) const {
        if (node->getType() != ast::Node::INT)
             return false;

        long i = node->as<ast::NodeInt>()->getValue();
        for (const auto &filter : filters)
             if (!filter(i))
                 return false;

        result = this->mapping(i);
        return true;
    }

    MatcherInt &MatcherInt::filter(const std::function<bool(long)> &filter) {
        this->filters.push_back(filter);
        return *this;
    }

    MatcherInt &MatcherInt::positive() {
        this->filters.emplace_back([](long i) { return i > 0; });
        return *this;
    }

    MatcherInt &MatcherInt::negative() {
        this->filters.emplace_back([](long i) { return i < 0; });
        return *this;
    }

    MatcherInt &MatcherInt::nonPositive() {
        this->filters.emplace_back([](long i) { return i <= 0; });
        return *this;
    }

    MatcherInt &MatcherInt::nonNegative() {
        this->filters.emplace_back([](long i) { return i >= 0; });
        return *this;
    }

    MatcherInt &MatcherInt::greater(long value) {
        this->filters.emplace_back([value](long i) { return i > value; });
        return *this;
    }

    MatcherInt &MatcherInt::greaterEquals(long value) {
        this->filters.emplace_back([value](long i) { return i >= value; });
        return *this;
    }

    MatcherInt &MatcherInt::less(long value) {
        this->filters.emplace_back([value](long i) { return i < value; });
        return *this;
    }

    MatcherInt &MatcherInt::lessEquals(long value) {
        this->filters.emplace_back([value](long i) { return i <= value; });
        return *this;
    }

    MatcherInt &MatcherInt::inRange(long low, long high) {
        Expects(low <= high);
        this->filters.emplace_back([low, high](long i) { return i >= low && i <= high; });
        return *this;
    }

    MatcherInt &MatcherInt::equals(long value) {
        this->filters.emplace_back([value](long i) { return i == value; });
        return *this;
    }

    MatcherInt &MatcherInt::anyOf(const std::vector<long> &values) {
        auto filter = [values](long i) {
            return std::find(values.begin(), values.end(), i) != values.end();
        };
        this->filters.emplace_back(filter);
        return *this;
    }

    MatcherInt &MatcherInt::mapTo(const std::function<Any(long)> &mapping_) {
        this->mapping = mapping_;
        return *this;
    }

    MatcherInt::MatcherInt(long i) {
        this->equals(i);
    }

    MatcherInt::MatcherInt(std::initializer_list<long> values) {
        this->anyOf(values);
    }
} // matcher