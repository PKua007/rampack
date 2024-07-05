//
// Created by pkua on 21.11.22.
//

#include "XCWolframShapePrinter.h"


#include <sstream>
#include <utility>

#include "geometry/xenocollide/XCPrinter.h"
#include "utils/Exceptions.h"


std::string XCWolframShapePrinter::print(const Shape &shape) const {
    std::ostringstream out;
    out << std::fixed;
    out << "{EdgeForm[None]," << std::endl;

    Matrix<3, 3> rot = shape.getOrientation();
    for (std::size_t i{}; i < this->interactionCentres.size(); i++) {
        const auto &center = this->interactionCentres[i];
        const auto &polyhedron = this->polyhedra[i];

        Vector<3> pos = shape.getPosition() + rot * center;

        out << "GeometricTransformation[";
        polyhedron.storeWolframGraphicsComplex(out);
        out << "," << std::endl;
        out << "AffineTransform[" << std::endl;
        out << "    {{{" << rot(0, 0) << ", " << rot(0, 1) << ", " << rot(0, 2) << "}," << std::endl;
        out << "      {" << rot(1, 0) << ", " << rot(1, 1) << ", " << rot(1, 2) << "}," << std::endl;
        out << "      {" << rot(2, 0) << ", " << rot(2, 1) << ", " << rot(2, 2) << "}}," << std::endl;
        out << "      " << pos << "}]" << std::endl;
        out << "]";

        if (i < this->interactionCentres.size() - 1)
            out << ",";
        out << std::endl;
    }
    out << "}";

    return out.str();
}

XCWolframShapePrinter::XCWolframShapePrinter(const AbstractXCGeometry &geometry, size_t subdivisions)
        : polyhedra{XCPrinter::buildPolyhedron(geometry, subdivisions)}, interactionCentres{{0, 0, 0}}
{ }

XCWolframShapePrinter::XCWolframShapePrinter(const std::vector<const AbstractXCGeometry *> &geometries,
                                             std::vector<Vector<3>> interactionCentres, size_t subdivisions)
        : polyhedra{buildPolyhedra(geometries, subdivisions)}, interactionCentres{std::move(interactionCentres)}
{
    Expects(!geometries.empty());
    Expects(geometries.size() == this->interactionCentres.size());
}

std::vector<Polyhedron> XCWolframShapePrinter::buildPolyhedra(const std::vector<const AbstractXCGeometry *> &geometries,
                                                              std::size_t subdivisions)
{
    std::vector<Polyhedron> polyhedra;
    polyhedra.reserve(geometries.size());
    for (auto geometry : geometries)
        polyhedra.push_back(XCPrinter::buildPolyhedron(*geometry, subdivisions));
    return polyhedra;
}
