//
// Created by pkua on 18.11.22.
//

#include "WolframWriter.h"


void WolframWriter::write(std::ostream &out, const Packing &packing, const ShapeTraits &traits,
                          [[maybe_unused]] const std::map<std::string, std::string> &auxInfo) const
{
    const auto &shapePrinter = traits.getPrinter("wolfram");
    std::size_t size = packing.size();

    out << std::fixed;
    switch (this->style) {
        case WolframStyle::STANDARD: {
            out << "Graphics3D[{" << std::endl;
            for (std::size_t i{}; i < size; i++) {
                const auto &shape = packing[i];
                out << shapePrinter.print(shape);
                if (i != size - 1)
                    out << "," << std::endl;
            }
            out << "}]" << std::endl;
            break;
        }

        case WolframStyle::AFFINE_TRANSFORM: {
            out << "Graphics3D[GeometricTransformation[" << std::endl;
            out << shapePrinter.print({});
            out << ",AffineTransform@#]& /@ {" << std::endl;
            for (std::size_t i{}; i < size; i++) {
                const auto &pos = packing[i].getPosition();
                const auto &rot = packing[i].getOrientation();
                out << "{{{" << rot(0, 0) << ", " << rot(0, 1) << ", " << rot(0, 2) << "}, ";
                out << "{" << rot(1, 0) << ", " << rot(1, 1) << ", " << rot(1, 2) << "}, ";
                out << "{" << rot(2, 0) << ", " << rot(2, 1) << ", " << rot(2, 2) << "}}, ";
                out << pos << "}";
                if (i != size - 1)
                    out << ",";
                out << std::endl;
            }
            out << "}]" << std::endl;
            break;
        }
    }
}
