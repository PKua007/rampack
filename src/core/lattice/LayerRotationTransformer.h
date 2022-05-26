//
// Created by pkua on 19.05.22.
//

#ifndef RAMPACK_LAYERROTATIONTRANSFORMER_H
#define RAMPACK_LAYERROTATIONTRANSFORMER_H

#include "LayerWiseTransformer.h"
#include "LatticeTraits.h"


/**
 * @brief Perform rotations of particles in the layers (possibly alternating).
 */
class LayerRotationTransformer : public LayerWiseTransformer {
private:
    std::size_t rotationAxisIdx{};
    double rotationAngle{};
    bool isAlternating{};

protected:
    [[nodiscard]] Shape transformShape(const Shape &shape, std::size_t layerIdx) const override;
    [[nodiscard]] std::size_t getRequestedNumOfLayers() const override;

public:
    /**
     * @brief Constructs the object
     * @param layerAxis axis along which to create layers
     * @param rotationAxis axis along which to perform rotations
     * @param rotationAngle angle by which to perform rotations
     * @param isAlternating if @a true, molecules in even layers will be rotated clockwise and anti-clockwise in odd
     * layers. If @a false, all molecules will be rotated clockwise
     */
    LayerRotationTransformer(LatticeTraits::Axis layerAxis, LatticeTraits::Axis rotationAxis, double rotationAngle,
                             bool isAlternating = true);
};


#endif //RAMPACK_LAYERROTATIONTRANSFORMER_H
