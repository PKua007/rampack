//
// Created by pkua on 21.11.22.
//

#ifndef RAMPACK_XCWOLFRAMSHAPEPRINTER_H
#define RAMPACK_XCWOLFRAMSHAPEPRINTER_H

#include "core/ShapePrinter.h"
#include "geometry/xenocollide/AbstractXCGeometry.h"
#include "geometry/Polyhedron.h"


class XCWolframShapePrinter : public ShapePrinter {
private:
    std::vector<Polyhedron> polyhedra;
    std::vector<Vector<3>> interactionCentres;

    static std::vector<Polyhedron> buildPolyhedra(const std::vector<const AbstractXCGeometry *> &geometries,
                                                  std::size_t subdivisions);

public:
    XCWolframShapePrinter(const AbstractXCGeometry &geometry, size_t subdivisions);
    XCWolframShapePrinter(const std::vector<const AbstractXCGeometry *> &geometries,
                          std::vector<Vector<3>> interactionCentres, size_t subdivisions);

    [[nodiscard]] std::string print(const Shape &shape) const override;
};


#endif //RAMPACK_XCWOLFRAMSHAPEPRINTER_H
