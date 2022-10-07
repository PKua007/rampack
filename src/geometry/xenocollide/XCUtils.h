//
// Created by pkua on 07.10.22.
//

#ifndef RAMPACK_XCUTILS_H
#define RAMPACK_XCUTILS_H

#include <memory>
#include <algorithm>

#include "geometry/Vector.h"


inline bool is_vector_zero(const Vector<3> &v) {
    constexpr double ZERO = 9.094947e-13;
    return std::all_of(v.begin(), v.end(), [](double d) { return std::abs(d) < ZERO; });
}

inline Vector<3> vector_comp_mul(const Vector<3> &a, const Vector<3> &b) {
    return {a[0]*b[0], a[1]*b[1], a[2]*b[2]};
}


#endif //RAMPACK_XCUTILS_H
