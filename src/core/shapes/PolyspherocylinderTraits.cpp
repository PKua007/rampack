//
// Created by Piotr Kubala on 26/04/2021.
//

#include <numeric>
#include <algorithm>
#include <iterator>

#include "PolyspherocylinderTraits.h"
#include "utils/Exceptions.h"
#include "geometry/SegmentDistanceCalculator.h"
#include "printers/XCObjShapePrinter.h"
#include "geometry/xenocollide/XCBodyBuilder.h"
#include "printers/PolydisperseXCObjShapePrinter.h"


// PolyspherocylinderShape::SpherocylinderData #########################################################################

PolyspherocylinderShape::SpherocylinderData::SpherocylinderData(const Vector<3> &position, const Vector<3> &halfAxis,
                                                                double radius)
        : position{position}, halfAxis{halfAxis}, halfLength{halfAxis.norm()}, radius{radius},
          circumsphereRadius{radius + halfLength}
{
    Expects(radius > 0);
}

void PolyspherocylinderShape::SpherocylinderData::toWolfram(std::ostream &out, const Shape &shape) const {
    Vector<3> shapeHalfAxis = this->halfAxisForShape(shape);
    Vector<3> beg = shape.getPosition() + shape.getOrientation() * this->position + shapeHalfAxis;
    Vector<3> end = shape.getPosition() + shape.getOrientation() * this->position - shapeHalfAxis;

    out << "Tube[{" << beg << "," << end << "}," << this->radius << "]";
}

Vector<3> PolyspherocylinderShape::SpherocylinderData::centreForShape(const Shape &shape) const {
    return shape.getPosition() + shape.getOrientation() * this->position;
}

double PolyspherocylinderShape::SpherocylinderData::getVolume() const {
    double radius2 = this->radius * this->radius;
    double radius3 = radius2 * this->radius;
    double axisLength = 2 * this->halfLength;
    return 4*M_PI/3*radius3 + M_PI*radius2*axisLength;
}

Vector<3> PolyspherocylinderShape::SpherocylinderData::halfAxisForShape(const Shape &shape) const {
    return shape.getOrientation() * this->halfAxis;
}

std::shared_ptr<AbstractXCGeometry> PolyspherocylinderShape::SpherocylinderData::createXCGeometry() const {
    XCBodyBuilder builder;
    builder.sphere(this->radius);
    Vector<3> pos1 = -this->halfAxis;
    builder.move(pos1[0], pos1[1], pos1[2]);
    builder.sphere(this->radius);
    Vector<3> pos2 = this->halfAxis;
    builder.move(pos2[0], pos2[1], pos2[2]);
    builder.wrap();
    return builder.releaseCollideGeometry();
}


// PolyspherocylinderShape #############################################################################################

double PolyspherocylinderShape::calculateVolume() const {
    ExpectsMsg(!this->spherocylindersOverlap(), "PolyspherocylinderTraits::PolyspherocylinderGeometry::calculateVolume:"
                                                " automatic volume not supported for overlapping spheres");

    auto volumeAccumulator = [](double volume_, const SpherocylinderData &data) {
        return volume_ + data.getVolume();
    };
    return std::accumulate(this->spherocylinderData.begin(), this->spherocylinderData.end(), 0., volumeAccumulator);
}

PolyspherocylinderShape::PolyspherocylinderShape(std::vector<SpherocylinderData> spherocylinderData,
                                                 OptionalAxis primaryAxis, OptionalAxis secondaryAxis,
                                                 const Vector<3> &geometricOrigin, std::optional<double> volume,
                                                 const std::map<std::string, Vector<3>> &customNamedPoints)
        : spherocylinderData{std::move(spherocylinderData)}, primaryAxis{primaryAxis}, secondaryAxis{secondaryAxis},
          geometricOrigin{geometricOrigin}, namedPoints{customNamedPoints}
{
    Expects(!this->spherocylinderData.empty());
    if (!this->primaryAxis.has_value())
        Expects(!this->secondaryAxis.has_value());
    if (this->primaryAxis.has_value())
        this->primaryAxis = this->primaryAxis->normalized();
    if (this->secondaryAxis.has_value())
        this->secondaryAxis = this->secondaryAxis->normalized();

    if (volume.has_value())
        this->volume = *volume;
    else
        this->volume = this->calculateVolume();

    for (std::size_t i{}; i < this->spherocylinderData.size(); i++) {
        const auto &scData = this->spherocylinderData[i];
        std::string iStr = std::to_string(i);
        this->namedPoints["o" + iStr] = scData.position;
        this->namedPoints["b" + iStr] = scData.position - scData.halfAxis;
        this->namedPoints["e" + iStr] = scData.position + scData.halfAxis;
    }
}

