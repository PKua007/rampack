//
// Created by Piotr Kubala on 29/02/2024.
//

#include "PolydisperseXCObjShapePrinter.h"


std::string PolydisperseXCObjShapePrinter::doPrint(const Shape &shape,
                                                   const PolyhedronComplex &polyhedronComplex) const
{
    std::ostringstream out;
    std::size_t offset{};
    for (const auto &[center, polyhedron] : polyhedronComplex) {
        Matrix<3, 3> rot = shape.getOrientation();
        Vector<3> pos = shape.getPosition() + rot*center;
        auto transformedPolyhedron = polyhedron.transformed(pos, rot);
        transformedPolyhedron.storeWavefrontObj(out, offset);
        offset += transformedPolyhedron.vertices.size();
    }
    return out.str();
}
