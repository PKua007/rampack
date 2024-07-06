//
// Created by Piotr Kubala on 29/02/2024.
//

#ifndef RAMPACK_POLYDISPERSEXCOBJSHAPEPRINTER_H
#define RAMPACK_POLYDISPERSEXCOBJSHAPEPRINTER_H

#include "PolydisperseXCShapePrinter.h"


/**
 * @brief PolydisperseXCShapePrinter for the Wavefront OBJ format.
 */
class PolydisperseXCObjShapePrinter : public PolydisperseXCShapePrinter {
protected:
    std::string doPrint(const Shape &shape, const PolyhedronComplex &polyhedronComplex) const override;

public:
    using PolydisperseXCShapePrinter::PolydisperseXCShapePrinter;
};


#endif //RAMPACK_POLYDISPERSEXCOBJSHAPEPRINTER_H
