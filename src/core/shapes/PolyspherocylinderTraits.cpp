//
// Created by Piotr Kubala on 26/04/2021.
//

#include <numeric>
#include <algorithm>
#include <iterator>

#include "PolyspherocylinderTraits.h"
#include "utils/Assertions.h"
#include "geometry/SegmentDistanceCalculator.h"
#include "XCObjShapePrinter.h"
#include "geometry/xenocollide/XCBodyBuilder.h"


double PolyspherocylinderTraits::PolyspherocylinderGeometry::calculateVolume() const {
    auto volumeAccumulator = [](double volume_, const SpherocylinderData &data) {
        return volume_ + data.getVolume();
    };
    return std::accumulate(this->spherocylinderData.begin(), this->spherocylinderData.end(), 0., volumeAccumulator);
}

PolyspherocylinderTraits::PolyspherocylinderGeometry
    ::PolyspherocylinderGeometry(std::vector<SpherocylinderData> spherocylinderData, OptionalAxis primaryAxis,
                                 OptionalAxis secondaryAxis, const Vector<3> &geometricOrigin,
                                 std::optional<double> volume, const ShapeGeometry::NamedPoints &customNamedPoints)
        : spherocylinderData{std::move(spherocylinderData)}, primaryAxis{primaryAxis}, secondaryAxis{secondaryAxis},
          geometricOrigin{geometricOrigin}
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
        this->registerNamedPoint("o" + iStr, scData.position);
        this->registerNamedPoint("b" + iStr, scData.position - scData.halfAxis);
        this->registerNamedPoint("e" + iStr, scData.position + scData.halfAxis);
    }

    this->registerNamedPoints(customNamedPoints);
}

PolyspherocylinderTraits::SpherocylinderData::SpherocylinderData(const Vector<3> &position, const Vector<3> &halfAxis,
                                                                 double radius)
        : position{position}, halfAxis{halfAxis}, halfLength{halfAxis.norm()}, radius{radius},
          circumsphereRadius{radius + halfLength}
{
    Expects(radius > 0);
}

void PolyspherocylinderTraits::SpherocylinderData::toWolfram(std::ostream &out, const Shape &shape) const {
    Vector<3> shapeHalfAxis = this->halfAxisForShape(shape);
    Vector<3> beg = shape.getPosition() + shape.getOrientation() * this->position + shapeHalfAxis;
    Vector<3> end = shape.getPosition() + shape.getOrientation() * this->position - shapeHalfAxis;

    out << "Tube[{" << beg << "," << end << "}," << this->radius << "]";
}

Vector<3> PolyspherocylinderTraits::SpherocylinderData::centreForShape(const Shape &shape) const {
    return shape.getPosition() + shape.getOrientation() * this->position;
}

double PolyspherocylinderTraits::SpherocylinderData::getVolume() const {
    double radius2 = this->radius * this->radius;
    double radius3 = radius2 * this->radius;
    double axisLength = 2 * this->halfLength;
    return 4*M_PI/3*radius3 + M_PI*radius2*axisLength;
}

Vector<3> PolyspherocylinderTraits::SpherocylinderData::halfAxisForShape(const Shape &shape) const {
    return shape.getOrientation() * this->halfAxis;
}

bool PolyspherocylinderTraits::overlapBetween(const Vector<3> &pos1, const Matrix<3, 3> &orientation1, std::size_t idx1,
                                              const Vector<3> &pos2, const Matrix<3, 3> &orientation2, std::size_t idx2,
                                              const BoundaryConditions &bc) const
{
    const auto &spherocylinderData = this->getSpherocylinderData();
    const auto &data1 = spherocylinderData[idx1];
    const auto &data2 = spherocylinderData[idx2];

    Vector<3> pos2bc = pos2 + bc.getTranslation(pos1, pos2);
    double distance2 = (pos2bc - pos1).norm2();
    double insphereR = data1.radius + data2.radius;
    double insphereR2 = insphereR * insphereR;
    if (distance2 < insphereR2)
        return true;
    double circumsphereR = data1.circumsphereRadius + data2.circumsphereRadius;
    double circumsphereR2 = circumsphereR * circumsphereR;
    if (distance2 >= circumsphereR2)
        return false;

    Vector<3> halfAxis1 = orientation1 * data1.halfAxis;
    Vector<3> halfAxis2 = orientation2 * data2.halfAxis;
    return SegmentDistanceCalculator::calculate(pos1+halfAxis1, pos1-halfAxis1, pos2bc+halfAxis2, pos2bc-halfAxis2)
           < insphereR2;
}

