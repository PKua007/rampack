//
// Created by pkup on 02.06.2021.
//

#ifndef RAMPACK_MATRIXAPPROXMATCHER_H
#define RAMPACK_MATRIXAPPROXMATCHER_H

#include <catch2/catch.hpp>
#include <sstream>
#include <vector>
#include <iterator>

#include "geometry/Vector.h"

class MatrixApproxMatcher : public Catch::MatcherBase<Matrix<3, 3>> {
private:
    Matrix<3, 3> expected;
    double epsilon;

public:
    MatrixApproxMatcher(const Matrix<3, 3> &expected, double epsilon)
            : expected{expected}, epsilon{epsilon}
    { }

    [[nodiscard]] bool match(const Matrix<3, 3> &actual) const override {
        for (std::size_t i{}; i < 3; i++)
            for (std::size_t j{}; j < 3; j++)
                if (this->expected(i, j) != Approx(actual(i, j)).margin(this->epsilon))
                    return false;
        return true;
    }

    [[nodiscard]] std::string describe() const override {
        std::ostringstream ss;
        ss << "is, within " << this->epsilon << " tolerance threshold, equal to " << this->expected << std::endl;
        return ss.str();
    }
};

inline MatrixApproxMatcher IsApproxEqual(const Matrix<3, 3> &expected, double epsilon)
{
    return MatrixApproxMatcher(expected, epsilon);
}

#endif //RAMPACK_MATRIXAPPROXMATCHER_H
