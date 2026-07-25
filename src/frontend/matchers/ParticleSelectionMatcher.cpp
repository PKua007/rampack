//
// Created by Codex on 25/07/2026.
//

#include "ParticleSelectionMatcher.h"

#include <vector>

#include "core/ParticleSelection.h"
#include "frontend/matchers/generic/RangeMatcher.h"


using namespace pyon::matcher;

namespace {
    bool has_particle_mask(const Any &mask) {
        return mask.is<std::vector<std::size_t>>();
    }
}


const MatcherAlternative ParticleSelectionMatcher::particleMaskMatcher
    = RangeMatcher::create() | MatcherNone{};


bool ParticleSelectionMatcher::hasAtMostOneParticleMask(const DataclassData &data) {
    return !has_particle_mask(data["whitelist_shapes"]) || !has_particle_mask(data["blacklist_shapes"]);
}

void ParticleSelectionMatcher::apply(ParticleSelectable &selectable, const DataclassData &data) {
    Expects(ParticleSelectionMatcher::hasAtMostOneParticleMask(data));

    const auto &whitelist = data["whitelist_shapes"];
    const auto &blacklist = data["blacklist_shapes"];

    if (has_particle_mask(whitelist))
        selectable.setParticleSelection(ParticleSelection::whitelist(whitelist.as<std::vector<std::size_t>>()));
    else if (has_particle_mask(blacklist))
        selectable.setParticleSelection(ParticleSelection::blacklist(blacklist.as<std::vector<std::size_t>>()));
}
