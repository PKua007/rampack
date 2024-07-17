//
// Created by Piotr Kubala on 12/02/2024.
//

#ifndef RAMPACK_SHAPEDATASERIALIZER_H
#define RAMPACK_SHAPEDATASERIALIZER_H

#include <type_traits>

#include "core/ShapeDataManager.h"
#include "geometry/Vector.h"
#include "utils/Exceptions.h"


/**
 * @brief Helper class for ShapeData serializaion.
 * @details It supports serialization of integer types, floating point types, `std::string`, and Vector<3>. For the
 * serialization format refer to particular overloads of ParamAccess::operator=.
 *
 * Serialization is done by assigning the values to the proxies returned by ShapeDataSerializer::operator[]. When all
 * key=value pairs are stored, one can fetch the resulting @ref TextualShapeData using toTextualShapeData(). Example
 * usage:
 * @code
 * ShapeDataSerializer serializer;
 * serializer["int"] = 42;
 * serializer["double"] = 3.14;
 * serializer["string"] = "Hallo Wereld!";
 * serializer["vector"] = Vector<3>{0.5, 1.0, 1.5};
 * TextualShapeData serializedData = serializer.toTextualShapeData();
 *
 * TextualShapeData expected{{"int", "42"}, {"double", "3.14"}, {"string", "Hallo Wereld!"}, {"vector", "0.5,1.0,1.5"}};
 * assert(serializedData == expected);
 * @endcode
 */
class ShapeDataSerializer {
public:
    /**
     * @brief Helper proxy for serialization.
     */
    class ParamAccess {
    private:
        std::string &paramValue;

        explicit ParamAccess(std::string &paramValue) : paramValue{paramValue} { }

        friend ShapeDataSerializer;

    public:
        /**
         * @brief Serializes integral value using `operator<<` for `std::ostream` with a default formatting.
         */
        template <typename T, std::enable_if_t<std::is_integral_v<std::decay_t<T>>, int> = 0>
        ParamAccess &operator=(T value) {
            this->paramValue = std::to_string(value);
            return *this;
        }

        /**
         * @brief Serializes floating point value value using `operator<<` for `std::ostream` with a default formatting
         * and a full numeric precision. The format may be scientific or decimal, depending on which gives a more
         * compact representation.
         * @throws PreconditionException if the number is infinite or NaN
         */
        template <typename T, std::enable_if_t<std::is_floating_point_v<std::decay_t<T>>, int> = 0>
        ParamAccess &operator=(T value) {
            Expects(!std::isnan(value));
            Expects(!std::isinf(value));

            std::ostringstream ostr;
            ostr << std::setprecision(std::numeric_limits<T>::max_digits10) << value;
            this->paramValue = ostr.str();
            return *this;
        }

        /**
         * @brief Serializes `std::string` as-is.
         */
        ParamAccess &operator=(const std::string &str);

        /**
         * @brief Serializes Vector<3> as a comma-separated list of floating-point values in a format as the floating
         * point overload of operator=, without any whitespace.
         */
        ParamAccess &operator=(const Vector<3> &v);
    };

private:
    TextualShapeData data;

public:
    /**
     * @brief Returns a ParamAccess proxy for storing a value under the key @a paramKey.
     */
    ParamAccess operator[](const std::string &paramKey) { return ParamAccess(this->data[paramKey]); }

    /**
     * @brief Returns @ref TextualShapeData prepared by the class.
     */
    [[nodiscard]] const TextualShapeData &toTextualShapeData() const { return this->data; }
};


#endif //RAMPACK_SHAPEDATASERIALIZER_H
