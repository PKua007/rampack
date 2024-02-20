//
// Created by Piotr Kubala on 20/12/2020.
//

#include "SpherocylinderTraits.h"
#include "utils/Exceptions.h"
#include "geometry/SegmentDistanceCalculator.h"
#include "geometry/xenocollide/XCBodyBuilder.h"
#include "XCObjShapePrinter.h"
#include "core/io/ShapeDataSerializer.h"
#include "core/io/ShapeDataDeserializer.h"


SpherocylinderTraits::SpherocylinderTraits(std::optional<double> defaultLength, std::optional<double> defaultRadius)
        : defaultLength{defaultLength}, defaultRadius{defaultRadius},
          wolframPrinter{std::make_shared<WolframPrinter>()}
{
    if (this->defaultLength)
        Expects(*this->defaultLength >= 0);
    if (this->defaultRadius)
        Expects(*this->defaultRadius > 0);

    ShapeDataSerializer serializer;
    if (this->defaultLength)
        serializer["length"] = *this->defaultLength;
    if (this->defaultRadius)
        serializer["radius"] = *this->defaultRadius;
    this->defaultData = serializer.toTextualShapeData();

    this->registerStaticNamedPoint("cm", {0, 0, 0});
    this->registerDynamicNamedPoint("beg", [](const ShapeData &data) {
        return SpherocylinderTraits::getCapCentre(-1, data);
    });
    this->registerDynamicNamedPoint("end", [](const ShapeData &data) {
        return SpherocylinderTraits::getCapCentre(1, data);
    });
}

Vector<3> SpherocylinderTraits::getCapCentre(short beginOrEnd, const Matrix<3, 3> &rot, double length) {
    return rot.column(2) * (0.5 * beginOrEnd * length);
}

Vector<3> SpherocylinderTraits::getCapCentre(short beginOrEnd, const ShapeData &data) {
    return {0, 0, 0.5 * beginOrEnd * data.as<Data>().length};
}

Vector<3> SpherocylinderTraits::getCapCentre(short beginOrEnd, const Shape &shape) {
    double length = shape.getData().as<Data>().length;
    return shape.getPosition() + shape.getOrientation().column(2) * (0.5 * beginOrEnd * length);
}

bool SpherocylinderTraits::overlapBetween(const Vector<3> &pos1, const Matrix<3, 3> &orientation1,
                                          const std::byte *data1, [[maybe_unused]] std::size_t idx1,
                                          const Vector<3> &pos2, const Matrix<3, 3> &orientation2,
                                          const std::byte *data2, [[maybe_unused]] std::size_t idx2,
                                          const BoundaryConditions &bc) const
{
    const auto &scData1 = ShapeData::as<Data>(data1);
    const auto &scData2 = ShapeData::as<Data>(data2);

    Vector<3> pos2bc = pos2 + bc.getTranslation(pos1, pos2);
    double distance2 = (pos2bc - pos1).norm2();
    if (distance2 < std::pow(scData1.radius + scData2.radius, 2))
        return true;
    else if (distance2 >= std::pow(scData1.radius + scData1.length/2 + scData2.radius + scData2.length/2, 2))
        return false;

    Vector<3> cap11 = SpherocylinderTraits::getCapCentre(-1, orientation1, scData1.length);
    Vector<3> cap12 = -cap11;
    Vector<3> cap21 = SpherocylinderTraits::getCapCentre(-1, orientation2, scData2.length);
    Vector<3> cap22 = -cap21;
    return SegmentDistanceCalculator::calculate(pos1 + cap11, pos1 + cap12, pos2bc + cap21, pos2bc + cap22)
           < std::pow(scData1.radius + scData2.radius, 2);
}

double SpherocylinderTraits::getVolume(const Shape &shape) const {
    const auto &scData = shape.getData().as<Data>();
    double radius = scData.radius;
    double length = scData.length;
    return M_PI*radius*radius*length + 4./3*M_PI*radius*radius*radius;
}

Vector<3> SpherocylinderTraits::getPrimaryAxis(const Shape &shape) const {
    return shape.getOrientation().column(2);
}

bool SpherocylinderTraits::overlapWithWall(const Vector<3> &pos, const Matrix<3, 3> &orientation,
                                           const std::byte *data, [[maybe_unused]] std::size_t idx,
                                           const Vector<3> &wallOrigin, const Vector<3> &wallVector) const
{
    const auto &scData = ShapeData::as<Data>(data);
    Vector<3> halfAxis = orientation.column(2) * (scData.length/2);

    Vector<3> cap1 = pos + halfAxis;
    double dotProduct1 = wallVector * (cap1 - wallOrigin);
    if (dotProduct1 < scData.radius)
        return true;

    Vector<3> cap2 = pos - halfAxis;
    double dotProduct2 = wallVector * (cap2 - wallOrigin);
    if (dotProduct2 < scData.radius)
        return true;

    return false;
}

std::shared_ptr<const ShapePrinter>
SpherocylinderTraits::getPrinter(const std::string &format, const std::map<std::string, std::string> &params) const {
    std::size_t meshSubdivisions = DEFAULT_MESH_SUBDIVISIONS;
    if (params.find("mesh_divisions") != params.end()) {
        meshSubdivisions = std::stoul(params.at("mesh_divisions"));
        Expects(meshSubdivisions >= 1);
    }

    if (format == "wolfram")
        return this->wolframPrinter;
    // TODO: obj printer
    // else if (format == "obj")
    //    return createObjPrinter(this->length, this->radius, meshSubdivisions);
    else
        throw NoSuchShapePrinterException("SphereTraits: unknown printer format: " + format);
}

std::unique_ptr<ShapePrinter> SpherocylinderTraits::createObjPrinter(double length, double radius,
                                                                     std::size_t subdivisions)
{
    XCBodyBuilder builder;
    builder.sphere(radius);
    builder.move(0, 0, -length/2);
    builder.sphere(radius);
    builder.move(0, 0, length/2);
    builder.wrap();

    return std::make_unique<XCObjShapePrinter>(*builder.releaseCollideGeometry(), subdivisions);
}

void SpherocylinderTraits::validateShapeData(const ShapeData &data) const {
    const auto &scData = data.as<Data>();
    ValidateMsg(scData.radius > 0, "spherocylinder's radius must be > 0");
    ValidateMsg(scData.length >= 0, "spherocylinder's length must be >= 0");
}

TextualShapeData SpherocylinderTraits::serialize(const ShapeData &data) const {
    const auto &scData = data.as<Data>();
    ShapeDataSerializer serializer;
    serializer["radius"] = scData.radius;
    serializer["length"] = scData.length;
    return serializer.toTextualShapeData();
}

ShapeData SpherocylinderTraits::deserialize(const TextualShapeData &data) const {
    ShapeDataDeserializer deserializer(data);
    Data scData;
    scData.radius = deserializer.as<double>("radius");
    scData.length = deserializer.as<double>("length");
    deserializer.throwIfNotAccessed();

    ShapeData shapeData(scData);
    this->validateShapeData(shapeData);
    return shapeData;
}

std::string SpherocylinderTraits::WolframPrinter::print(const Shape &shape) const {
    std::stringstream out;
    out << std::fixed;
    Vector<3> beg = SpherocylinderTraits::getCapCentre(-1, shape);
    Vector<3> end = SpherocylinderTraits::getCapCentre(1, shape);
    double radius = shape.getData().as<Data>().radius;
    out << "CapsuleShape[{" << beg << "," << end << "}," << radius << "]";
    return out.str();
}
