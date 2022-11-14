//
// Created by pkua on 22.02.2022.
//

#ifndef RAMPACK_TRICLINICDELTASCALER_H
#define RAMPACK_TRICLINICDELTASCALER_H

#include "core/TriclinicBoxScaler.h"

/**
 * @brief TriclinicBoxScaler translating box vectors by random vectors with uniformly sampled coordinates (enabling
 * triclinic box moves).
 */
class TriclinicDeltaScaler : public TriclinicBoxScaler {
private:
    bool scaleTogether{};

public:
    /**
     * @brief Constructs the scaler.
     * @param stepSize initial step size
     * @param scaleTogether If @a true, all sides will be scaled at once. Otherwise, only a single, randomly chosen
     * side will be perturbed.
     */
    explicit TriclinicDeltaScaler(double stepSize, bool scaleTogether)
            : TriclinicBoxScaler(stepSize), scaleTogether{scaleTogether}
    { }

    [[nodiscard]] TriclinicBox updateBox(const TriclinicBox &oldBox, std::mt19937 &mt) const override;
};


#endif //RAMPACK_TRICLINICDELTASCALER_H
