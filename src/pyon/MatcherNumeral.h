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
            struct Filter {
                std::function<bool(NumeralT)> predicate;
                std::string description;
            };

            std::vector<Filter> filters;
            std::function<Any(NumeralT)> mapping = [](NumeralT i) { return i; };
            
            ConcreteNumeral &concrete() { return static_cast<ConcreteNumeral&>(*this); }

            [[nodiscard]] std::string stringify(NumeralT i) const {
                std::ostringstream out;
                out << i;
                return out.str();
            }

            [[nodiscard]] std::string generateUnmatchedReport(const std::string &reason) const {
                std::ostringstream out;
                out << "Matching " << this->getName() << " failed:" << std::endl;
                out << "✖ " << reason << std::endl;
                out << "✓ Expected format: " << this->outline(0);
                return out.str();
            }

        protected:
            virtual bool matchNodeType(const std::shared_ptr<const ast::Node> &node, NumeralT &numeral) const = 0;
            [[nodiscard]] virtual std::string getName() const = 0;

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

            MatchReport match(std::shared_ptr<const ast::Node> node, Any &result) const override {
                NumeralT numeral{};
                if (!this->matchNodeType(node, numeral))
                    return this->generateUnmatchedReport("Got incorrect node type: " + node->getNodeName());

                for (const auto &filter: filters)
                    if (!filter.predicate(numeral))
                        return this->generateUnmatchedReport("Condition not satisfied: " + filter.description);

                result = this->mapping(numeral);
                return true;
            }

            [[nodiscard]] std::string outline(std::size_t indent) const override {
                std::string spaces(indent, ' ');
                std::ostringstream out;
                out << spaces << this->getName();
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
                this->filters.push_back({filter, "<undefined filter>"});
                return this->concrete();
            }

            ConcreteNumeral &describe(const std::string &description) {
                Expects(!this->filters.empty());
                this->filters.back().description = description;
                return this->concrete();
            }

            ConcreteNumeral &positive() {
                this->filter([](NumeralT i) { return i > 0; });
                this->describe("> 0");
                return this->concrete();
            }

            ConcreteNumeral &negative() {
                this->filter([](NumeralT i) { return i < 0; });
                this->describe("< 0");
                return this->concrete();
            }

            ConcreteNumeral &nonPositive() {
                this->filter([](NumeralT i) { return i <= 0; });
                this->describe("<= 0");
                return this->concrete();
            }

            ConcreteNumeral &nonNegative() {
                this->filter([](NumeralT i) { return i >= 0; });
                this->describe(">= 0");
                return this->concrete();
            }

            ConcreteNumeral &greater(NumeralT value) {
                this->filter([value](NumeralT i) { return i > value; });
                this->describe("> " + this->stringify(value));
                return this->concrete();
            }

            ConcreteNumeral &greaterEquals(NumeralT value) {
                this->filter([value](NumeralT i) { return i >= value; });
                this->describe(">= " + this->stringify(value));
                return this->concrete();
            }

            ConcreteNumeral &less(NumeralT value) {
                this->filter([value](NumeralT i) { return i < value; });
                this->describe("< " + this->stringify(value));
                return this->concrete();
            }

            ConcreteNumeral &lessEquals(NumeralT value) {
                this->filter([value](NumeralT i) { return i <= value; });
                this->describe("<= " + this->stringify(value));
                return this->concrete();
            }

            ConcreteNumeral &inRange(NumeralT low, NumeralT high) {
                Expects(low <= high);
                this->filter([low, high](NumeralT i) { return i >= low && i <= high; });
                this->describe("in range [" + this->stringify(low) + ", " + this->stringify(high) + "]");
                return this->concrete();
            }

            ConcreteNumeral &equals(NumeralT value) {
                this->filter([value](NumeralT i) { return i == value; });
                this->describe("= " + this->stringify(value));
                return this->concrete();
            }

            ConcreteNumeral &anyOf(const std::vector<NumeralT> &values) {
                Expects(!values.empty());
                auto filter = [values](NumeralT i) {
                    return std::find(values.begin(), values.end(), i) != values.end();
                };
                this->filter(filter);

                std::ostringstream out;
                out << "one of: ";
                for (std::size_t i{}; i < values.size() - 1; i++)
                    out << values[i] << ", ";
                out << values.back();
                this->describe(out.str());

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

        [[nodiscard]] std::string getName() const override {
            return "Integer";
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

        [[nodiscard]] std::string getName() const override {
            return "Float";
        }

    public:
        using MatcherNumeral::MatcherNumeral;
    };
} // matcher

#endif //RAMPACK_MATCHERNUMERAL_H
