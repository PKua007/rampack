//
// Created by pkua on 22.02.2022.
//

#ifndef RAMPACK_TRICLINICBOXSCALER_H
#define RAMPACK_TRICLINICBOXSCALER_H

#include <random>

#include "TriclinicBox.h"

class TriclinicBoxScaler {
public:
    virtual ~TriclinicBoxScaler() = default;

    [[nodiscard]] virtual TriclinicBox updateBox(const TriclinicBox &oldBox, double scalingStepSize,
                                                 std::mt19937 &mt) const = 0;
};


#endif //RAMPACK_TRICLINICBOXSCALER_H
