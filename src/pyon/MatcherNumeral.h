//
// Created by pkua on 10.12.22.
//

#ifndef RAMPACK_MATCHERNUMERAL_H
#define RAMPACK_MATCHERNUMERAL_H

#include <vector>
#include <functional>

#include "MatcherBase.h"
#include "NodeLiteral.h"
#include "utils/Assertions.h"


namespace pyon::matcher {
    namespace detail {
        template<typename NumeralT, typename NodeT>
        class MatcherNumeral : public MatcherBase {
        private:
            std::vector<std::function<bool(NumeralT)>> filters;
            std::function<Any(NumeralT)> mapping = [](NumeralT i) { return i; };

        public:
            MatcherNumeral() = default;

            explicit MatcherNumeral(NumeralT i) {
                this->equals(i);
            }

            MatcherNumeral(std::initializer_list<NumeralT> values) {
                this->anyOf(values);
            }

            bool match(std::shared_ptr<const ast::Node> node, Any &result) const override {
                if (node->getType() != NodeT::NODE_TYPE)
                    return false;

                NumeralT i = node->as<NodeT>()->getValue();
                for (const auto &filter: filters)
                    if (!filter(i))
                        return false;

                result = this->mapping(i);
                return true;
            }

            template<typename T>
            MatcherNumeral &mapTo() {
                this->mapping = [](NumeralT i) { return static_cast<T>(i); };
                return *this;
            }

            MatcherNumeral &mapTo(const std::function<Any(NumeralT)> &mapping_) {
                this->mapping = mapping_;
                return *this;
            }

            MatcherNumeral &filter(const std::function<bool(NumeralT)> &filter) {
                this->filters.push_back(filter);
                return *this;
            }

            MatcherNumeral &positive() {
                this->filters.emplace_back([](NumeralT i) { return i > 0; });
                return *this;
            }

            MatcherNumeral &negative() {
                this->filters.emplace_back([](NumeralT i) { return i < 0; });
                return *this;
            }

            MatcherNumeral &nonPositive() {
                this->filters.emplace_back([](NumeralT i) { return i <= 0; });
                return *this;
            }

            MatcherNumeral &nonNegative() {
                this->filters.emplace_back([](NumeralT i) { return i >= 0; });
                return *this;
            }

            MatcherNumeral &greater(NumeralT value) {
                this->filters.emplace_back([value](NumeralT i) { return i > value; });
                return *this;
            }

            MatcherNumeral &greaterEquals(NumeralT value) {
                this->filters.emplace_back([value](NumeralT i) { return i >= value; });
                return *this;
            }

            MatcherNumeral &less(NumeralT value) {
                this->filters.emplace_back([value](NumeralT i) { return i < value; });
                return *this;
            }

            MatcherNumeral &lessEquals(NumeralT value) {
                this->filters.emplace_back([value](NumeralT i) { return i <= value; });
                return *this;
            }

            MatcherNumeral &inRange(NumeralT low, NumeralT high) {
                Expects(low <= high);
                this->filters.emplace_back([low, high](NumeralT i) { return i >= low && i <= high; });
                return *this;
            }

            MatcherNumeral &equals(NumeralT value) {
                this->filters.emplace_back([value](NumeralT i) { return i == value; });
                return *this;
            }

            MatcherNumeral &anyOf(const std::vector<NumeralT> &values) {
                auto filter = [values](NumeralT i) {
                    return std::find(values.begin(), values.end(), i) != values.end();
                };
                this->filters.emplace_back(filter);
                return *this;
            }
        };
    }


    class MatcherInt : public detail::MatcherNumeral<long, ast::NodeInt> {
    public:
        using MatcherNumeral::MatcherNumeral;
    };


    class MatcherFloat : public detail::MatcherNumeral<double, ast::NodeFloat> {
    public:
        using MatcherNumeral::MatcherNumeral;
    };
} // matcher

#endif //RAMPACK_MATCHERNUMERAL_H
