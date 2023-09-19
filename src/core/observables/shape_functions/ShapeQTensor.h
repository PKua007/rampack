//
// Created by Piotr Kubala on 19/09/2023.
//

#ifndef RAMPACK_SHAPEQTENSOR_H
#define RAMPACK_SHAPEQTENSOR_H

#include "core/observables/ShapeFunction.h"


/**
 * @brief Function returning 6 independent components of the Q-tensor calculated for a single shape.
 * @details <p> Q-tensor is defined as
 *
 * \f[
 * \mathbf{Q} = \frac{3}{2}\mathbf{\hat{a}}\otimes\mathbf{\hat{a}} - \frac{1}{2}\mathbf{I},
 * \f]
 *
 * where \f$\mathbf{\hat{a}}\f$ is shape's primary, secondary or auxiliary axis (see ShapeGeometry).
 *
 * <p> The tensor is symmetric, so only the upper-triangular part has independent values.
 */
class ShapeQTensor : public ShapeFunction {
private:
    std::vector<double> values;
    ShapeGeometry::Axis axis{};

public:
    explicit ShapeQTensor(ShapeGeometry::Axis axis) : values(6), axis{axis} { }

    void calculate(const Shape &shape, const ShapeTraits &traits) override;

    /**
     * @brief Returns `Q_[axis]` as a primary name, where `[axis]` is `pa`, `sa` or `aa`, corresponding to,
     * respectively, primary, secondary or auxiliary axis, as chosen in the constructor.
     */
    [[nodiscard]] std::string getPrimaryName() const override;

    /**
     * @brief Returns names of independent Q-tensor components: `xx`, `xy`, `xz`, `yy`, `yz` and `zz`.
     */
    [[nodiscard]] std::vector<std::string> getNames() const override { return {"xx", "xy", "xz", "yy", "yz", "zz"}; }

    /**
     * @brief Returns values of independent Q-tensor upper-triangular components: \f$\mathbf{Q}_{11}\f$,
     * \f$\mathbf{Q}_{12}\f$, \f$\mathbf{Q}_{13}\f$, \f$\mathbf{Q}_{22}\f$, \f$\mathbf{Q}_{23}\f$,
     * \f$\mathbf{Q}_{33}\f$, in the given order.
     */
    [[nodiscard]] std::vector<double> getValues() const override { return this->values; };
};


#endif //RAMPACK_SHAPEQTENSOR_H
