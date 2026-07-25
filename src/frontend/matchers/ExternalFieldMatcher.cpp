//
// Created by Codex on 25/07/2026.
//

#include "ExternalFieldMatcher.h"

#include <memory>
#include <optional>
#include <utility>

#include "core/ExternalField.h"
#include "core/external_fields/GravityField.h"
#include "ParticleSelectionMatcher.h"


using namespace pyon::matcher;


namespace {
    constexpr double DIRECTION_NORM_EPSILON = 1e-12;

    MatcherDataclass create_gravity() {
        const auto direction = MatcherArray(MatcherFloat{}, 3)
            .filter([](const ArrayData &directionData) {
                return directionData.asVector<3>().norm2() > DIRECTION_NORM_EPSILON * DIRECTION_NORM_EPSILON;
            })
            .describe("with non-zero norm")
            .mapToVector<3>();

        const auto pointName = MatcherString{}
            .nonEmpty()
            .mapTo([](const std::string &point) -> std::optional<std::string> { return point; });
        const auto pointNone = MatcherNone{}.mapTo<std::optional<std::string>>();
        const auto point = pointName | pointNone;

        const auto boxAnchor = MatcherArray(MatcherFloat{}.inRange(0, 1), 3).mapToVector<3>();

        return MatcherDataclass("gravity")
            .arguments({{"g", MatcherFloat{}.positive()},
                        {"direction_hkl", direction},
                        {"point", point, "None"},
                        {"box_anchor", boxAnchor, "[0, 0, 0]"},
                        {"whitelist_shapes", ParticleSelectionMatcher::particleMaskMatcher, "None"},
                        {"blacklist_shapes", ParticleSelectionMatcher::particleMaskMatcher, "None"}})
            .filter(ParticleSelectionMatcher::hasAtMostOneParticleMask)
            .describe(ParticleSelectionMatcher::mutualExclusionDescription)
            .mapTo([](const DataclassData &gravity) -> std::shared_ptr<ExternalField> {
                const auto g = gravity["g"].as<double>();
                const auto directionHkl = gravity["direction_hkl"].as<Vector<3>>();
                const auto point = gravity["point"].as<std::optional<std::string>>();
                const auto boxAnchor = gravity["box_anchor"].as<Vector<3>>();
                auto field = std::make_shared<GravityField>(g, directionHkl, point, boxAnchor);
                return ParticleSelectionMatcher::apply(std::move(field), gravity);
            });
    }
}


MatcherAlternative ExternalFieldMatcher::create() {
    MatcherAlternative externalFieldMatcher;
    externalFieldMatcher |= create_gravity();
    return externalFieldMatcher;
}
