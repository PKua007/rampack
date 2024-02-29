//
// Created by Piotr Kubala on 28/02/2024.
//

#ifndef RAMPACK_GENERICSHAPEREGISTRY_H
#define RAMPACK_GENERICSHAPEREGISTRY_H

#include <map>
#include <string>

#include "core/ShapeGeometry.h"
#include "core/ShapeDataManager.h"
#include "utils/Exceptions.h"
#include "core/io/ShapeDataSerializer.h"
#include "core/io/ShapeDataDeserializer.h"


template <typename ShapeClass>
class GenericShapeRegistry : public ShapeGeometry, public ShapeDataManager {
private:
    std::map<std::string, std::size_t> shapeNameIdxMap;
    std::vector<ShapeClass> shapes;

protected:
    void registerCustomNamedPoint(const std::string &pointName) {
        if (this->hasNamedPoint(pointName))
            return;

        this->registerDynamicNamedPoint(pointName, [this, pointName](const ShapeData &data) -> Vector<3> {
            std::size_t shapeIdx = data.as<Data>().shapeIdx;
            const auto &namedPoints = this->getShape(shapeIdx).getCustomNamedPoints();

            auto it = namedPoints.find(pointName);
            if (it == namedPoints.end())
                this->throwUnavailableNamedPoint(shapeIdx, pointName);

            return it->second;
        });
    }

    void throwUnavailableNamedPoint(std::size_t shapeIdx, const std::string &pointName) const {
        const std::string &shapeName = this->getShapeName(shapeIdx);
        std::ostringstream msg;
        msg << "Named point " << pointName << " is not available for shape " << shapeName;
        throw NoSuchNamedPointForShapeException(msg.str());
    }

    [[nodiscard]] const ShapeClass &shapeFor(const Shape &shape) const {
        return this->shapeFor(shape.getData());
    }

    [[nodiscard]] const ShapeClass &shapeFor(const ShapeData &data) const {
        std::size_t shapeIdx = data.as<Data>().shapeIdx;
        Expects(shapeIdx < this->shapes.size());
        return this->shapes[shapeIdx];
    }

    [[nodiscard]] const ShapeClass &shapeFor(const std::byte *data) const {
        std::size_t shapeIdx = ShapeData::as<Data>(data).shapeIdx;
        return this->shapes[shapeIdx];
    }

public:
    struct Data {
        std::size_t shapeIdx{};

        friend bool operator==(Data lhs, Data rhs) { return lhs.shapeIdx == rhs.shapeIdx; }
    };

    [[nodiscard]] Vector<3> getPrimaryAxis(const Shape &shape) const final {
        const auto &polysphereShape = this->shapeFor(shape);
        return shape.getOrientation() * polysphereShape.getPrimaryAxis();
    }

    [[nodiscard]] Vector<3> getSecondaryAxis(const Shape &shape) const final {
        const auto &polysphereShape = this->shapeFor(shape);
        return shape.getOrientation() * polysphereShape.getSecondaryAxis();
    }

    [[nodiscard]] Vector<3> getGeometricOrigin(const Shape &shape) const final {
        const auto &polysphereShape = this->shapeFor(shape);
        return shape.getOrientation() * polysphereShape.getGeometricOrigin();
    }

    [[nodiscard]] double getVolume(const Shape &shape) const final {
        return this->shapeFor(shape).getVolume(shape);
    }

    [[nodiscard]] std::size_t getShapeDataSize() const final {
        return sizeof(Data);
    }

    void validateShapeData(const ShapeData &data) const final {
        const auto &polysphereData = data.as<Data>();
        ShapeDataValidateMsg(polysphereData.shapeIdx < this->shapes.size(), "Shape index out of range");
    }

    [[nodiscard]] ShapeData::Comparator getComparator() const final {
        return ShapeData::Comparator::forType<Data>();
    }

    [[nodiscard]] TextualShapeData serialize(const ShapeData &data) const final {
        std::size_t shapeIdx = data.as<Data>().shapeIdx;
        const std::string &shapeName = this->getShapeName(shapeIdx);

        ShapeDataSerializer serializer;
        serializer["type"] = shapeName;
        return serializer.toTextualShapeData();
    }

    [[nodiscard]] ShapeData deserialize(const TextualShapeData &data) const final {
        ShapeDataDeserializer deserializer(data);
        auto shapeName = deserializer.as<std::string>("type");
        deserializer.throwIfNotAccessed();

        if (!this->hasShape(shapeName))
            throw ShapeDataSerializationException("Unknown shape type: " + shapeName);
        return ShapeData(Data{this->getShapeIdx(shapeName)});
    }

    virtual ShapeData addShape(const std::string &shapeName, const ShapeClass &shape) {
        auto containsQuotes = [](const std::string &str) { return str.find('"') != std::string::npos; };
        Expects(!containsWhitespace(shapeName) && !containsQuotes(shapeName));
        Expects(!this->hasShape(shapeName));

        this->shapes.push_back(shape);
        this->shapeNameIdxMap.emplace(shapeName, this->shapes.size() - 1);

        for (const auto &namedPoint : shape.getCustomNamedPoints()) {
            const auto &pointName = namedPoint.first;
            this->registerCustomNamedPoint(pointName);
        }

        return ShapeData(Data{this->shapes.size() - 1});
    }


    [[nodiscard]] const ShapeClass &getDefaultShape() const {
        const auto &defaultShapeData = this->getDefaultShapeData();
        ExpectsMsg(!defaultShapeData.empty(), "Default shape is not defined");
        return this->getShape(this->getDefaultShapeData().at("type"));
    }

    void setDefaultShape(const std::string &shapeName) {
        Expects(this->hasShape(shapeName));
        this->setDefaultShapeData({{"type", shapeName}});
    }

    [[nodiscard]] bool hasShape(const std::string &shapeName) const {
        return this->shapeNameIdxMap.find(shapeName) != this->shapeNameIdxMap.end();
    }

    [[nodiscard]] const ShapeClass &getShape(const std::string &shapeName) const {
        return this->shapes[this->getShapeIdx(shapeName)];
    }

    [[nodiscard]] const ShapeClass &getShape(std::size_t shapeIdx) const {
        Expects(shapeIdx < this->shapes.size());
        return this->shapes[shapeIdx];
    }

    [[nodiscard]] ShapeData shapeDataFor(const std::string &shapeName) const {
        return ShapeData(Data{this->getShapeIdx(shapeName)});
    }

    [[nodiscard]] ShapeData shapeDataForDefault() const {
        const auto &defaultShapeData = this->getDefaultShapeData();
        ExpectsMsg(!defaultShapeData.empty(), "Default shape is not defined");
        std::size_t shapeIdx = this->getShapeIdx(this->getDefaultShapeData().at("type"));
        return ShapeData(Data{shapeIdx});
    }

    [[nodiscard]] std::size_t getShapeIdx(const std::string &shapeName) const {
        auto it = this->shapeNameIdxMap.find(shapeName);
        ExpectsMsg(it != this->shapeNameIdxMap.end(), "Shape " + shapeName + " not found");
        return it->second;
    }

    [[nodiscard]] const std::string &getShapeName(std::size_t shapeIdx) const {
        Expects(shapeIdx < this->shapes.size());
        for (const auto &[name, idx] : this->shapeNameIdxMap)
            if (idx == shapeIdx)
                return name;
        AssertThrow("Unreachable");
    }
};


#endif //RAMPACK_GENERICSHAPEREGISTRY_H
