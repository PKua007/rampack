//
// Created by Piotr Kubala on 20/12/2020.
//

#include <cmath>
#include <sstream>

#include "SphereTraits.h"
#include "utils/Exceptions.h"
#include "XCObjShapePrinter.h"
#include "geometry/xenocollide/XCPrimitives.h"


SphereTraits::SphereTraits(double radius)
        : radius{radius}, interaction{std::make_shared<HardInteraction>(radius)},
          wolframPrinter{std::make_shared<WolframPrinter>(radius)}
{
    Expects(radius > 0);
    this->registerNamedPoint("cm", {0, 0, 0});
}

SphereTraits::SphereTraits(double radius, std::shared_ptr<CentralInteraction> centralInteraction)
        : radius{radius}, wolframPrinter{std::make_shared<WolframPrinter>(radius)}
{
    Expects(radius > 0);
    centralInteraction->installOnSphere();
    this->interaction = std::move(centralInteraction);
    this->registerNamedPoint("cm", {0, 0, 0});
}

double SphereTraits::getVolume([[maybe_unused]] const Shape &shape) const {
    return 4./3 * M_PI * std::pow(this->radius, 3);
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
    else if (format == "obj")
        return createObjPrinter(this->radius, meshSubdivisions);
    else
        throw NoSuchShapePrinterException("SphereTraits: unknown printer format: " + format);
}

std::shared_ptr<ShapePrinter> SphereTraits::createObjPrinter(double radius, std::size_t subdivisions) {
    return std::make_unique<XCObjShapePrinter>(XCSphere{radius}, subdivisions);
}

bool SphereTraits::HardInteraction::overlapBetween(const Vector<3> &pos1,
                                                   [[maybe_unused]] const Matrix<3, 3> &orientation1,
                                                   [[maybe_unused]] const std::byte *data1,
                                                   [[maybe_unused]] std::size_t idx1,
                                                   const Vector<3> &pos2,
                                                   [[maybe_unused]] const Matrix<3, 3> &orientation2,
                                                   [[maybe_unused]] const std::byte *data2,
                                                   [[maybe_unused]] std::size_t idx2,
                                                   const BoundaryConditions &bc) const
{
    return bc.getDistance2(pos1, pos2) < std::pow(2 * this->radius, 2);
}

bool SphereTraits::HardInteraction::overlapWithWall(const Vector<3> &pos,
                                                    [[maybe_unused]] const Matrix<3, 3> &orientation,
                                                    [[maybe_unused]] const std::byte *data,
                                                    [[maybe_unused]] std::size_t idx,
                                                    const Vector<3> &wallOrigin,
                                                    const Vector<3> &wallVector) const
{
    double dotProduct = wallVector * (pos - wallOrigin);
    return dotProduct < this->radius;
}

std::string SphereTraits::WolframPrinter::print(const Shape &shape) const {
    std::ostringstream out;
    out << "Sphere[" << (shape.getPosition()) << "," << this->radius << "]";
    return out.str();
}
