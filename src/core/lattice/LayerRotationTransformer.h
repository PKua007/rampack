//
// Created by pkua on 19.05.22.
//

#ifndef RAMPACK_LAYERROTATIONTRANSFORMER_H
#define RAMPACK_LAYERROTATIONTRANSFORMER_H

#include <variant>

#include "LayerWiseTransformer.h"
#include "LatticeTraits.h"


/**
 * @brief Perform rotations of particles in the layers (possibly alternating).
 */
class LayerRotationTransformer : public LayerWiseTransformer {
public:
    /**
     * @brief Double-precision floating point alias for radians.
     */
    using Radians = double;

    /**
     * @brief Quotient of a full \f$2\pi\ [\mathrm{rad}]\f$ rotation, reduced at construction
     * [`gcd(numerator, denominator) = 1`].
     */
    struct FullRotationQuotient {
        const unsigned numerator{};
        const unsigned denominator{};

        FullRotationQuotient(unsigned numerator, unsigned denominator);

        [[nodiscard]] double toRadians() const { return 2*M_PI*numerator/denominator; }
    };

    /**
     * @brief Angle of the rotation, expressed either explicitly in radians or as a full rotation quotient.
     */
    using RotationAngle = std::variant<Radians, FullRotationQuotient>;

private:
    std::size_t rotationAxisIdx{};
    RotationAngle rotationAngle = Radians{0};
    bool isAlternating{};

    [[nodiscard]] double getRotationAngle() const;

protected:
    [[nodiscard]] Shape transformShape(const Shape &shape, std::size_t layerIdx) const override;
    [[nodiscard]] std::optional<std::size_t> getRequestedNumOfLayers() const override;

public:
    /**
     * @brief Constructs the object
     * @param layerAxis axis along which to create layers
     * @param rotationAxis axis along which to perform rotations
     * @param rotationAngle angle by which to perform rotations (either explicit in Radians or FullRotationQuotient)
     * @param isAlternating if @a true, molecules in even layers will be rotated counter-clockwise and clockwise in odd
     * layers. If @a false, all molecules will be rotated counter-clockwise
     */
    LayerRotationTransformer(LatticeTraits::Axis layerAxis, LatticeTraits::Axis rotationAxis,
                             const RotationAngle& rotationAngle, bool isAlternating = true);
};


#endif //RAMPACK_LAYERROTATIONTRANSFORMER_H
