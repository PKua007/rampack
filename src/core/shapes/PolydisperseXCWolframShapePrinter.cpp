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

    Matrix<3, 3> rot = shape.getOrientation();
    for (std::size_t i{}; i < polyhedronComplex.size(); i++) {
        const auto &polyhedron = polyhedronComplex[i];

        Vector<3> pos = shape.getPosition() + rot * polyhedron.center;

        out << "GeometricTransformation[";
        polyhedron.polyhedron.storeWolframGraphicsComplex(out);
        out << "," << std::endl;
        out << "AffineTransform[" << std::endl;
        out << "    {{{" << rot(0, 0) << ", " << rot(0, 1) << ", " << rot(0, 2) << "}," << std::endl;
        out << "      {" << rot(1, 0) << ", " << rot(1, 1) << ", " << rot(1, 2) << "}," << std::endl;
        out << "      {" << rot(2, 0) << ", " << rot(2, 1) << ", " << rot(2, 2) << "}}," << std::endl;
        out << "      " << pos << "}]" << std::endl;
        out << "]";

        if (i < polyhedronComplex.size() - 1)
            out << ",";
        out << std::endl;
    }
    out << "}";

    return out.str();
}
