//
// Created by Piotr Kubala on 29/02/2024.
//

#include "PolydisperseXCWolframShapePrinter.h"


std::string PolydisperseXCWolframShapePrinter::doPrint(const Shape &shape,
                                                       const PolyhedronComplex &polyhedronComplex) const
{
    std::ostringstream out;
    out << std::fixed;
    out << "{EdgeForm[None]," << std::endl;

    for (std::size_t i{}; i < polyhedronComplex.size(); i++) {
        const auto &polyhedron = polyhedronComplex[i];
        const auto &transformed = polyhedron.transformed(shape.getPosition(), shape.getOrientation());
        transformed.storeWolframGraphicsComplex(out);
        if (i < polyhedronComplex.size() - 1)
            out << ",";
        out << std::endl;
    }
    out << "}";

    return out.str();
}
