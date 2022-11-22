//
// Created by pkua on 18.11.22.
//

#ifndef RAMPACK_WOLFRAMWRITER_H
#define RAMPACK_WOLFRAMWRITER_H

#include "core/SnapshotWriter.h"


class WolframWriter : public SnapshotWriter {
public:
    /**
     * @brief Style of Wolfram Mathematica packing output.
     */
    enum class WolframStyle {
        /**
         * @brief A standard style - each shape is printed separately.
         * @details The output is Graphics3D with a list of shapes, which is easy to process but may grow large. It is
         * advised for shapes with a small footprint.
         */
        STANDARD,

        /**
         * @brief A single shape with the default position and orientation is printed and then, for each shape in the
         * packing it is copied and appropriate affine transformation is applied to it.
         * @details The output is Graphics3D with a list of positions and orientations which is mapped over a single
         * shape using @a AffineTransform. Is is advised for shapes with a large footprint and for easy access to
         * positions and orientations of the shapes.
         */
        AFFINE_TRANSFORM
    };

private:
    WolframStyle style{};

    static void storeStandard(std::ostream &out, const Packing &packing, const ShapePrinter &shapePrinter);
    static void storeAffineTransform(std::ostream &out, const Packing &packing, const ShapePrinter &shapePrinter);

public:
    explicit WolframWriter(WolframStyle style = WolframStyle::STANDARD) : style{style} { }

    [[nodiscard]] WolframStyle getStyle() const { return style; }
    void setStyle(WolframStyle style_) { this->style = style_; }

    void write(std::ostream &out, const Packing &packing, const ShapeTraits &traits,
               const std::map<std::string, std::string> &auxInfo) const override;
};


#endif //RAMPACK_WOLFRAMWRITER_H
