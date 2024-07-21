//
// Created by Piotr Kubala on 22/07/2024.
//

#ifndef RAMPACK_TRIANGULARSHAPEPARAMETERRANDOMIZER_H
#define RAMPACK_TRIANGULARSHAPEPARAMETERRANDOMIZER_H

#include <vector>

#include "core/lattice/ShapeParameterRandomizer.h"


/**
 * @brief ShapeParameterRandomizer sampling values from a triangular distribution.
 */
class TriangularShapeParameterRandomizer : public ShapeParameterRandomizer {
private:
    std::vector<double> intervals;
    std::vector<double> weights;

public:
    /**
     * @brief Creates a symmetric distribution, with the mode (maximal value of PDF) at `(beg + end)2`
     * @param beg lower limit of PDF
     * @param end upper limit of PDF
     */
    TriangularShapeParameterRandomizer(double beg, double end)
            : TriangularShapeParameterRandomizer(beg, (beg + end)/2, end)
    { }

    /**
     * @brief Creates a possibly asymmetric distribution, with the mode (maximal value of PDF) at @a mid
     * @param beg lower limit of PDF
     * @param mid mode of PDF
     * @param end upper limit of PDF
     */
    TriangularShapeParameterRandomizer(double beg, double mid, double end);

    std::string randomize(const std::string &oldValue, std::mt19937 &mt) const override;
};


#endif //RAMPACK_TRIANGULARSHAPEPARAMETERRANDOMIZER_H
