//
// Created by pkua on 18.11.22.
//

#ifndef RAMPACK_WOLFRAMWRITER_H
#define RAMPACK_WOLFRAMWRITER_H

#include "core/SnapshotWriter.h"


/**
 * @brief Stores a snapshot in Wolfram Mathematica format.
 */
class WolframWriter : public SnapshotWriter {
public:
    /**
     * @brief Style of Wolfram Mathematica snapshot output.
     */
    enum class WolframStyle {
        /**
         * @brief A standard style - each shape is printed separately.
         * @details The output is Graphics3D with a list of shapes, which is easy to process but may grow large. This
         * style advised for shapes with a small Wolfram code footprint.
         */
        STANDARD,

        /**
         * @brief Shapes are grouped by their ShapeData. For each group, a single shape with the default position
         * and orientation is printed, and a mapping over an array of appropriate affine transformations is applied to
         * it to accommodate for each shape in the group.
         * @details More precisely, the output is Graphics3D with a list of lambdas, which map position and orientation
         * to a shape with fixed ShapeData translated and rotates as given by the arguments. Consequently, a separate
         * lambda function is generated for all ShapeData groups. Then, each lambda is mapped over a list of positions
         * and orientations, all of which constitutes the whole packing.
         *
         * This style is advised for shapes with a large footprint and, alternatively, when an easy access to positions
         * and orientations of the shapes is needed.
         */
        AFFINE_TRANSFORM
    };

private:
    using ShapeGroup = std::pair<ShapeData, std::vector<Shape>>;

    WolframStyle style{};
    std::map<std::string, std::string> params{};

    static void storeStandard(std::ostream &out, const Packing &packing, const ShapePrinter &shapePrinter);
    static void storeAffineTransform(std::ostream &out, const Packing &packing, const ShapePrinter &shapePrinter);
    static std::vector<ShapeGroup> groupShapesByData(const Packing &packing);
    static void printShapeGroup(std::ostream &out, const ShapeGroup &shapeGroup, const ShapePrinter &shapePrinter);

public:
    /**
     * @brief Creates the class.
     * @param style style of the output
     * @param params auxiliary parameters passed to Wolfram ShapePrinter
     */
    explicit WolframWriter(WolframStyle style = WolframStyle::STANDARD, std::map<std::string, std::string> params = {})
        : style{style}, params{std::move(params)} { }

    [[nodiscard]] WolframStyle getStyle() const { return style; }
    void setStyle(WolframStyle style_) { this->style = style_; }

    void write(std::ostream &out, const Packing &packing, const ShapeTraits &traits,
               const std::map<std::string, std::string> &auxInfo) const override;
};


#endif //RAMPACK_WOLFRAMWRITER_H
