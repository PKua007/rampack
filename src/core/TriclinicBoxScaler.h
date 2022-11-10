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
protected:
    double stepSize{};

public:
    static constexpr double STEP_MODIFY_FACTOR = 1.1;

    explicit TriclinicBoxScaler(double stepSize);
    virtual ~TriclinicBoxScaler() = default;

    /**
     * @brief Given the old box @a oldBox, scaling step size @a scalingStepSize and random generator, sample a new box
     * according to implementing class dependent scheme.
     */
    [[nodiscard]] virtual TriclinicBox updateBox(const TriclinicBox &oldBox, std::mt19937 &mt) const = 0;

    virtual bool increaseStepSize();
    virtual bool decreaseStepSize();
    [[nodiscard]] double getStepSize() const { return this->stepSize; }
    virtual void setStepSize(double stepSize_);
};


#endif //RAMPACK_TRICLINICBOXSCALER_H
