//
// Created by Piotr Kubala on 20/12/2020.
//

#include "SpherocylinderTraits.h"
#include "utils/Assertions.h"
#include "geometry/SegmentDistanceCalculator.h"
#include "geometry/xenocollide/XCBodyBuilder.h"
#include "XCObjShapePrinter.h"


SpherocylinderTraits::SpherocylinderTraits(double length, double radius)
        : length{length}, radius{radius}, wolframPrinter{std::make_shared<WolframPrinter>(*this)}
{
    Expects(length >= 0);
    Expects(radius > 0);

    this->registerNamedPoint("cm", {0, 0, 0});
    this->registerNamedPoint("beg", this->getCapCentre(-1, {}));
    this->registerNamedPoint("end", this->getCapCentre(1, {}));
    this->objPrinter = SpherocylinderTraits::createObjPrinter(length, radius);

}

Vector<3> SpherocylinderTraits::getCapCentre(short beginOrEnd, const Shape &shape) const {
    return shape.getPosition() + shape.getOrientation().column(2) * (0.5 * beginOrEnd * this->length);
}

bool SpherocylinderTraits::overlapBetween(const Vector<3> &pos1, const Matrix<3, 3> &orientation1,
                                          [[maybe_unused]] std::size_t idx1, const Vector<3> &pos2,
                                          const Matrix<3, 3> &orientation2, [[maybe_unused]] std::size_t idx2,
                                          const BoundaryConditions &bc) const
{
    Vector<3> pos2bc = pos2 + bc.getTranslation(pos1, pos2);
    double distance2 = (pos2bc - pos1).norm2();
    if (distance2 < 4 * this->radius * this->radius)
        return true;
    else if (distance2 >= std::pow(2 * this->radius + this->length, 2))
        return false;

    Shape shape1(pos1, orientation1);
    Shape shape2(pos2bc, orientation2);

    return SegmentDistanceCalculator::calculate(this->getCapCentre(-1, shape1), this->getCapCentre(1, shape1),
                                                this->getCapCentre(-1, shape2),this->getCapCentre(1, shape2))
           < 4 * this->radius * this->radius;
}

double SpherocylinderTraits::getVolume() const {
    return M_PI*this->radius*this->radius*this->length + 4./3*M_PI*std::pow(this->radius, 3);
}

Vector<3> SpherocylinderTraits::getPrimaryAxis(const Shape &shape) const {
    return shape.getOrientation().column(2);
}

bool SpherocylinderTraits::overlapWithWall(const Vector<3> &pos, const Matrix<3, 3> &orientation,
                                           [[maybe_unused]] std::size_t idx, const Vector<3> &wallOrigin,
                                           const Vector<3> &wallVector) const
{
    Vector<3> halfAxis = orientation.column(2) * (this->length/2);

    Vector<3> cap1 = pos + halfAxis;
    double dotProduct1 = wallVector * (cap1 - wallOrigin);
    if (dotProduct1 < this->radius)
        return true;

    Vector<3> cap2 = pos - halfAxis;
    double dotProduct2 = wallVector * (cap2 - wallOrigin);
    if (dotProduct2 < this->radius)
        return true;

    return false;
}

std::shared_ptr<const ShapePrinter>
SpherocylinderTraits::getPrinter(const std::string &format, const std::map<std::string, std::string> &params) const {
    if (format == "wolfram")
        return this->wolframPrinter;
    else if (format == "obj")
        return this->objPrinter;
    else
        throw NoSuchShapePrinterException("SphereTraits: unknown printer format: " + format);
}

std::unique_ptr<ShapePrinter> SpherocylinderTraits::createObjPrinter(double length, double radius) {
    XCBodyBuilder builder;
    builder.sphere(radius);
    builder.move(0, 0, -length/2);
    builder.sphere(radius);
    builder.move(0, 0, length/2);
    builder.wrap();

    return std::make_unique<XCObjShapePrinter>(*builder.releaseCollideGeometry(), 4);
}

std::string SpherocylinderTraits::WolframPrinter::print(const Shape &shape) const {
    std::stringstream out;
    out << std::fixed;
    Vector<3> beg = this->traits.getCapCentre(-1, shape);
    Vector<3> end = this->traits.getCapCentre(1, shape);
    out << "CapsuleShape[{" << beg << "," << end << "}," << this->traits.radius << "]";
    return out.str();
}
