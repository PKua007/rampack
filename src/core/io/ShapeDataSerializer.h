//
// Created by Piotr Kubala on 12/02/2024.
//

#ifndef RAMPACK_SHAPEDATASERIALIZER_H
#define RAMPACK_SHAPEDATASERIALIZER_H

#include <type_traits>

#include "core/ShapeDataManager.h"
#include "geometry/Vector.h"
#include "utils/Exceptions.h"


class ShapeDataSerializer {
public:
    class ParamAccess {
    private:
        std::string &paramValue;

        explicit ParamAccess(std::string &paramValue) : paramValue{paramValue} { }

        friend ShapeDataSerializer;

    public:
        template <typename T, std::enable_if_t<std::is_integral_v<std::decay_t<T>>, int> = 0>
        ParamAccess &operator=(T value) {
            this->paramValue = std::to_string(value);
            return *this;
        }

        template <typename T, std::enable_if_t<std::is_floating_point_v<std::decay_t<T>>, int> = 0>
        ParamAccess &operator=(T value) {
            Expects(!std::isnan(value));
            Expects(!std::isinf(value));

            std::ostringstream ostr;
            ostr << std::setprecision(std::numeric_limits<T>::max_digits10) << value;
            this->paramValue = ostr.str();
            return *this;
        }

        ParamAccess &operator=(const std::string &str);
        ParamAccess &operator=(const Vector<3> &v);
    };

private:
    TextualShapeData data;

public:
    ParamAccess operator[](const std::string &paramKey) { return ParamAccess(this->data[paramKey]); }

    [[nodiscard]] const TextualShapeData &toTextualShapeData() const { return this->data; }
};


#endif //RAMPACK_SHAPEDATASERIALIZER_H
