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
    static pyon::matcher::MatcherArray createPosition();
    static Vector<3> matchPosition(const std::string &expression);
    static pyon::matcher::MatcherArray createOrientation();
    static Matrix<3, 3> matchOrientation(const std::string &expression);
    static pyon::matcher::MatcherDictionary createShapeData();
    static TextualShapeData matchShapeData(const std::string &expression);
};


#endif //RAMPACK_SHAPEMATCHER_H
