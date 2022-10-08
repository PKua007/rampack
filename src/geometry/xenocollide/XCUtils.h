/*
XenoCollide Collision Detection and Physics Library
Copyright (c) 2007-2014 Gary Snethen http://xenocollide.com

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising
from the use of this software.
Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it freely,
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must
not claim that you wrote the original software. If you use this
software in a product, an acknowledgment in the product documentation
would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must
not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/

/*
 * Heavily adapted by Michal Ciesla and Piotr Kubala
 */

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
