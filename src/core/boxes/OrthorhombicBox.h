//
// Created by pkua on 02.02.2022.
//

#ifndef RAMPACK_ORTHORHOMBICBOX_H
#define RAMPACK_ORTHORHOMBICBOX_H

#include <array>

#include "core/Box.h"


class OrthorhombicBox : public Box {
private:
    std::array<double, 3> dimensions{};
    std::array<double, 3> inverseDimensions{};

public:
    explicit OrthorhombicBox(const std::array<double, 3> &dimensions);
    explicit OrthorhombicBox(double linearSize) : OrthorhombicBox({linearSize, linearSize, linearSize}) { }

    void absoluteToRelative(std::vector<Shape> &shapes) const override;
    [[nodiscard]] Vector<3> absoluteToRelative(const Vector<3> &pos) const override {
        Vector<3> result;
        for (std::size_t i{}; i < 3; i++)
            result[i] = this->inverseDimensions[i] * pos[i];
        return result;
    }
    void relativeToAbsolute(std::vector<Shape> &shapes) const override;

    [[nodiscard]] Vector<3> relativeToAbsolute(const Vector<3> &pos) const override {
        Vector<3> result;
        for (std::size_t i{}; i < 3; i++)
            result[i] = this->dimensions[i] * pos[i];
        return result;
    }
};


#endif //RAMPACK_ORTHORHOMBICBOX_H
