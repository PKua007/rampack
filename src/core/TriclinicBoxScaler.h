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
    /** @brief A current step size. */
    double stepSize{};

public:
    /** @brief A factor by which the step size is increased/decreased. */
    static constexpr double STEP_MODIFY_FACTOR = 1.1;

    /**
     * @brief Creates the scaler with a given initial step size @a stepSize.
     */
    explicit TriclinicBoxScaler(double stepSize);

    virtual ~TriclinicBoxScaler() = default;

    /**
     * @brief Given the old box @a oldBox, scaling step size @a scalingStepSize and random generator, sample a new box
     * according to implementing class dependent scheme.
     */
    [[nodiscard]] virtual TriclinicBox updateBox(const TriclinicBox &oldBox, std::mt19937 &mt) const = 0;

    /**
     * @brief If possible, increases the current step size by a factor TriclinicBoxScaler::STEP_MODIFY_FACTOR.
     * @details Default implementation always increases the step size and returns @a true, however derived classes may
     * override it.
     * @return @a true if the factor was increased, @a false otherwise.
     */
    virtual bool increaseStepSize();

    /**
     * @brief If possible, decreases the current step size by a factor TriclinicBoxScaler::STEP_MODIFY_FACTOR.
     * @details Default implementation always decrease the step size and returns @a true, however derived classes may
     * override it.
     * @return @a true if the factor was decreased, @a false otherwise.
     */
    virtual bool decreaseStepSize();

    [[nodiscard]] double getStepSize() const { return this->stepSize; }
    virtual void setStepSize(double stepSize_);
};


#endif //RAMPACK_TRICLINICBOXSCALER_H
