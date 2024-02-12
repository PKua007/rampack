//
// Created by Piotr Kubala on 08/02/2024.
//

#ifndef RAMPACK_SHAPEDATAMANAGER_H
#define RAMPACK_SHAPEDATAMANAGER_H

#include <map>

#include "ShapeData.h"
#include "utils/Exceptions.h"


class ShapeDataFormatException : public ValidationException {
    using ValidationException::ValidationException;
};

class ShapeDataSerializationException : public ValidationException {
    using ValidationException::ValidationException;
};

using TextualShapeData = std::map<std::string, std::string>;

class ShapeDataManager {
public:
    virtual ~ShapeDataManager() = default;

    [[nodiscard]] virtual std::size_t getShapeDataSize() const { return 0; }
    virtual void validateShapeData([[maybe_unused]] const ShapeData &data) const { }

    [[nodiscard]] virtual TextualShapeData serialize([[maybe_unused]] const ShapeData &data) const {
        return {};
    }

    [[nodiscard]] virtual ShapeData deserialize([[maybe_unused]] const TextualShapeData &data) const {
        return {};
    }
};


#endif //RAMPACK_SHAPEDATAMANAGER_H
