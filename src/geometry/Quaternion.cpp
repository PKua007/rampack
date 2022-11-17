//
// Created by pkua on 17.11.22.
//

#include "Quaternion.h"


Vector<4> Quaternion::fromMatrix(const Matrix<3, 3> &mat) {
    double trace = mat.tr();

    Vector<4> quat;
    if (trace > 0) {
        quat[0] = mat(2, 1) - mat(1, 2);
        quat[1] = mat(0, 2) - mat(2, 0);
        quat[2] = mat(1, 0) - mat(0, 1);
        quat[3] = 1 + trace;
    } else {
        double mx = mat(0, 0);
        std::size_t ix = 0;
        if (mat(1, 1) > mx) {
            mx = mat(1, 1);
            ix = 1;
        }
        if (mat(2,2) > mx) {
            mx = mat(2, 2);
            ix = 2;
        }

        switch (ix) {
            case 0:
                quat[0] = 1 + mat(0, 0) - mat(1, 1) - mat(2, 2);
                quat[1] = mat(1, 0) + mat(0, 1);
                quat[2] = mat(2, 0) + mat(0, 2);
                quat[3] = mat(2, 1) - mat(1, 2);
                break;

            case 1:
                quat[0] = mat(0, 1) + mat(1, 0);
                quat[1] = 1 - mat(0, 0) + mat(1, 1) - mat(2, 2);
                quat[2] = mat(2, 1) + mat(1, 2);
                quat[3] = mat(0, 2) - mat(2, 0);
                break;

            case 2:
                quat[0] = mat(0, 2) + mat(2, 0);
                quat[1] = mat(1, 2) + mat(2, 1);
                quat[2] = 1 - mat(0, 0) - mat(1, 1) + mat(2, 2);
                quat[3] = mat(1, 0) - mat(0, 1);
                break;

            default:
                break;
        }
    }

    return quat.normalized();
}
