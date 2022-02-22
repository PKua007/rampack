//
// Created by pkua on 22.02.2022.
//

#ifndef RAMPACK_TRICLINICBOXSCALER_H
#define RAMPACK_TRICLINICBOXSCALER_H

#include <random>

#include "TriclinicBox.h"

/**
 * @brief A class sampling random triclinic box perturbations.
 */
class TriclinicBoxScaler {
public:
    virtual ~TriclinicBoxScaler() = default;

    /**
     * @brief Given the old box @a oldBox, scaling step size @a scalingStepSize and random generator, sample a new box
     * according to implementing class dependent scheme.
     */
    [[nodiscard]] virtual TriclinicBox updateBox(const TriclinicBox &oldBox, double scalingStepSize,
                                                 std::mt19937 &mt) const = 0;
};


#endif //RAMPACK_TRICLINICBOXSCALER_H
