//
// Created by Piotr Kubala on 29/02/2024.
//

#include "PolydisperseXCObjShapePrinter.h"


std::string PolydisperseXCObjShapePrinter::doPrint(const Shape &shape,
                                                   const PolyhedronComplex &polyhedronComplex) const
{
    std::ostringstream out;
    std::size_t offset{};
    for (const auto &polyhedron : polyhedronComplex) {
        auto transformedPolyhedron = polyhedron.transformed(shape.getPosition(), shape.getOrientation());
        transformedPolyhedron.storeWavefrontObj(out, offset);
        offset += transformedPolyhedron.vertices.size();
    }
    return out.str();
}