Vector<3> PolyspherocylinderShape::getPrimaryAxis() const /* override */ {
    if (!this->primaryAxis.has_value())
        throw std::runtime_error("PolyspherocylinderGeometry::getPrimaryAxis: primary axis not defined");
    return this->primaryAxis.value();
}

Vector<3> PolyspherocylinderShape::getSecondaryAxis() const /* override */ {
    if (!this->primaryAxis.has_value())
        throw std::runtime_error("PolyspherocylinderGeometry::getSecondaryAxis: secondary axis not defined");
    return this->secondaryAxis.value();
}

Vector<3> PolyspherocylinderShape::getGeometricOrigin() const /* override */ {
    return this->geometricOrigin;
}

bool PolyspherocylinderShape::spherocylindersOverlap() const {
    for (std::size_t i{}; i < this->spherocylinderData.size(); i++) {
        for (std::size_t j = i + 1; j < this->spherocylinderData.size(); j++) {
            const auto &data1 = spherocylinderData[i];
            const auto &data2 = spherocylinderData[j];

            Vector<3> sc11 = data1.position - data1.halfAxis;
            Vector<3> sc12 = data1.position + data1.halfAxis;
            Vector<3> sc21 = data2.position - data2.halfAxis;
            Vector<3> sc22 = data2.position + data2.halfAxis;
            double distance = SegmentDistanceCalculator::calculate(sc11, sc12, sc21, sc22);

            constexpr double EPSILON = 1e-12;
            if (distance * (1 + EPSILON) < data1.radius + data2.radius)
                return true;
        }
    }

    return false;
}

void PolyspherocylinderShape::addCustomNamedPoints(std::map<std::string, Vector<3>> customNamedPoints) {
    customNamedPoints.merge(std::move(this->namedPoints));
    this->namedPoints = std::move(customNamedPoints);
}

bool operator==(const PolyspherocylinderShape &lhs, const PolyspherocylinderShape &rhs) {
    return std::tie(
        lhs.spherocylinderData, lhs.primaryAxis, lhs.secondaryAxis, lhs.geometricOrigin, lhs.namedPoints, lhs.volume
    ) == std::tie(
        rhs.spherocylinderData, rhs.primaryAxis, rhs.secondaryAxis, rhs.geometricOrigin, rhs.namedPoints, rhs.volume
    );
}


// PolysphereTraits::WolframPrinter ####################################################################################

std::string PolyspherocylinderTraits::WolframPrinter::print(const Shape &shape) const {
    std::ostringstream out;
    out << std::fixed;
    out << "{";
    const auto &spherocylinderData = this->traits.speciesFor(shape).getSpherocylinderData();
    for (std::size_t i{}; i < spherocylinderData.size() - 1; i++) {
        const auto &data = spherocylinderData[i];
        data.toWolfram(out, shape);
        out << ",";
    }
    spherocylinderData.back().toWolfram(out, shape);
    out << "}";
    return out.str();
}


// PolyspherocylinderTraits ############################################################################################

PolyspherocylinderTraits::PolyspherocylinderTraits(const PolyspherocylinderShape &defaultShape)
    : PolyspherocylinderTraits()
{
    this->addSpecies("A", defaultShape);
    this->setDefaultSpecies("A");
}