std::vector<Vector<3>> PolyspherocylinderTraits::getInteractionCentres() const {
    std::vector<Vector<3>> centres;
    const auto &spherocylinderData = this->getSpherocylinderData();
    centres.reserve(spherocylinderData.size());
    for (const auto &data : spherocylinderData)
        centres.push_back(data.position);
    return centres;
}

double PolyspherocylinderTraits::getRangeRadius() const {
    auto comparator = [](const SpherocylinderData &sd1, const SpherocylinderData &sd2) {
        return sd1.circumsphereRadius < sd2.circumsphereRadius;
    };
    const auto &spherocylinderData = this->getSpherocylinderData();
    auto maxIt = std::max_element(spherocylinderData.begin(), spherocylinderData.end(), comparator);
    return 2 * maxIt->circumsphereRadius;
}

bool PolyspherocylinderTraits::overlapWithWall(const Vector<3> &pos, const Matrix<3, 3> &orientation, std::size_t idx,
                                               const Vector<3> &wallOrigin, const Vector<3> &wallVector) const
{
    const auto &spherocylinderData = this->getSpherocylinderData()[idx];
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

PolyspherocylinderTraits::PolyspherocylinderTraits(PolyspherocylinderTraits::PolyspherocylinderGeometry geometry)
        : geometry{std::move(geometry)}, wolframPrinter{std::make_shared<WolframPrinter>(*this)}
{

}

std::shared_ptr<ShapePrinter> PolyspherocylinderTraits::createObjPrinter(std::size_t subdivisions) const {
    const auto &spherocylinderData = this->getSpherocylinderData();

    std::vector<std::shared_ptr<AbstractXCGeometry>> xcSpherocylinders;
    std::vector<const AbstractXCGeometry *> geometries;
    xcSpherocylinders.reserve(spherocylinderData.size());
    geometries.reserve(spherocylinderData.size());
    for (const auto &scData : spherocylinderData) {
        xcSpherocylinders.push_back(PolyspherocylinderTraits::buildXCSpherocylinder(scData));
        geometries.push_back(xcSpherocylinders.back().get());
    }

    auto interactionCentres = this->getInteractionCentres();

    return std::make_shared<XCObjShapePrinter>(geometries, interactionCentres, subdivisions);
}

std::shared_ptr<AbstractXCGeometry> PolyspherocylinderTraits::buildXCSpherocylinder(const SpherocylinderData &scData) {
    XCBodyBuilder builder;
    builder.sphere(scData.radius);
    Vector<3> pos1 = -scData.halfAxis;
    builder.move(pos1[0], pos1[1], pos1[2]);
    builder.sphere(scData.radius);
    Vector<3> pos2 = scData.halfAxis;
    builder.move(pos2[0], pos2[1], pos2[2]);
    builder.wrap();
    return builder.releaseCollideGeometry();
}

std::string PolyspherocylinderTraits::WolframPrinter::print(const Shape &shape) const {
    std::ostringstream out;
    out << std::fixed;
    out << "{";
    const auto &spherocylinderData = this->traits.getSpherocylinderData();
    for (std::size_t i{}; i < spherocylinderData.size() - 1; i++) {
        const auto &data = spherocylinderData[i];
        data.toWolfram(out, shape);
        out << ",";
    }
    spherocylinderData.back().toWolfram(out, shape);
    out << "}";
    return out.str();
}
