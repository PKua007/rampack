//
// Created by Piotr Kubala on 08/02/2024.
//

#ifndef RAMPACK_SHAPEDATAMANAGER_H
#define RAMPACK_SHAPEDATAMANAGER_H

#include "ShapeData.h"
#include "utils/Exceptions.h"


class ShapeDataFormatException : public ValidationException {
    using ValidationException::ValidationException;
};

class ShapeDataManager {
public:
    virtual ~ShapeDataManager() = default;

    [[nodiscard]] virtual std::size_t getShapeDataSize() const { return 0; }
    virtual void validateShapeData([[maybe_unused]] const ShapeData &data) const { }
};


#endif //RAMPACK_SHAPEDATAMANAGER_H
