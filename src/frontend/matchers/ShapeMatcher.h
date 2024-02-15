//
// Created by Piotr Kubala on 16/12/2022.
//

#ifndef RAMPACK_SHAPEMATCHER_H
#define RAMPACK_SHAPEMATCHER_H

#include <memory>

#include "pyon/Matcher.h"
#include "core/ShapeTraits.h"


struct DefaultedShapeTraits {
    std::shared_ptr<ShapeTraits> traits{};
    TextualShapeData defaultShapeData{};
};

class ShapeMatcher {
public:
    static pyon::matcher::MatcherAlternative create();
    static DefaultedShapeTraits match(const std::string &expression);
};


#endif //RAMPACK_SHAPEMATCHER_H
