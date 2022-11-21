//
// Created by pkua on 21.11.22.
//

#ifndef RAMPACK_XCOBJSHAPEPRINTER_H
#define RAMPACK_XCOBJSHAPEPRINTER_H

#include "core/ShapePrinter.h"
#include "geometry/xenocollide/AbstractXCGeometry.h"
#include "geometry/Polyhedron.h"


class XCObjShapePrinter : public ShapePrinter {
private:
    std::vector<Polyhedron> polyhedra;
    std::vector<Vector<3>> interactionCentres;

    static std::vector<Polyhedron> buildPolyhedra(const std::vector<const AbstractXCGeometry *> &geometries,
                                                  std::size_t subdivisions);

public:
    XCObjShapePrinter(const AbstractXCGeometry &geometry, size_t subdivisions);
    XCObjShapePrinter(const std::vector<const AbstractXCGeometry *> &geometries,
                      std::vector<Vector<3>> interactionCentres, size_t subdivisions);

    [[nodiscard]] std::string print(const Shape &shape) const override;
};


#endif //RAMPACK_XCOBJSHAPEPRINTER_H
