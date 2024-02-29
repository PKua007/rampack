//
// Created by Piotr Kubala on 29/02/2024.
//

#ifndef RAMPACK_POLYDISPERSEXCWOLFRAMSHAPEPRINTER_H
#define RAMPACK_POLYDISPERSEXCWOLFRAMSHAPEPRINTER_H

#include "PolydisperseXCShapePrinter.h"


class PolydisperseXCWolframShapePrinter : public PolydisperseXCShapePrinter {
protected:
    std::string doPrint(const Shape &shape, const PolyhedronComplex &polyhedronComplex) const override;

public:
    using PolydisperseXCShapePrinter::PolydisperseXCShapePrinter;
};


#endif //RAMPACK_POLYDISPERSEXCWOLFRAMSHAPEPRINTER_H
