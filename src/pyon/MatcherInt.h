//
// Created by pkua on 10.12.22.
//

#ifndef RAMPACK_MATCHERINT_H
#define RAMPACK_MATCHERINT_H

#include <vector>
#include <functional>

#include "MatcherBase.h"
#include "NodeLiteral.h"


namespace pyon::matcher {
    class MatcherInt : public MatcherBase {
    private:
        std::vector<std::function<bool(long)>> filters;
        std::function<Any(long)> mapping = [](long i) { return i; };

    public:
        MatcherInt() = default;
        explicit MatcherInt(long i);
        MatcherInt(std::initializer_list<long> values);

        bool match(std::shared_ptr<const ast::Node> node, Any &result) const override;

        template<typename T>
        MatcherInt &mapTo() {
            this->mapping = [](long i) { return static_cast<T>(i); };
            return *this;
        }

        MatcherInt &mapTo(const std::function<Any(long)> &mapping_);

        MatcherInt &filter(const std::function<bool(long)>& filter);
        MatcherInt &positive();
        MatcherInt &negative();
        MatcherInt &nonPositive();
        MatcherInt &nonNegative();
        MatcherInt &greater(long value);
        MatcherInt &greaterEquals(long value);
        MatcherInt &less(long value);
        MatcherInt &lessEquals(long value);
        MatcherInt &equals(long value);
        MatcherInt &anyOf(const std::vector<long> &values);
        MatcherInt &inRange(long low, long high);
    };
} // matcher

#endif //RAMPACK_MATCHERINT_H
