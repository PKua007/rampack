//
// Created by pkua on 18.11.22.
//

#include <vector>
#include "WolframWriter.h"


void WolframWriter::write(std::ostream &out, const Packing &packing, const ShapeTraits &traits,
                          [[maybe_unused]] const std::map<std::string, std::string> &auxInfo) const
{
    out << std::fixed;

    auto shapePrinter = traits.getPrinter("wolfram", this->params);
    switch (this->style) {
        case WolframStyle::STANDARD:
            WolframWriter::storeStandard(out, packing, *shapePrinter);
            break;
        case WolframStyle::AFFINE_TRANSFORM:
            WolframWriter::storeAffineTransform(out, packing, *shapePrinter);
            break;
    }
}

void WolframWriter::storeStandard(std::ostream &out, const Packing &packing, const ShapePrinter &shapePrinter) {
    std::size_t size = packing.size();

    out << "Graphics3D[{" << std::endl;
    for (std::size_t i{}; i < size; i++) {
        const auto &shape = packing[i];
        out << shapePrinter.print(shape);
        if (i != size - 1)
            out << "," << std::endl;
    }
    out << "}]" << std::endl;
}

void WolframWriter::storeAffineTransform(std::ostream &out, const Packing &packing,
                                         const ShapePrinter &shapePrinter)
{
    std::vector<ShapeGroup> shapeGroups = WolframWriter::groupShapesByData(packing);
    switch (shapeGroups.size()) {
        case 0:
            out << "Graphics3D[]" << std::endl;
            break;

        case 1:
            out << "Graphics3D[";
            WolframWriter::printShapeGroup(out, shapeGroups.front(), shapePrinter);
            out << "]" << std::endl;
            break;

        default:
            out << "Graphics3D[{" << std::endl;
            std::size_t numGroups = shapeGroups.size();
            for (std::size_t i{}; i < numGroups; i++) {
                const auto &shapeGroup = shapeGroups[i];
                WolframWriter::printShapeGroup(out, shapeGroup, shapePrinter);
                if (i < numGroups - 1)
                    out << ",";
                out << std::endl;
            }
            out << "}]" << std::endl;
            break;
    }
}

std::vector<WolframWriter::ShapeGroup> WolframWriter::groupShapesByData(const Packing &packing) {
    std::vector<ShapeGroup> groupedShapes;
    for (const auto &shape : packing) {
        auto groupIt = std::find_if(groupedShapes.begin(), groupedShapes.end(), [&shape](const ShapeGroup &group) {
            return group.first == shape.getData();
        });
        if (groupIt == groupedShapes.end())
            groupIt = groupedShapes.insert(groupedShapes.end(), {shape.getData(), {}});

        groupIt->second.emplace_back(shape.getPosition(), shape.getOrientation());
    }

    return groupedShapes;
}

void WolframWriter::printShapeGroup(std::ostream &out, const WolframWriter::ShapeGroup &shapeGroup,
                                    const ShapePrinter &shapePrinter)
{
    const auto &shapeData = shapeGroup.first;
    const auto &shapes = shapeGroup.second;

    out << "GeometricTransformation[" << std::endl;
    out << shapePrinter.print(Shape({}, Matrix<3, 3>::identity(), shapeData));
    out << ",AffineTransform@#]& /@ {" << std::endl;

    std::size_t size = shapes.size();
    for (std::size_t i{}; i < size; i++) {
        const auto &shape = shapes[i];
        const auto pos = shape.getPosition();
        const auto rot = shape.getOrientation();
        out << "{{{" << rot(0, 0) << ", " << rot(0, 1) << ", " << rot(0, 2) << "}, ";
        out << "{" << rot(1, 0) << ", " << rot(1, 1) << ", " << rot(1, 2) << "}, ";
        out << "{" << rot(2, 0) << ", " << rot(2, 1) << ", " << rot(2, 2) << "}}, ";
        out << pos << "}";
        if (i != size - 1)
            out << ",";
        out << std::endl;
    }

    out << "}";
}
