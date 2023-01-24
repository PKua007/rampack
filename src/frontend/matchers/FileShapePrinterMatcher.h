//
// Created by Piotr Kubala on 23/01/2023.
//

#ifndef RAMPACK_FILESHAPEPRINTERMATCHER_H
#define RAMPACK_FILESHAPEPRINTERMATCHER_H

#include "pyon/Matcher.h"
#include "core/ShapeTraits.h"
#include "frontend/FileShapePrinter.h"


class FileShapePrinterMatcher {
public:
    static pyon::matcher::MatcherAlternative create(const ShapeTraits &traits);
    static FileShapePrinter match(const std::string &expression, const ShapeTraits &traits);
};


#endif //RAMPACK_FILESHAPEPRINTERMATCHER_H
