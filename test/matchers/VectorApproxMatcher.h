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


template <std::size_t DIM>
class VectorApproxMatcher : public Catch::MatcherBase<Vector<DIM>> {
private:
    Vector<DIM> expected;
    double epsilon;

public:
    VectorApproxMatcher(const Vector<DIM> &expected, double epsilon)
            : expected(expected), epsilon(epsilon)
    { }

    bool match(const Vector<DIM> &actual) const override {
        for (std::size_t i{}; i < DIM; i++)
            if (this->expected[i] != Approx(actual[i]).margin(this->epsilon))
                return false;
        return true;
    }

    [[nodiscard]] std::string describe() const override {
        std::ostringstream ss;
        ss << "is, within " << this->epsilon << " tolerance threshold, equal to " << this->expected << std::endl;
        return ss.str();
    }
};

template <std::size_t DIM>
inline VectorApproxMatcher<DIM> IsApproxEqual(const Vector<DIM> &expected, double epsilon)
{
    return VectorApproxMatcher<DIM>(expected, epsilon);
}

inline VectorApproxMatcher<3> IsApproxEqual(const Vector<3> &expected, double epsilon)
{
    return VectorApproxMatcher<3>(expected, epsilon);
}

#endif //RAMPACK_VECTORAPPROXMATCHER_H
