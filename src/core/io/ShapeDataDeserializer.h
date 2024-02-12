//
// Created by Piotr Kubala on 12/02/2024.
//

#ifndef RAMPACK_SHAPEDATADESERIALIZER_H
#define RAMPACK_SHAPEDATADESERIALIZER_H

#include <charconv>
#include <algorithm>

#include "core/ShapeDataManager.h"
#include "geometry/Vector.h"
#include "utils/ParseUtils.h"
#include "utils/Utils.h"


class ShapeDataDeserializer {
private:
    struct GuardedEntry {
        std::string value;
        bool wasAccessed{};
    };


    std::map<std::string, GuardedEntry> textualShapeData;


    template<typename T>
    static T asArithmetic(const std::string &paramKey, const std::string &paramValue) {
        T parsed;
        auto begPtr = paramValue.data();
        auto endPtr = begPtr + paramValue.size();
        auto [parsePtr, errorCode] = std::from_chars(begPtr, endPtr, parsed);

        if (errorCode == std::errc{}) {
            if (endPtr != parsePtr)
                throw ShapeDataSerializationException("Malformed param " + paramKey);
            return parsed;
        }

        switch (errorCode) {
            case std::errc::invalid_argument:
                throw ShapeDataSerializationException("Malformed param " + paramKey);
            case std::errc::result_out_of_range:
                throw ShapeDataSerializationException("Value for param " + paramKey + " out of range");
            default:
                AssertThrow("Unreachable");
        }
    }

    template<typename T>
    static T asIntegral(const std::string &paramKey, const std::string &paramValue) {
        return ShapeDataDeserializer::asArithmetic<T>(paramKey, paramValue);
    }

    template<typename T>
    static T asFloatingPoint(const std::string &paramKey, const std::string &paramValue) {
        T parsed = ShapeDataDeserializer::asArithmetic<T>(paramKey, paramValue);

        if (std::isnan(parsed))
            throw ShapeDataSerializationException("Param " + paramKey + " cannot be NaN");
        if (std::isinf(parsed))
            throw ShapeDataSerializationException("Param " + paramKey + " cannot be infinite");

        return parsed;
    }

    static Vector<3> asVector(const std::string &paramKey, const std::string &paramValue);

public:
    ShapeDataDeserializer() = default;
    explicit ShapeDataDeserializer(const TextualShapeData &data);

    [[nodiscard]] bool hasParam(const std::string &paramKey) const {
        return this->textualShapeData.find(paramKey) != this->textualShapeData.end();
    }

    [[nodiscard]] bool wasAccessed(const std::string &paramKey) const;

    template <typename T>
    T as(const std::string &paramKey) {
        auto it = this->textualShapeData.find(paramKey);
        if (it == this->textualShapeData.end())
            throw ShapeDataSerializationException("Unknown parameter: " + paramKey);

        GuardedEntry &entry = it->second;
        entry.wasAccessed = true;
        const std::string &paramValue = entry.value;

        using DecayT = std::decay_t<T>;

        if constexpr (std::is_integral_v<DecayT>)
            return ShapeDataDeserializer::asIntegral<T>(paramKey, paramValue);
        else if constexpr (std::is_floating_point_v<DecayT>)
            return ShapeDataDeserializer::asFloatingPoint<T>(paramKey, paramValue);
        else if constexpr (std::is_same_v<std::string, DecayT>)
            return entry.value;
        else if constexpr (std::is_same_v<Vector<3>, DecayT>)
            return ShapeDataDeserializer::asVector(paramKey, paramValue);
        else
            static_assert(false, "Only integral, floating point, Vector<3> and std::string types are supported");
    }

    void throwIfNotAccessed() const;
};


#endif //RAMPACK_SHAPEDATADESERIALIZER_H
