//
// Created by Codex on 26.02.2026.
//

#ifndef RAMPACK_INTERACTIONCENTRESAPPROXMATCHER_H
#define RAMPACK_INTERACTIONCENTRESAPPROXMATCHER_H

#include <catch2/catch.hpp>
#include <sstream>
#include <vector>

#include "matchers/VectorApproxMatcher.h"

class InteractionCentresApproxMatcher : public Catch::MatcherBase<std::vector<Vector<3>>> {
private:
    std::vector<Vector<3>> expected;
    double epsilon;

public:
    InteractionCentresApproxMatcher(std::vector<Vector<3>> expected, const double epsilon)
            : expected{std::move(expected)}, epsilon{epsilon}
    { }

    [[nodiscard]] bool match(const std::vector<Vector<3>> &actual) const override {
        if (actual.size() != this->expected.size())
            return false;

        for (std::size_t i = 0; i < actual.size(); i++) {
            if (!IsApproxEqual(this->expected[i], this->epsilon).match(actual[i]))
                return false;
        }

        return true;
    }

    [[nodiscard]] std::string describe() const override {
        std::ostringstream ss;
        ss << "has, within " << this->epsilon
           << " tolerance threshold, interaction centres (element-wise) equal to ";
        ss << "{";
        for (std::size_t i = 0; i < this->expected.size(); i++) {
            ss << this->expected[i];
            if (i + 1 < this->expected.size())
                ss << ", ";
        }
        ss << "}";
        return ss.str();
    }
};

inline InteractionCentresApproxMatcher AreApproxEqual(std::vector<Vector<3>> expected, const double epsilon)
{
    return InteractionCentresApproxMatcher(std::move(expected), epsilon);
}

#endif //RAMPACK_INTERACTIONCENTRESAPPROXMATCHER_H
