//
// Created by Piotr Kubala on 21/07/2024.
//

#ifndef RAMPACK_DISCRETESHAPEPARAMETERRANDOMIZER_H
#define RAMPACK_DISCRETESHAPEPARAMETERRANDOMIZER_H

#include <vector>
#include <string>

#include "core/lattice/ShapeParameterRandomizer.h"


/**
 * @brief ShapeParameterRandomizer with selects randomly one of the values from a discrete set passed in the
 * constructor.
 */
class DiscreteShapeParameterRandomizer : public ShapeParameterRandomizer {
private:
    std::vector<std::string> values;

public:
    /**
     * @brief Initializes the set of random values using a vector @a values.
     * @throws PreconditionException if the number of values is less than 2
     */
    explicit DiscreteShapeParameterRandomizer(std::vector<std::string> values);

    /**
     * @brief Initializes the set of random values using an initializer list @a values.
     * @throws PreconditionException if the number of values is less than 2
     */
    DiscreteShapeParameterRandomizer(std::initializer_list<std::string> values);

    [[nodiscard]] std::string randomize(const std::string &oldValue, std::mt19937 &mt) const override;
};


#endif //RAMPACK_DISCRETESHAPEPARAMETERRANDOMIZER_H
