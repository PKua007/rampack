//
// Created by Piotr Kubala on 20/07/2024.
//

#ifndef RAMPACK_UNIFORMSHAPEPARAMRANDOMIZER_H
#define RAMPACK_UNIFORMSHAPEPARAMRANDOMIZER_H

#include <type_traits>
#include <random>
#include <sstream>
#include <limits>

#include "core/lattice/ShapeParameterRandomizer.h"
#include "utils/Exceptions.h"


/**
 * @brief ShapeParamRandomizer which samples integer or floating point shape param from a uniformly distributed range.
 * @tparam Arithmetic type of the randomized value; it must be either an integer or a floating point type
 */
template<typename Arithmetic>
class UniformShapeParamRandomizer : public ShapeParameterRandomizer {
    static_assert(std::is_arithmetic_v<Arithmetic>, "Arithmetic should be an integral or floating point type");

private:
    Arithmetic beg{};
    Arithmetic end{};

public:
    /**
     * @brief Creates the randomizer for the uniform range [@a beg, @a end] (the upper range is inclusive for integer
     * types and exclusive for floating point types).
     * @throws PreconditionException if @a beg &ge; @a end.
     */
    UniformShapeParamRandomizer(Arithmetic beg, Arithmetic end) : beg{beg}, end{end} {
        Expects(beg < end);
    }

    std::string randomize([[maybe_unused]] const std::string &oldValue, std::mt19937 &mt) const override {
        if constexpr (std::is_integral_v<Arithmetic>) {
            std::uniform_int_distribution<Arithmetic> intRange(this->beg, this->end);
            Arithmetic newValue = intRange(mt);
            return std::to_string(newValue);
        } else {    // std::is_floating_point<T> == true
            std::uniform_real_distribution<Arithmetic> floatRange(this->beg, this->end);
            Arithmetic newValue = floatRange(mt);
            std::ostringstream out;
            out.precision(std::numeric_limits<Arithmetic>::max_digits10);
            out << newValue;
            return out.str();
        }
    }
};


#endif //RAMPACK_UNIFORMSHAPEPARAMRANDOMIZER_H
