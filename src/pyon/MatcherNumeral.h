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
        template<typename NumeralT, typename ConcreteNumeral>
        class MatcherNumeral : public MatcherBase {
        private:
            std::vector<std::function<bool(NumeralT)>> filters;
            std::function<Any(NumeralT)> mapping = [](NumeralT i) { return i; };
            
            ConcreteNumeral &concrete() { return static_cast<ConcreteNumeral&>(*this); }

        protected:
            virtual bool matchNodeType(const std::shared_ptr<const ast::Node> &node, NumeralT &numeral) const = 0;

        public:
            MatcherNumeral() = default;

            explicit MatcherNumeral(NumeralT i) {
                this->equals(i);
            }

            MatcherNumeral(std::initializer_list<NumeralT> values) {
                this->anyOf(values);
            }

            [[nodiscard]] ConcreteNumeral copy() const {
                return this->concrete();
            }

            bool match(std::shared_ptr<const ast::Node> node, Any &result) const override {
                NumeralT numeral{};
                if (!this->matchNodeType(node, numeral))
                    return false;

                for (const auto &filter: filters)
                    if (!filter(numeral))
                        return false;

                result = this->mapping(numeral);
                return true;
            }

            template<typename T>
            ConcreteNumeral &mapTo() {
                this->mapping = [](NumeralT i) { return static_cast<T>(i); };
                return this->concrete();
            }

            ConcreteNumeral &mapTo(const std::function<Any(NumeralT)> &mapping_) {
                this->mapping = mapping_;
                return this->concrete();
            }

            ConcreteNumeral &filter(const std::function<bool(NumeralT)> &filter) {
                this->filters.push_back(filter);
                return this->concrete();
            }

            ConcreteNumeral &positive() {
                this->filters.emplace_back([](NumeralT i) { return i > 0; });
                return this->concrete();
            }

            ConcreteNumeral &negative() {
                this->filters.emplace_back([](NumeralT i) { return i < 0; });
                return this->concrete();
            }

            ConcreteNumeral &nonPositive() {
                this->filters.emplace_back([](NumeralT i) { return i <= 0; });
                return this->concrete();
            }

            ConcreteNumeral &nonNegative() {
                this->filters.emplace_back([](NumeralT i) { return i >= 0; });
                return this->concrete();
            }

            ConcreteNumeral &greater(NumeralT value) {
                this->filters.emplace_back([value](NumeralT i) { return i > value; });
                return this->concrete();
            }

            ConcreteNumeral &greaterEquals(NumeralT value) {
                this->filters.emplace_back([value](NumeralT i) { return i >= value; });
                return this->concrete();
            }

            ConcreteNumeral &less(NumeralT value) {
                this->filters.emplace_back([value](NumeralT i) { return i < value; });
                return this->concrete();
            }

            ConcreteNumeral &lessEquals(NumeralT value) {
                this->filters.emplace_back([value](NumeralT i) { return i <= value; });
                return this->concrete();
            }

            ConcreteNumeral &inRange(NumeralT low, NumeralT high) {
                Expects(low <= high);
                this->filters.emplace_back([low, high](NumeralT i) { return i >= low && i <= high; });
                return this->concrete();
            }

            ConcreteNumeral &equals(NumeralT value) {
                this->filters.emplace_back([value](NumeralT i) { return i == value; });
                return this->concrete();
            }

            ConcreteNumeral &anyOf(const std::vector<NumeralT> &values) {
                auto filter = [values](NumeralT i) {
                    return std::find(values.begin(), values.end(), i) != values.end();
                };
                this->filters.emplace_back(filter);
                return this->concrete();
            }
        };
    }


    class MatcherInt : public detail::MatcherNumeral<long, MatcherInt> {
    protected:
        bool matchNodeType(const std::shared_ptr<const ast::Node> &node, long &numeral) const override {
            if (node->getType() != ast::Node::INT)
                return false;

            numeral = node->as<ast::NodeInt>()->getValue();
            return true;
        }

    public:
        using MatcherNumeral::MatcherNumeral;
    };


    class MatcherFloat : public detail::MatcherNumeral<double, MatcherFloat> {
    protected:
        bool matchNodeType(const std::shared_ptr<const ast::Node> &node, double &numeral) const override {
            switch (node->getType()) {
                case ast::Node::INT:
                    numeral = static_cast<double>(node->as<ast::NodeInt>()->getValue());
                    return true;
                case ast::Node::FLOAT:
                    numeral = node->as<ast::NodeFloat>()->getValue();
                    return true;
                default:
                    return false;
            }
        }

    public:
        using MatcherNumeral::MatcherNumeral;
    };
} // matcher

#endif //RAMPACK_MATCHERNUMERAL_H
