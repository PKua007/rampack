//
// Created by Codex on 25/07/2026.
//

#ifndef RAMPACK_PARTICLESELECTIONMATCHER_H
#define RAMPACK_PARTICLESELECTIONMATCHER_H

#include <memory>
#include <type_traits>

#include "core/ParticleSelectable.h"
#include "pyon/Matcher.h"


class ParticleSelectionMatcher {
private:
    static void apply(ParticleSelectable &selectable, const pyon::matcher::DataclassData &data);

public:
    static const pyon::matcher::MatcherAlternative particleMaskMatcher;

    static constexpr const char *mutualExclusionDescription
        = "whitelist_shapes and blacklist_shapes cannot be specified together";

    static bool hasAtMostOneParticleMask(const pyon::matcher::DataclassData &data);

    template<typename T>
    static std::shared_ptr<T> apply(std::shared_ptr<T> selectable, const pyon::matcher::DataclassData &data) {
        static_assert(std::is_base_of_v<ParticleSelectable, T>,
                      "ParticleSelectionMatcher can only be applied to ParticleSelectable objects");
        apply(*selectable, data);
        return selectable;
    }
};


#endif //RAMPACK_PARTICLESELECTIONMATCHER_H
