//
// Created by pkua on 21.11.22.
//

#ifndef RAMPACK_XCOBJSHAPEPRINTER_H
#define RAMPACK_XCOBJSHAPEPRINTER_H

#include "core/ShapePrinter.h"
#include "geometry/xenocollide/AbstractXCGeometry.h"
#include "geometry/Polyhedron.h"


/**
 * @brief Shape printer printing the shape in Wavefront OBJ format using XenoCollide AbstractXCGeometry to generate
 * shape mesh.
 */
class XCObjShapePrinter : public ShapePrinter {
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
    XCObjShapePrinter(const AbstractXCGeometry &geometry, size_t subdivisions);

    /**
     * @brief Constructs the class for multiple interaction centers.
     * @param geometries geometries describing subsequent interaction centres
     * @param interactionCentres interaction centres' positions
     * @param subdivisions see XCPrinter::buildPolyhedron
     */
    XCObjShapePrinter(const std::vector<const AbstractXCGeometry *> &geometries,
                      std::vector<Vector<3>> interactionCentres, size_t subdivisions);

    [[nodiscard]] std::string print(const Shape &shape) const override;
};


#endif //RAMPACK_XCOBJSHAPEPRINTER_H