bool PolyspherocylinderTraits::overlapBetween(const Vector<3> &pos1, const Matrix<3, 3> &orientation1,
                                              [[maybe_unused]] const std::byte *data1, std::size_t idx1,
                                              const Vector<3> &pos2, const Matrix<3, 3> &orientation2,
                                              [[maybe_unused]] const std::byte *data2, std::size_t idx2,
                                              const BoundaryConditions &bc) const
{
    const auto &scData1 = this->speciesFor(data1).getSpherocylinderData()[idx1];
    const auto &scData2 = this->speciesFor(data2).getSpherocylinderData()[idx2];

    Vector<3> pos2bc = pos2 + bc.getTranslation(pos1, pos2);
    double distance2 = (pos2bc - pos1).norm2();
    double insphereR = scData1.radius + scData2.radius;
    double insphereR2 = insphereR * insphereR;
    if (distance2 < insphereR2)
        return true;
    double circumsphereR = scData1.circumsphereRadius + scData2.circumsphereRadius;
    double circumsphereR2 = circumsphereR * circumsphereR;
    if (distance2 >= circumsphereR2)
        return false;

    Vector<3> halfAxis1 = orientation1 * scData1.halfAxis;
    Vector<3> halfAxis2 = orientation2 * scData2.halfAxis;
    return SegmentDistanceCalculator::calculate(pos1+halfAxis1, pos1-halfAxis1, pos2bc+halfAxis2, pos2bc-halfAxis2)
           < insphereR2;
}

bool PolyspherocylinderTraits::overlapWithWall(const Vector<3> &pos, const Matrix<3, 3> &orientation,
                                               [[maybe_unused]] const std::byte *data, std::size_t idx,
                                               const Vector<3> &wallOrigin, const Vector<3> &wallVector) const
{
    const auto &spherocylinderData = this->speciesFor(data).getSpherocylinderData()[idx];
    Vector<3> halfAxis = orientation * spherocylinderData.halfAxis;
    double radius = spherocylinderData.radius;

    Vector<3> cap1 = pos - halfAxis;
    double dotProduct1 = wallVector * (cap1 - wallOrigin);
    if (dotProduct1 < radius)
        return true;

    Vector<3> cap2 = pos + halfAxis;
    double dotProduct2 = wallVector * (cap2 - wallOrigin);
    if (dotProduct2 < radius)
        return true;

    return false;
}

std::vector<Vector<3>> PolyspherocylinderTraits::getInteractionCentres(const std::byte *data) const {
    std::vector<Vector<3>> centres;
    const auto &spherocylinderData = this->speciesFor(data).getSpherocylinderData();
    centres.reserve(spherocylinderData.size());
    for (const auto &scData : spherocylinderData)
        centres.push_back(scData.position);
    return centres;
}

double PolyspherocylinderTraits::getRangeRadius(const std::byte *data) const {
    using SpherocylinderData = PolyspherocylinderShape::SpherocylinderData;
    auto comparator = [](const SpherocylinderData &sd1, const SpherocylinderData &sd2) {
        return sd1.circumsphereRadius < sd2.circumsphereRadius;
    };
    const auto &spherocylinderData = this->speciesFor(data).getSpherocylinderData();
    auto maxIt = std::max_element(spherocylinderData.begin(), spherocylinderData.end(), comparator);
    return 2 * maxIt->circumsphereRadius;
}

std::shared_ptr<const ShapePrinter>
PolyspherocylinderTraits::getPrinter(const std::string &format,
                                     const std::map<std::string, std::string> &params) const
{
    std::size_t meshSubdivisions = DEFAULT_MESH_SUBDIVISIONS;
    if (params.find("mesh_divisions") != params.end()) {
        meshSubdivisions = std::stoul(params.at("mesh_divisions"));
        Expects(meshSubdivisions >= 1);
    }

    if (format == "wolfram")
        return this->wolframPrinter;
    else if (format == "obj")
        return this->createObjPrinter(meshSubdivisions);
    else
        throw NoSuchShapePrinterException("PolyspherocylinderTraits: unknown printer format: " + format);
}

std::shared_ptr<ShapePrinter> PolyspherocylinderTraits::createObjPrinter(std::size_t subdivisions) const {
    PolydisperseXCShapePrinter::GeometryComplexProvider provider = [this](const ShapeData &data) {
        const auto &spherocylinderData = this->speciesFor(data).getSpherocylinderData();

        PolydisperseXCShapePrinter::GeometryComplex xcSpheres;
        xcSpheres.reserve(spherocylinderData.size());
        for (const auto &spherocylinderDataEntry : spherocylinderData)
            xcSpheres.emplace_back(spherocylinderDataEntry.createXCGeometry(), spherocylinderDataEntry.position);

        return xcSpheres;
    };

    return std::make_shared<PolydisperseXCObjShapePrinter>(std::move(provider), subdivisions);
}
