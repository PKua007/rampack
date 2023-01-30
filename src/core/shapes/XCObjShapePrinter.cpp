//
// Created by pkua on 21.11.22.
//

#include <sstream>
#include <utility>
#include <ZipIterator.hpp>

#include "XCObjShapePrinter.h"
#include "geometry/xenocollide/XCPrinter.h"
#include "utils/Exceptions.h"


std::string XCObjShapePrinter::print(const Shape &shape) const {
    std::ostringstream out;
    std::size_t offset{};
    for (const auto &[polyhedron, center] : Zip(this->polyhedra, this->interactionCentres)) {
        Matrix<3, 3> rot = shape.getOrientation();
        Vector<3> pos = shape.getPosition() + rot*center;
        auto transformedPolyhedron = polyhedron.transformed(pos, rot);
        transformedPolyhedron.storeWavefrontObj(out, offset);
        offset += transformedPolyhedron.vertices.size();
    }
    return out.str();
}

XCObjShapePrinter::XCObjShapePrinter(const AbstractXCGeometry &geometry, size_t subdivisions)
        : polyhedra{XCPrinter::buildPolyhedron(geometry, subdivisions)}, interactionCentres{{0, 0, 0}}
{ }

XCObjShapePrinter::XCObjShapePrinter(const std::vector<const AbstractXCGeometry *> &geometries,
                                     std::vector<Vector<3>> interactionCentres, size_t subdivisions)
        : polyhedra{buildPolyhedra(geometries, subdivisions)}, interactionCentres{std::move(interactionCentres)}
{
    Expects(!geometries.empty());
    Expects(geometries.size() == this->interactionCentres.size());
}

std::vector<Polyhedron> XCObjShapePrinter::buildPolyhedra(const std::vector<const AbstractXCGeometry *> &geometries,
                                                          std::size_t subdivisions)
{
    std::vector<Polyhedron> polyhedra;
    polyhedra.reserve(geometries.size());
    for (auto geometry : geometries)
        polyhedra.push_back(XCPrinter::buildPolyhedron(*geometry, subdivisions));
    return polyhedra;
}
