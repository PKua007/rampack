//
// Created by pkua on 22.02.2022.
//

#ifndef RAMPACK_TRICLINICDELTASCALER_H
#define RAMPACK_TRICLINICDELTASCALER_H

#include "core/TriclinicBoxScaler.h"

class TriclinicDeltaScaler : public TriclinicBoxScaler {
private:
    bool scaleTogether{};

public:
    explicit TriclinicDeltaScaler(bool scaleTogether) : scaleTogether{scaleTogether} { }

    [[nodiscard]] TriclinicBox updateBox(const TriclinicBox &oldBox, double scalingStepSize,
                                         std::mt19937 &mt) const override;
};


#endif //RAMPACK_TRICLINICDELTASCALER_H
