//
// Created by pkua on 02.02.2022.
//

#ifndef RAMPACK_TRICLINICBOX_H
#define RAMPACK_TRICLINICBOX_H

#include "core/Box.h"
#include "geometry/Matrix.h"


class TriclinicBox : public Box {
private:
    Matrix<3, 3> dimensions;
    Matrix<3, 3> inverseDimensions;

public:
    explicit TriclinicBox(const Matrix<3, 3> &dimensions)
            : dimensions{dimensions}, inverseDimensions{dimensions.inverse()}
    { }

    explicit TriclinicBox(const std::array<Vector<3>, 3> &dimensions);

    void absoluteToRelative(std::vector<Shape> &shapes) const override;
    [[nodiscard]] Vector<3> absoluteToRelative(const Vector<3> &pos) const override;
    void relativeToAbsolute(std::vector<Shape> &shapes) const override;
    [[nodiscard]] Vector<3> relativeToAbsolute(const Vector<3> &pos) const override;
};


#endif //RAMPACK_TRICLINICBOX_H
