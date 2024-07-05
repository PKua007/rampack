//
// Created by pkua on 21.11.22.
//

#ifndef RAMPACK_XCWOLFRAMSHAPEPRINTER_H
#define RAMPACK_XCWOLFRAMSHAPEPRINTER_H

#include "core/ShapePrinter.h"
#include "geometry/xenocollide/AbstractXCGeometry.h"
#include "geometry/Polyhedron.h"


/**
 * @brief Shape printer printing the shape in Wolfram Mathematica format using XenoCollide AbstractXCGeometry to
 * generate shape mesh (`GraphicsComplex`).
 */
class XCWolframShapePrinter : public ShapePrinter {
private:
    std::vector<Polyhedron> polyhedra;
    std::vector<Vector<3>> interactionCentres;

    static std::vector<Polyhedron> buildPolyhedra(const std::vector<const AbstractXCGeometry *> &geometries,
                                                  std::size_t subdivisions);

public:
    /**
     * @brief Constructs the class for a single interaction center specified by @a geometry.
     * @param geometry geometry of teh interaction center
     * @param subdivisions see XCPrinter::buildPolyhedron
     */
    XCWolframShapePrinter(const AbstractXCGeometry &geometry, size_t subdivisions);

    /**
     * @brief Constructs the class for multiple interaction centers.
     * @param geometries geometries describing subsequent interaction centres
     * @param interactionCentres interaction centres' positions
     * @param subdivisions see XCPrinter::buildPolyhedron
     */
    XCWolframShapePrinter(const std::vector<const AbstractXCGeometry *> &geometries,
                          std::vector<Vector<3>> interactionCentres, size_t subdivisions);

    [[nodiscard]] std::string print(const Shape &shape) const override;
};


#endif //RAMPACK_XCWOLFRAMSHAPEPRINTER_H
