//
// Created by Piotr Kubala on 13/02/2024.
//

#ifndef RAMPACK_PACKINGAPPROXMATCHER_H
#define RAMPACK_PACKINGAPPROXMATCHER_H


#include <catch2/catch.hpp>
#include <sstream>
#include <vector>
#include <iterator>

#include "MatrixApproxMatcher.h"
#include "VectorApproxMatcher.h"

#include "core/Packing.h"


class PackingApproxMatcher : public Catch::MatcherBase<Packing> {
private:
    const Packing &expected;
    double epsilon;

public:
    PackingApproxMatcher(const Packing &expected, double epsilon)
            : expected(expected), epsilon(epsilon)
    { }

    bool match(const Packing &actual) const override {
        const auto &actualBox = actual.getBox().getDimensions();
        const auto &expectedBox = expected.getBox().getDimensions();
        if (!MatrixApproxMatcher(expectedBox, this->epsilon).match(actualBox))
            return false;

        if (this->expected.size() != actual.size())
            return false;

        for (std::size_t i{}; i < actual.size(); i++) {
            const auto &actualShape = actual[i];
            const auto &expectedShape = expected[i];

            if (!VectorApproxMatcher(expectedShape.getPosition(), this->epsilon).match(actualShape.getPosition()))
                return false;
            if (!MatrixApproxMatcher(expectedShape.getOrientation(), this->epsilon).match(actualShape.getOrientation()))
                return false;
            if (actualShape.getData() != expectedShape.getData())
                return false;
        }

        return true;
    }

    [[nodiscard]] std::string describe() const override {
        std::ostringstream ss;
        ss << "is, within " << this->epsilon << " tolerance threshold, the same as " << std::endl;
        ss << this->expected;
        return ss.str();
    }
};

inline PackingApproxMatcher IsApproxEqual(const Packing &expected, double epsilon) {
    return PackingApproxMatcher(expected, epsilon);
}


#endif //RAMPACK_PACKINGAPPROXMATCHER_H
