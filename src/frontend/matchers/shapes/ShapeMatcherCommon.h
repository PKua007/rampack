//
// Created by Codex on 11/04/2026.
//

#ifndef RAMPACK_SHAPEMATCHERCOMMON_H
#define RAMPACK_SHAPEMATCHERCOMMON_H

#include "pyon/Matcher.h"


class ShapeMatcherCommon {
public:
    static pyon::matcher::MatcherArray create_vector_matcher();
    static pyon::matcher::MatcherArray create_axis_matcher();
    static pyon::matcher::MatcherDictionary create_named_points_matcher();
    static bool validate_axes(const pyon::matcher::DataclassData &dataclass);
};


#endif //RAMPACK_SHAPEMATCHERCOMMON_H
