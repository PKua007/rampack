//
// Created by Piotr Kubala on 20/06/2023.
//

#include "AxisRotationRandomizingTransformer.h"


AxisRotationRandomizingTransformer::AxisRotationRandomizingTransformer(const Axis &axis, unsigned long seed)
        : mt(seed), axis{axis}
{
    if (std::holds_alternative<Vector<3>>(this->axis)) {
        auto &vectorAxis = std::get<Vector<3>>(this->axis);

        // Absolute values of components at least approx 1e-10
        Expects(vectorAxis.norm2() > 1e-20);
        vectorAxis = vectorAxis.normalized();
    }
}

void AxisRotationRandomizingTransformer::transform(Lattice &lattice, const ShapeTraits &shapeTraits) const {
    auto dim = lattice.getDimensions();
    for (std::size_t i{}; i < dim[0]; i++)
        for (std::size_t j{}; j < dim[1]; j++)
            for (std::size_t k{}; k < dim[2]; k++)
                for (auto &shape : lattice.modifySpecificCellMolecules(i, j, k))
                    this->rotateRandomly(shape, shapeTraits.getGeometry());
}

void AxisRotationRandomizingTransformer::rotateRandomly(Shape &shape, const ShapeGeometry &geometry) const {
    auto getAxis = [&geometry, &shape](auto &&axisVariant) -> Vector<3> {
        using T = std::decay_t<decltype(axisVariant)>;
        if constexpr (std::is_same_v<T, Vector<3>>)
            return axisVariant;
        else if constexpr (std::is_same_v<T, ShapeGeometry::Axis>)
            return geometry.getAxis(shape, axisVariant);
    };
    auto theAxis = std::visit(getAxis, this->axis);

    std::uniform_real_distribution<double> zero2pi(0, 2*M_PI);
    shape.rotate(Matrix<3, 3>::rotation(theAxis, zero2pi(this->mt)));
}
