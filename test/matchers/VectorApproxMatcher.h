//
// Created by Piotr Kubala on 23/12/2020.
//

#ifndef RAMPACK_VECTORAPPROXMATCHER_H
#define RAMPACK_VECTORAPPROXMATCHER_H

#include <catch2/catch.hpp>
#include <sstream>
#include <vector>
#include <iterator>

#include "geometry/Vector.h"

class VectorApproxMatcher : public Catch::MatcherBase<Vector<3>> {
private:
    Vector<3> expected;
    double epsilon;

public:
    VectorApproxMatcher(const Vector<3> &expected, double epsilon)
            : expected(expected), epsilon(epsilon)
    { }

    bool match(const Vector<3> &actual) const override {
        return this->expected[0] == Approx(actual[0]).epsilon(this->epsilon)
               && this->expected[1] == Approx(actual[1]).epsilon(this->epsilon)
               && this->expected[2] == Approx(actual[2]).epsilon(this->epsilon);
    }

    [[nodiscard]] std::string describe() const override {
        std::ostringstream ss;
        ss << "is, within " << this->epsilon << " tolerance threshold, equal to " << this->expected << std::endl;
        return ss.str();
    }
};

inline VectorApproxMatcher IsApproxEqual(const Vector<3> &expected, double epsilon)
{
    return VectorApproxMatcher(expected, epsilon);
}

#endif //RAMPACK_VECTORAPPROXMATCHER_H
