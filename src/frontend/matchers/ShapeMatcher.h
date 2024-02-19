//
// Created by Piotr Kubala on 16/12/2022.
//

#ifndef RAMPACK_SHAPEMATCHER_H
#define RAMPACK_SHAPEMATCHER_H

#include <memory>

#include "pyon/Matcher.h"
#include "core/ShapeTraits.h"


class ShapeMatcher {
public:
    static pyon::matcher::MatcherAlternative create();
    static std::shared_ptr<ShapeTraits> match(const std::string &expression);
};


#endif //RAMPACK_SHAPEMATCHER_H
