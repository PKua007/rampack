//
// Created by Piotr Kubala on 03/03/2024.
//

#ifndef RAMPACK_SHAPEPARAMETERRANDOMIZER_H
#define RAMPACK_SHAPEPARAMETERRANDOMIZER_H

#include <string>
#include <random>


/**
 * @brief A concrete way of randomizing shape parameter, used by ShapeParameterRandomizingTransformer, working on
 * serialized @ref TextualShapeData fields.
 */
class ShapeParameterRandomizer {
public:
    virtual ~ShapeParameterRandomizer() = default;

    /**
     * @brief Performs randomization of a @ref TextualShapeData field in an implementation-specific way.
     * @details The caller using the resulting value must ensure that the concrete ShapeParameterRandomizer
     * implementation produces data eligible for deserialization of the concrete ShapeData type which is being
     * randomized.
     * @param oldValue implementations may use the old value of shape parameter to sample a new value
     * @param mt Mersenne Twister engine being the source of randomness
     * @return the new, randomized value of the shape parameter
     */
    [[nodiscard]] virtual std::string randomize(const std::string &oldValue, std::mt19937 &mt) const = 0;
};


#endif //RAMPACK_SHAPEPARAMETERRANDOMIZER_H
