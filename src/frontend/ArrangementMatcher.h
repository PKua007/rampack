//
// Created by Piotr Kubala on 03/01/2023.
//

#ifndef RAMPACK_ARRANGEMENTMATCHER_H
#define RAMPACK_ARRANGEMENTMATCHER_H

#include "pyon/Matcher.h"
#include "core/Packing.h"
#include "core/ShapeTraits.h"


class ArrangementMatcher {
public:
    class PackingFactory {
    public:
        virtual ~PackingFactory() = default;
        virtual std::unique_ptr<Packing> createPacking(std::unique_ptr<BoundaryConditions> bc,
                                                       const Interaction &interaction, std::size_t moveThreads,
                                                       std::size_t scalingThreads) = 0;
    };

    static pyon::matcher::MatcherAlternative create(const ShapeTraits &traits);
};


#endif //RAMPACK_ARRANGEMENTMATCHER_H
