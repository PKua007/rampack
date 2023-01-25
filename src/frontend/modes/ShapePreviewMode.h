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
    void printInteractionInfo(const Interaction &interaction);
    void printGeometryInfo(const ShapeGeometry &geometry);

public:
    explicit ShapePreviewMode(Logger &logger) : ModeBase(logger) { }

    int main(int argc, char **argv);
};


#endif //RAMPACK_SHAPEPREVIEWMODE_H
