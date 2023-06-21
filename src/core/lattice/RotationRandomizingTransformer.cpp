//
// Created by Piotr Kubala on 20/06/2023.
//

#include "RotationRandomizingTransformer.h"
#include "utils/Utils.h"


RotationRandomizingTransformer::RotationRandomizingTransformer(const Axis &axis, unsigned long seed)
        : mt(seed), axis{axis}
{
    if (std::holds_alternative<Vector<3>>(this->axis)) {
        auto &vectorAxis = std::get<Vector<3>>(this->axis);

        // Absolute values of components at least approx 1e-10
        Expects(vectorAxis.norm2() > 1e-20);
        vectorAxis = vectorAxis.normalized();
    }
}

void RotationRandomizingTransformer::transform(Lattice &lattice, const ShapeTraits &shapeTraits) const {
    auto dim = lattice.getDimensions();
    for (std::size_t i{}; i < dim[0]; i++)
        for (std::size_t j{}; j < dim[1]; j++)
            for (std::size_t k{}; k < dim[2]; k++)
                for (auto &shape : lattice.modifySpecificCellMolecules(i, j, k))
                    this->rotateRandomly(shape, shapeTraits.getGeometry());
}

void RotationRandomizingTransformer::rotateRandomly(Shape &shape, const ShapeGeometry &geometry) const {
    auto getRotation = [this, &geometry, &shape](auto &&axisVariant) -> Matrix<3, 3> {
        std::uniform_real_distribution<double> plusMinusOne(-1, 1);
        std::uniform_real_distribution<double> zero2pi(0, 2*M_PI);

        using T = std::decay_t<decltype(axisVariant)>;
        if constexpr (std::is_same_v<T, Vector<3>>) {
            return Matrix<3, 3>::rotation(axisVariant, zero2pi(this->mt));
        } else if constexpr (std::is_same_v<T, ShapeGeometry::Axis>) {
            auto theAxis = geometry.getAxis(shape, axisVariant).normalized();
            return Matrix<3, 3>::rotation(theAxis, zero2pi(this->mt));
        } else if constexpr (std::is_same_v<T, RandomAxisType>) {
            // For Tait-Bryan angles, this gives a uniform probability of rotations
            double angleX = zero2pi(this->mt);
            double angleY = std::asin(plusMinusOne(this->mt));
            double angleZ = zero2pi(this->mt);
            return Matrix<3, 3>::rotation(angleX, angleY, angleZ);
        } else {
            static_assert(always_false<T>);
        }
    };
    auto rotation = std::visit(getRotation, this->axis);

    shape.rotate(rotation);
}
