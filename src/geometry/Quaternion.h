//
// Created by pkua on 17.11.22.
//

#ifndef RAMPACK_QUATERNION_H
#define RAMPACK_QUATERNION_H

#include "Matrix.h"
#include "Vector.h"


/**
 * @brief Helper class converting rotation matrix to a quaternion.
 */
class Quaternion {
public:
    static Vector<4> fromMatrix(const Matrix<3, 3> &mat);
};


#endif //RAMPACK_QUATERNION_H
