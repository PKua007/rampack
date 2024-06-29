//
// Created by Piotr Kubala on 08/02/2024.
//

#ifndef RAMPACK_SHAPEDATAMANAGER_H
#define RAMPACK_SHAPEDATAMANAGER_H

#include <map>
#include <utility>

#include "ShapeData.h"
#include "utils/Exceptions.h"


#define ShapeDataValidateMsg(cond, msg) EXCEPTIONS_BLOCK(   \
    if (!(cond))                                            \
        throw ShapeDataFormatException(msg);                \
)


/**
 * @brief Main exception class describing all types of ShapeData exceptions.
 */
class ShapeDataException : public ValidationException {
    using ValidationException::ValidationException;
};

/**
 * @brief Exception thrown by ShapeDataManager::validateShapeData, when the validation fails.
 */
class ShapeDataFormatException : public ShapeDataException {
    using ShapeDataException::ShapeDataException;
};

/**
 * @brief Exception thrown when ShapeDataManager::deserialize fails due to serialization format error.
 */
class ShapeDataSerializationException : public ShapeDataException {
    using ShapeDataException::ShapeDataException;
};


/**
 * @brief Descriptive alias for textual (serialized) shape data, which is just a key=value string map.
 */
using TextualShapeData = std::map<std::string, std::string>;


/**
 * @brief Class responsible for managing ShapeData of the concrete shape type.
 * @details ShapeData can recognize what specific data type is stored in the raw byte array via the
 * ShapeData::Comparator instance. However, the data type is plain and it cannot perform any operations on itself. This
 * is the purpose of the ShapeDataManager. It is responsible for ShapeData validation and serialization and contains the
 * meta, such as size in byte and Comparator instance. The default implementations of the interface methods correctly
 * handle empty ShapeData.
 */
class ShapeDataManager {
private:
    TextualShapeData defaultData;

protected:
    /**
     * @brief Constructs the class, setting the default serialized shape data (or part of it).
     */
    explicit ShapeDataManager(TextualShapeData defaultData) : defaultData{std::move(defaultData)} { }

    /**
     * @brief Sets the default serialized shape data, used by ShapeDataManager::defaultSerialize and
     * ShapeDataManager::defaultDeserialize methods.
     */
    void setDefaultShapeData(TextualShapeData defaultData_) { this->defaultData = std::move(defaultData_); }

public:
    ShapeDataManager() = default;
    virtual ~ShapeDataManager() = default;

    /**
     * @brief Returns the size in bytes of the associated ShapeData type.
     */
    [[nodiscard]] virtual std::size_t getShapeDataSize() const { return 0; }

    /**
     * @brief Validates the correctness of @a data against the associated ShapeData type.
     * @throws ShapeDataFormatException if validation fails.
     */
    virtual void validateShapeData([[maybe_unused]] const ShapeData &data) const { }

    /**
     * @brief Returns the Comparator instance the associated ShapeData type.
     */
    [[nodiscard]] virtual ShapeData::Comparator getComparator() const { return {}; }

    /**
     * @brief Serializes @a data in an implementation-specific format into TextualShapeData key=value map. @a data in
     * usually not validated before the serialization.
     */
    [[nodiscard]] virtual TextualShapeData serialize([[maybe_unused]] const ShapeData &data) const { return {}; }

    /**
     * @brief Deserializes @a data TextualShapeData in an implementation-specific format into ShapeData. Deserialized
     * data is then validated.
     * @throws ShapeDataSerializationException when serialization data is incomplete or malformed
     * @throws ShapeDataFormatException is the validation after deserialization fails
     */
    [[nodiscard]] virtual ShapeData deserialize([[maybe_unused]] const TextualShapeData &data) const { return {}; }

    /**
     * @brief Return the default shape data in the serialized form. The serialized data may be only partial (containing
     * only selected fields).
     */
    [[nodiscard]] const TextualShapeData &getDefaultShapeData() const { return this->defaultData; }

    /**
     * @brief Serializes @a data analogously to ShapeDataManager::serialize method, however all key=value pairs that are
     * identical as in the default shape data (ShapeDataManager::getDefaultShapeData) are removed.
     */
    [[nodiscard]] TextualShapeData defaultSerialize([[maybe_unused]] const ShapeData &data) const;

    /**
     * @brief Deserializes @a data analogously to ShapeDataManager::deserialize method, however all missing key=value
     * pairs that are supplemented from the default shape data (ShapeDataManager::getDefaultShapeData), if available.
     */
    [[nodiscard]] ShapeData defaultDeserialize([[maybe_unused]] const TextualShapeData &data) const;
};


#endif //RAMPACK_SHAPEDATAMANAGER_H
