//
// Created by Piotr Kubala on 03/03/2024.
//

#ifndef RAMPACK_GAUSSIANSHAPEPARAMETERRANDOMIZER_H
#define RAMPACK_GAUSSIANSHAPEPARAMETERRANDOMIZER_H

#include <limits>

#include "ShapeParameterRandomizer.h"


class GaussianShapeParameterRandomizer : public ShapeParameterRandomizer {
private:
    double mean{};
    double sigma{};
    double cutoff{};

public:
    GaussianShapeParameterRandomizer(double mean, double sigma,
                                     double cutoff = std::numeric_limits<double>::infinity());

    [[nodiscard]] std::string randomize(const std::string &oldValue, std::mt19937 &mt) const override;
};


#endif //RAMPACK_GAUSSIANSHAPEPARAMETERRANDOMIZER_H
