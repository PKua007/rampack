//
// Created by Piotr Kubala on 23/01/2023.
//

#ifndef RAMPACK_FILESNAPSHOTWRITERMATCHER_H
#define RAMPACK_FILESNAPSHOTWRITERMATCHER_H

#include "pyon/Matcher.h"


class FileSnapshotWriterMatcher {
public:
    static pyon::matcher::MatcherAlternative create();
};


#endif //RAMPACK_FILESNAPSHOTWRITERMATCHER_H
