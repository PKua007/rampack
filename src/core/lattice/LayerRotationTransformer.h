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
        /** @brief Signed numerator of the quotient. */
        const int numerator{};
        /** @brief Nonzero, unsigned denominator of the quotient. */
        const unsigned denominator{};

        FullRotationQuotient(int numerator, unsigned denominator);

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
    bool isCumulative{};

    [[nodiscard]] double getRotationAngle() const;

protected:
    [[nodiscard]] Shape transformShape(const Shape &shape, std::size_t layerIdx) const override;
    [[nodiscard]] std::optional<std::size_t> getRequestedNumOfLayers() const override;

public:
    /**
     * @brief Constructs the object.
     * @param layerAxis axis along which to create layers
     * @param rotationAxis axis along which to perform rotations
     * @param rotationAngle angle by which to perform rotations (either explicit in Radians or FullRotationQuotient)
     * @param isAlternating if @a true, molecules in even layers will be rotated counter-clockwise and clockwise in odd
     * layers. If @a false, all molecules will be rotated counter-clockwise. *Note*: for @a isAlternating equal `true`,
     * the following is implied:
     * <ul>
     *   <li> if @a isCumulative equals `true`, even layers are not rotated, while odd layers are rotated clockwise from
     *        the original position
     *   <li> if @a isCumulative equals `false`, even layers are rotated counter-clockwise, while odd layers are rotated
     *        clockwise from the original position
     * </ul>
     * @param isCumulative if @a true, angle of rotation starts at zero at the lowest layer and is accumulated when
     * moving up the layers, according to @a rotationAngle and @a isAlternating. If @a false, each layer is rotated
     * independently. *Note*: for @a isCumulative equal `true` and @a isAlternating equal `false`
     * <ul>
     *   <li> if @a rotationAngle is Radians, the whole column of cells along the @a layerAxis will be merged into a
     *        single unit cell (LayerRotationTransformer::getRequestedNumOfLayers() returns `std::nullopt`)
     *   <li> if @a rotationAngle is FullRotationQuotient, the minimal number of cells along the @a layerAxis ensuring
     *        periodicity will be merged into a unit cell (LayerRotationTransformer::getRequestedNumOfLayers() returns
     *        FullRotationQuotient::denominator)
     * </ul>
     */
    LayerRotationTransformer(LatticeTraits::Axis layerAxis, LatticeTraits::Axis rotationAxis,
                             const RotationAngle& rotationAngle, bool isAlternating = true,
                             bool isCumulative = false);
};


#endif //RAMPACK_LAYERROTATIONTRANSFORMER_H
