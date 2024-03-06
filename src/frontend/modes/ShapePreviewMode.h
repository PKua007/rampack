//
// Created by Piotr Kubala on 25/01/2023.
//

#ifndef RAMPACK_SHAPEPREVIEWMODE_H
#define RAMPACK_SHAPEPREVIEWMODE_H

#include "frontend/ModeBase.h"
#include "core/Interaction.h"
#include "core/ShapeGeometry.h"


class ShapePreviewMode : public ModeBase {
private:
    [[nodiscard]] static Shape createTrialShape(const std::string &shapePos, const std::string &shapeRot,
                                                const std::string &shapeParams, const ShapeDataManager &manager);

    void printInteractionInfo(const Interaction &interaction, const Shape &trialShape);
    void printGeometryInfo(const ShapeGeometry &geometry, const Shape &trialShape);

public:
    explicit ShapePreviewMode(Logger &logger) : ModeBase(logger) { }

    int main(int argc, char **argv);
};


#endif //RAMPACK_SHAPEPREVIEWMODE_H
