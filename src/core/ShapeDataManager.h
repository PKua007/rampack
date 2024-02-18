//
// Created by Piotr Kubala on 08/02/2024.
//

#ifndef RAMPACK_SHAPEDATAMANAGER_H
#define RAMPACK_SHAPEDATAMANAGER_H

#include <map>
#include <utility>

#include "ShapeData.h"
#include "utils/Exceptions.h"


class ShapeDataException : public ValidationException {
    using ValidationException::ValidationException;
};

class ShapeDataFormatException : public ShapeDataException {
    using ShapeDataException::ShapeDataException;
};

class ShapeDataSerializationException : public ShapeDataException {
    using ShapeDataException::ShapeDataException;
};

using TextualShapeData = std::map<std::string, std::string>;

class ShapeDataManager {
private:
    TextualShapeData defaultData;

protected:
    explicit ShapeDataManager(TextualShapeData defaultData) : defaultData{std::move(defaultData)} { }

    void setDefaultShapeData(TextualShapeData defaultData_) { this->defaultData = std::move(defaultData_); }

public:
    ShapeDataManager() = default;

    virtual ~ShapeDataManager() = default;

    [[nodiscard]] virtual std::size_t getShapeDataSize() const { return 0; }
    virtual void validateShapeData([[maybe_unused]] const ShapeData &data) const { }
    [[nodiscard]] virtual ShapeData::Comparator getComparator() const { return {}; }
    [[nodiscard]] virtual TextualShapeData serialize([[maybe_unused]] const ShapeData &data) const { return {}; }
    [[nodiscard]] virtual ShapeData deserialize([[maybe_unused]] const TextualShapeData &data) const { return {}; }

    [[nodiscard]] const TextualShapeData &getDefaultShapeData() const { return this->defaultData; }
    [[nodiscard]] TextualShapeData defaultSerialize([[maybe_unused]] const ShapeData &data) const;
    [[nodiscard]] ShapeData defaultDeserialize([[maybe_unused]] const TextualShapeData &data) const;

};


#endif //RAMPACK_SHAPEDATAMANAGER_H
