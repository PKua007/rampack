//
// Created by pkua on 02.02.2022.
//

#ifndef RAMPACK_TRICLINICBOX_H
#define RAMPACK_TRICLINICBOX_H

#include <array>

#include "core/Box.h"
#include "geometry/Matrix.h"


class TriclinicBox : public Box {
private:
    Matrix<3, 3> dimensions;
    Matrix<3, 3> inverseDimensions;

public:
    TriclinicBox() : TriclinicBox(1) { }

    explicit TriclinicBox(double linearSize)
            : TriclinicBox(std::array<double, 3>{linearSize, linearSize, linearSize})
    { }

    explicit TriclinicBox(const Matrix<3, 3> &dimensions)
            : dimensions{dimensions}, inverseDimensions{dimensions.inverse()}
    { }

    explicit TriclinicBox(const std::array<double, 3> &dimensions)
            : TriclinicBox(Matrix<3, 3>{dimensions[0], 0, 0,
                                        0, dimensions[1], 0,
                                        0, 0, dimensions[2]})
    { }

    explicit TriclinicBox(const std::array<Vector<3>, 3> &dimensions);

    void absoluteToRelative(std::vector<Shape> &shapes) const override;
    void relativeToAbsolute(std::vector<Shape> &shapes) const override;

    [[nodiscard]] Vector<3> absoluteToRelative(const Vector<3> &pos) const override {
        return this->inverseDimensions * pos;
    }

    [[nodiscard]] Vector<3> relativeToAbsolute(const Vector<3> &pos) const override {
        return this->dimensions * pos;
    }

    const Matrix<3, 3> &getDimensions() const { return dimensions; }

    void transform(const Matrix<3, 3> &transformation);
    void scale(const std::array<double, 3> &factors);
    void scale(double factor) { this->scale(std::array<double, 3>{factor, factor, factor}); }
};


#endif //RAMPACK_TRICLINICBOX_H
