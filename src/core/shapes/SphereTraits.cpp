//
// Created by Piotr Kubala on 20/12/2020.
//

#include <cmath>
#include <sstream>

#include "SphereTraits.h"
#include "utils/Exceptions.h"
#include "XCObjShapePrinter.h"
#include "geometry/xenocollide/XCPrimitives.h"
#include "core/io/ShapeDataSerializer.h"
#include "core/io/ShapeDataDeserializer.h"


SphereTraits::HardDataManager::HardDataManager(std::optional<double> defaultRadius) {
    if (defaultRadius)
        Expects(defaultRadius > 0);

    ShapeDataSerializer serializer;
    if (defaultRadius)
        serializer["r"] = *defaultRadius;
    this->setDefaultShapeData(serializer.toTextualShapeData());
}

void SphereTraits::HardDataManager::validateShapeData(const ShapeData &data) const {
    double radius = data.as<HardData>().radius;
    ShapeDataValidateMsg(radius > 0, "sphere's radius must be > 0");
}

TextualShapeData SphereTraits::HardDataManager::serialize(const ShapeData &data) const {
    ShapeDataSerializer serializer;
    const auto &hardData = data.as<HardData>();
    serializer["r"] = hardData.radius;
    return serializer.toTextualShapeData();
}

ShapeData SphereTraits::HardDataManager::deserialize(const TextualShapeData &data) const {
    ShapeDataDeserializer deserializer(data);
    HardData hardData{deserializer.as<double>("r")};
    deserializer.throwIfNotAccessed();

    ShapeData shapeData(hardData);
    this->validateShapeData(shapeData);
    return shapeData;
}

SphereTraits::WolframPrinter::WolframPrinter(double fixedRadius) : fixedRadius{fixedRadius} {
    Expects(fixedRadius > 0);
}

SphereTraits::SphereTraits(std::optional<double> defaultRadius)
        : fixedRadius{std::nullopt},
          interaction{std::make_shared<HardInteraction>()},
          dataManager{std::make_shared<HardDataManager>(defaultRadius)},
          wolframPrinter{std::make_shared<WolframPrinter>()}
{
    this->registerStaticNamedPoint("cm", {0, 0, 0});
}

SphereTraits::SphereTraits(double fixedRadius, std::shared_ptr<CentralInteraction> centralInteraction)
        : fixedRadius{fixedRadius},
          dataManager{std::make_shared<ShapeDataManager>()},
          wolframPrinter{std::make_shared<WolframPrinter>(fixedRadius)}
{
    centralInteraction->installOnSphere();
    this->interaction = std::move(centralInteraction);
    this->registerStaticNamedPoint("cm", {0, 0, 0});
}

double SphereTraits::getVolume(const Shape &shape) const {
    double radius = this->fixedRadius.has_value()
                    ? *this->fixedRadius
                    : shape.getData().as<HardData>().radius;

    return 4./3*M_PI*radius*radius*radius;
}

std::shared_ptr<const ShapePrinter> SphereTraits::getPrinter(const std::string &format,
                                                             const std::map<std::string, std::string> &params) const
{
    std::size_t meshSubdivisions = DEFAULT_MESH_SUBDIVISIONS;
    if (params.find("mesh_divisions") != params.end()) {
        meshSubdivisions = std::stoul(params.at("mesh_divisions"));
        Expects(meshSubdivisions >= 1);
    }

    if (format == "wolfram")
        return this->wolframPrinter;
    // TODO: printer for OBJ
    // else if (format == "obj")
    //    return createObjPrinter(this->radius, meshSubdivisions);
    else
        throw NoSuchShapePrinterException("SphereTraits: unknown printer format: " + format);
}

std::shared_ptr<ShapePrinter> SphereTraits::createObjPrinter(double radius, std::size_t subdivisions) {
    return std::make_unique<XCObjShapePrinter>(XCSphere{radius}, subdivisions);
}

bool SphereTraits::HardInteraction::overlapBetween(const Vector<3> &pos1,
                                                   [[maybe_unused]] const Matrix<3, 3> &orientation1,
                                                   const std::byte *data1,
                                                   [[maybe_unused]] std::size_t idx1,
                                                   const Vector<3> &pos2,
                                                   [[maybe_unused]] const Matrix<3, 3> &orientation2,
                                                   const std::byte *data2,
                                                   [[maybe_unused]] std::size_t idx2,
                                                   const BoundaryConditions &bc) const
{
    double r1 = ShapeData::as<HardData>(data1).radius;
    double r2 = ShapeData::as<HardData>(data2).radius;
    return bc.getDistance2(pos1, pos2) < std::pow(r1 + r2, 2);
}

bool SphereTraits::HardInteraction::overlapWithWall(const Vector<3> &pos,
                                                    [[maybe_unused]] const Matrix<3, 3> &orientation,
                                                    const std::byte *data,
                                                    [[maybe_unused]] std::size_t idx,
                                                    const Vector<3> &wallOrigin,
                                                    const Vector<3> &wallVector) const
{
    double r = ShapeData::as<HardData>(data).radius;
    double dotProduct = wallVector * (pos - wallOrigin);
    return dotProduct < r;
}

double SphereTraits::HardInteraction::getRangeRadius(const std::byte *data) const {
    return 2 * ShapeData::as<HardData>(data).radius;
}

std::string SphereTraits::WolframPrinter::print(const Shape &shape) const {
    double radius = this->fixedRadius.has_value()
                    ? *this->fixedRadius
                    : shape.getData().as<HardData>().radius;

    std::ostringstream out;
    out << "Sphere[" << (shape.getPosition()) << "," << radius << "]";
    return out.str();
}