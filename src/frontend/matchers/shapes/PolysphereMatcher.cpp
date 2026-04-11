//
// Created by Codex on 11/04/2026.
//

#include "PolysphereMatcher.h"

#include <map>
#include <optional>
#include <set>
#include <string>
#include <vector>

#include "ShapeMatcherCommon.h"
#include "SoftInteractionMatcher.h"
#include "core/shapes/KMerTraits.h"
#include "core/shapes/PolysphereBananaTraits.h"
#include "core/shapes/PolysphereLollipopTraits.h"
#include "core/shapes/PolysphereWedgeTraits.h"
#include "core/shapes/SphereTraits.h"
#include "utils/Utils.h"


using namespace pyon::matcher;

// TYPES
namespace {
    struct ParsedPolysphereEntry {
        std::vector<Vector<3>> positions;
        double radius{};
        std::optional<std::string> typeLabel;
    };

    struct PolysphereCentreData {
        Vector<3> position;
        double radius{};
        std::string typeLabel;
    };

    struct InteractionCentreTypeCoverage {
        std::vector<std::string> typeLabels;

        [[nodiscard]] std::function<bool(const DataclassData &)> generateFilter() const;
        [[nodiscard]] std::string generateDescription() const;
    };

    class PolysphereLayoutBuilder {
    private:
        std::vector<Vector<3>> centrePositions;
        std::vector<std::size_t> centreIdxTypeIdxMap;
        std::vector<PolysphereTraits::InteractionCentreTypeMetadata> typeMetadata;
        std::vector<std::string> typeLabels;
        std::map<std::string, std::size_t> typeLabelToIdx;

        std::size_t fetch_or_create_type_idx(const PolysphereCentreData &centre);

    public:
        void consume(const PolysphereCentreData &centre);
        [[nodiscard]] PolysphereTraits::InteractionCentreLayoutWithMetadata releaseLayoutWithMetadata();
        [[nodiscard]] std::vector<std::string> releaseTypeLabels();
    };
}

// GLOBAL VARIABLES
namespace {
    const InteractionCentreTypeCoverage singleCentreTypeCoverage{{"s"}};
    const auto centralInteraction = SoftInteractionMatcher::create();
}

// FORWARD DECLARATIONS
namespace {
    template <typename Traits, typename... Args>
    std::shared_ptr<ShapeTraits>
    create_traits_with_central_interaction(const std::shared_ptr<SoftInteractionFactory> &interactionFactory,
                                           const std::vector<std::string> &typeLabels, Args&&... args);

    bool validates_interaction_factory(const std::shared_ptr<SoftInteractionFactory> &interactionFactory,
                                       const std::vector<std::string> &typeLabels);
    std::vector<PolysphereCentreData> flatten_polysphere_entries(const std::vector<ParsedPolysphereEntry> &entries);
    bool polysphere_centres_have_consistent_type_radii(const std::vector<PolysphereCentreData> &centres);
    std::vector<std::string> extract_polysphere_type_labels(const std::vector<PolysphereCentreData> &centres);
    std::vector<std::string> build_polysphere_wedge_type_labels(std::size_t sphereN);
    MatcherAlternative create_polysphere_spheres_matcher();

    PolysphereTraits::PolysphereGeometry
    create_polysphere_geometry(const DataclassData &polysphere,
                               PolysphereTraits::InteractionCentreLayoutWithMetadata layoutWithMetadata);

    MatcherDataclass create_sphere_matcher();
    MatcherDataclass create_kmer_matcher();
    MatcherDataclass create_polysphere_banana_matcher();
    MatcherDataclass create_polysphere_lollipop_matcher();
    MatcherDataclass create_polysphere_wedge_matcher();
    MatcherDataclass create_polysphere_matcher();
}

// DEFINITIONS
namespace {
    std::function<bool(const DataclassData &)> InteractionCentreTypeCoverage::generateFilter() const {
        return [typeLabels = this->typeLabels](const DataclassData &shape) {
            return validates_interaction_factory(
                shape["interaction"].as<std::shared_ptr<SoftInteractionFactory>>(), typeLabels
            );
        };
    }

    std::string InteractionCentreTypeCoverage::generateDescription() const {
        auto quotedTypeLabels = this->typeLabels;
        for (auto &typeLabel : quotedTypeLabels)
            typeLabel = "\"" + typeLabel + "\"";
        return "interaction parameters must cover the implicit type labels: " + implode(quotedTypeLabels);
    }

    std::size_t PolysphereLayoutBuilder::fetch_or_create_type_idx(const PolysphereCentreData &centre) {
        auto typeIt = this->typeLabelToIdx.find(centre.typeLabel);
        if (typeIt != this->typeLabelToIdx.end()) {
            const std::size_t typeIdx = typeIt->second;
            Assert(std::abs(this->typeMetadata[typeIdx].radius - centre.radius) <= 1e-10);
            return typeIdx;
        }

        const std::size_t typeIdx = this->typeMetadata.size();
        this->typeLabelToIdx.emplace(centre.typeLabel, typeIdx);
        this->typeLabels.push_back(centre.typeLabel);
        this->typeMetadata.emplace_back(centre.radius);
        return typeIdx;
    }

    void PolysphereLayoutBuilder::consume(const PolysphereCentreData &centre) {
        const std::size_t typeIdx = this->fetch_or_create_type_idx(centre);
        this->centrePositions.push_back(centre.position);
        this->centreIdxTypeIdxMap.push_back(typeIdx);
    }

    PolysphereTraits::InteractionCentreLayoutWithMetadata PolysphereLayoutBuilder::releaseLayoutWithMetadata() {
        return {
            InteractionCentreLayout(std::move(this->centrePositions), std::move(this->centreIdxTypeIdxMap)),
            std::move(this->typeMetadata)
        };
    }

    std::vector<std::string> PolysphereLayoutBuilder::releaseTypeLabels() {
        return std::move(this->typeLabels);
    }

    template <typename Traits, typename... Args>
    std::shared_ptr<ShapeTraits>
    create_traits_with_central_interaction(const std::shared_ptr<SoftInteractionFactory> &interactionFactory,
                                           const std::vector<std::string> &typeLabels, Args&&... args)
    {
        if (interactionFactory == nullptr)
            return std::make_shared<Traits>(std::forward<Args>(args)...);

        return std::make_shared<Traits>(std::forward<Args>(args)..., interactionFactory->createForTypes(typeLabels));
    }

    bool validates_interaction_factory(const std::shared_ptr<SoftInteractionFactory> &interactionFactory,
                                       const std::vector<std::string> &typeLabels)
    {
        if (interactionFactory == nullptr)
            return true;
        return interactionFactory->supportsTypes(typeLabels);
    }

    std::vector<PolysphereCentreData> flatten_polysphere_entries(const std::vector<ParsedPolysphereEntry> &entries) {
        std::vector<PolysphereCentreData> centres;
        for (std::size_t entryIdx{}; entryIdx < entries.size(); entryIdx++) {
            const auto &entry = entries[entryIdx];
            const std::string typeLabel = entry.typeLabel.value_or(std::to_string(entryIdx));
            for (const auto &position : entry.positions)
                centres.push_back({position, entry.radius, typeLabel});
        }

        return centres;
    }

    bool polysphere_centres_have_consistent_type_radii(const std::vector<PolysphereCentreData> &centres) {
        std::map<std::string, double> radiusByType;
        for (const auto &centre : centres) {
            auto [it, typeNotSeen] = radiusByType.emplace(centre.typeLabel, centre.radius);
            if (typeNotSeen)
                continue;

            if (std::abs(centre.radius - it->second) > 1e-10)
                return false;
        }

        return true;
    }

    std::vector<std::string> extract_polysphere_type_labels(const std::vector<PolysphereCentreData> &centres) {
        std::vector<std::string> typeLabels;
        std::set<std::string> seenLabels;
        for (const auto &centre : centres) {
            [[maybe_unused]] auto [it, inserted] = seenLabels.emplace(centre.typeLabel);
            if (inserted)
                typeLabels.push_back(centre.typeLabel);
        }

        return typeLabels;
    }

    std::vector<std::string> build_polysphere_wedge_type_labels(std::size_t sphereN) {
        std::vector<std::string> typeLabels;
        typeLabels.reserve(sphereN);
        for (std::size_t i{}; i < sphereN; i++)
            typeLabels.push_back("s" + std::to_string(i));

        return typeLabels;
    }

    MatcherAlternative create_polysphere_spheres_matcher() {
        auto vector = ShapeMatcherCommon::create_vector_matcher();
        auto singleSpherePos = vector.copy()
            .mapTo([](const ArrayData &array) {
                return std::vector<Vector<3>>{array.asVector<3>()};
            });
        auto multiSpherePos = MatcherArray{}
            .elementsMatch(ShapeMatcherCommon::create_vector_matcher())
            .nonEmpty()
            .mapToStdVector<Vector<3>>();
        auto spherePos = singleSpherePos | multiSpherePos;

        auto centreTypeImplicit = MatcherNone{}.mapTo<std::optional<std::string>>();
        auto centreTypeExplicit = MatcherString{}.nonEmpty().mapTo<std::optional<std::string>>();
        auto sphere = MatcherDataclass("sphere")
            .arguments({{"pos", spherePos},
                        {"r", MatcherFloat{}.positive()},
                        {"type", centreTypeImplicit | centreTypeExplicit, "None"}})
            .mapTo([](const DataclassData &sphere) -> ParsedPolysphereEntry {
                return {
                    sphere["pos"].as<std::vector<Vector<3>>>(),
                    sphere["r"].as<double>(),
                    sphere["type"].as<std::optional<std::string>>()
                };
            });
        auto singleSphere = sphere.copy()
            .mapTo([](const DataclassData &sphereDataclass) {
                ParsedPolysphereEntry entry{
                    sphereDataclass["pos"].as<std::vector<Vector<3>>>(),
                    sphereDataclass["r"].as<double>(),
                    sphereDataclass["type"].as<std::optional<std::string>>()
                };
                return flatten_polysphere_entries({entry});
            });
        auto sphereArray = MatcherArray{}.elementsMatch(sphere)
            .nonEmpty()
            .mapTo([](const ArrayData &array) {
                return flatten_polysphere_entries(array.asStdVector<ParsedPolysphereEntry>());
            });

        return singleSphere | sphereArray;
    }

    PolysphereTraits::PolysphereGeometry
    create_polysphere_geometry(const DataclassData &polysphere,
                               PolysphereTraits::InteractionCentreLayoutWithMetadata layoutWithMetadata)
    {
        std::optional<Vector<3>> primaryAxis;
        if (!polysphere["primary_axis"].isEmpty())
            primaryAxis = polysphere["primary_axis"].as<Vector<3>>();
        std::optional<Vector<3>> secondaryAxis;
        if (!polysphere["secondary_axis"].isEmpty())
            secondaryAxis = polysphere["secondary_axis"].as<Vector<3>>();
        auto geometricCenter = polysphere["geometric_center"].as<Vector<3>>();
        auto volume = polysphere["volume"].as<double>();
        auto namedPoints = polysphere["named_points"].as<ShapeGeometry::NamedPoints>();

        return PolysphereTraits::PolysphereGeometry(
            std::move(layoutWithMetadata), primaryAxis, secondaryAxis, geometricCenter, volume, namedPoints
        );
    }

    MatcherDataclass create_sphere_matcher() {
        return MatcherDataclass("sphere")
            .arguments({{"r", MatcherFloat{}.positive()},
                        {"interaction", centralInteraction, "hard"}})
            .filter(singleCentreTypeCoverage.generateFilter())
            .describe(singleCentreTypeCoverage.generateDescription())
            .mapTo(
                [typeLabels = singleCentreTypeCoverage.typeLabels]
                (const DataclassData &sphere) -> std::shared_ptr<ShapeTraits> {
                    return create_traits_with_central_interaction<SphereTraits>(
                        sphere["interaction"].as<std::shared_ptr<SoftInteractionFactory>>(), typeLabels,
                        sphere["r"].as<double>()
                    );
                }
            );
    }

    MatcherDataclass create_kmer_matcher() {
        return MatcherDataclass("kmer")
            .arguments({{"k", MatcherInt{}.greaterEquals(2).mapTo<std::size_t>()},
                        {"r", MatcherFloat{}.positive()},
                        {"distance", MatcherFloat{}.positive()},
                        {"interaction", centralInteraction, "hard"}})
            .filter(singleCentreTypeCoverage.generateFilter())
            .describe(singleCentreTypeCoverage.generateDescription())
            .mapTo(
                [typeLabels = singleCentreTypeCoverage.typeLabels]
                (const DataclassData &kmer) -> std::shared_ptr<ShapeTraits> {
                    return create_traits_with_central_interaction<KMerTraits>(
                        kmer["interaction"].as<std::shared_ptr<SoftInteractionFactory>>(), typeLabels,
                        kmer["k"].as<std::size_t>(), kmer["r"].as<double>(), kmer["distance"].as<double>()
                    );
                }
            );
    }

    MatcherDataclass create_polysphere_banana_matcher() {
        return MatcherDataclass("polysphere_banana")
            .arguments({{"sphere_n", MatcherInt{}.greaterEquals(2).mapTo<std::size_t>()},
                        {"sphere_r", MatcherFloat{}.positive()},
                        {"arc_r", MatcherFloat{}.positive()},
                        {"arc_angle", MatcherFloat{}.greaterEquals(0).less(2*M_PI)},
                        {"interaction", centralInteraction, "hard"}})
            .filter(singleCentreTypeCoverage.generateFilter())
            .describe(singleCentreTypeCoverage.generateDescription())
            .mapTo(
                [typeLabels = singleCentreTypeCoverage.typeLabels]
                (const DataclassData &banana) -> std::shared_ptr<ShapeTraits> {
                    return create_traits_with_central_interaction<PolysphereBananaTraits>(
                        banana["interaction"].as<std::shared_ptr<SoftInteractionFactory>>(), typeLabels,
                        banana["arc_r"].as<double>(), banana["arc_angle"].as<double>(),
                        banana["sphere_n"].as<std::size_t>(), banana["sphere_r"].as<double>()
                    );
                }
            );
    }

    MatcherDataclass create_polysphere_lollipop_matcher() {
        const InteractionCentreTypeCoverage lollipopCentreTypeCoverage{{"ss", "st"}};
        return MatcherDataclass("polysphere_lollipop")
            .arguments({{"sphere_n", MatcherInt{}.greaterEquals(2).mapTo<std::size_t>()},
                        {"stick_r", MatcherFloat{}.positive()},
                        {"tip_r", MatcherFloat{}.positive()},
                        {"stick_penetration", MatcherFloat{}.nonNegative(), "0"},
                        {"tip_penetration", MatcherFloat{}.nonNegative(), "0"},
                        {"interaction", centralInteraction, "hard"}})
            .filter([](const DataclassData &lollipop) {
                return lollipop["stick_penetration"].as<double>() < 2 * lollipop["stick_r"].as<double>();
            })
            .describe("stick_penetration < 2 * stick_r")
            .filter([](const DataclassData &lollipop) {
                double smallerR = std::min(lollipop["stick_r"].as<double>(), lollipop["tip_r"].as<double>());
                return lollipop["tip_penetration"].as<double>() < 2 * smallerR;
            })
            .describe("tip_penetration < 2 * min(stick_r, tip_r)")
            .filter(lollipopCentreTypeCoverage.generateFilter())
            .describe(lollipopCentreTypeCoverage.generateDescription())
            .mapTo(
                [typeLabels = lollipopCentreTypeCoverage.typeLabels]
                (const DataclassData &lollipop) -> std::shared_ptr<ShapeTraits> {
                    return create_traits_with_central_interaction<PolysphereLollipopTraits>(
                        lollipop["interaction"].as<std::shared_ptr<SoftInteractionFactory>>(), typeLabels,
                        lollipop["sphere_n"].as<std::size_t>(), lollipop["stick_r"].as<double>(),
                        lollipop["tip_r"].as<double>(), lollipop["stick_penetration"].as<double>(),
                        lollipop["tip_penetration"].as<double>()
                    );
                }
            );
    }

    MatcherDataclass create_polysphere_wedge_matcher() {
        return MatcherDataclass("polysphere_wedge")
            .arguments({{"sphere_n", MatcherInt{}.greaterEquals(2).mapTo<std::size_t>()},
                        {"bottom_r", MatcherFloat{}.positive()},
                        {"top_r", MatcherFloat{}.positive()},
                        {"penetration", MatcherFloat{}.nonNegative(), "0"},
                        {"interaction", centralInteraction, "hard"}})
            .filter([](const DataclassData &wedge) {
                return validates_interaction_factory(
                    wedge["interaction"].as<std::shared_ptr<SoftInteractionFactory>>(),
                    build_polysphere_wedge_type_labels(wedge["sphere_n"].as<std::size_t>())
                );
            })
            .describe(R"(interaction parameters must cover the implicit type labels: "s0", "s1", ..., "s[n-1]")")
            .filter([](const DataclassData &wedge) {
                double smallerR = std::min(wedge["bottom_r"].as<double>(), wedge["top_r"].as<double>());
                return wedge["penetration"].as<double>() < 2 * smallerR;
            })
            .describe("penetration < 2 * min(bottom_r, top_r)")
            .mapTo([](const DataclassData &wedge) -> std::shared_ptr<ShapeTraits> {
                auto wedgeTypeLabels = build_polysphere_wedge_type_labels(wedge["sphere_n"].as<std::size_t>());
                return create_traits_with_central_interaction<PolysphereWedgeTraits>(
                    wedge["interaction"].as<std::shared_ptr<SoftInteractionFactory>>(), wedgeTypeLabels,
                    wedge["sphere_n"].as<std::size_t>(), wedge["bottom_r"].as<double>(),
                    wedge["top_r"].as<double>(), wedge["penetration"].as<double>()
                );
            });
    }

    MatcherDataclass create_polysphere_matcher() {
        return MatcherDataclass("polysphere")
            .arguments({{"spheres", create_polysphere_spheres_matcher()},
                        {"volume", MatcherFloat{}.positive()},
                        {"geometric_center", ShapeMatcherCommon::create_vector_matcher(), "[0, 0, 0]"},
                        {"primary_axis", ShapeMatcherCommon::create_axis_matcher() | MatcherNone{}, "None"},
                        {"secondary_axis", ShapeMatcherCommon::create_axis_matcher() | MatcherNone{}, "None"},
                        {"named_points", ShapeMatcherCommon::create_named_points_matcher(), "{}"},
                        {"interaction", centralInteraction, "hard"}})
            .filter([](const DataclassData &polysphere) {
                return polysphere_centres_have_consistent_type_radii(
                    polysphere["spheres"].as<std::vector<PolysphereCentreData>>()
                );
            })
            .describe("all polysphere spheres sharing one type label must have the same radius")
            .filter([](const DataclassData &polysphere) {
                auto centres = polysphere["spheres"].as<std::vector<PolysphereCentreData>>();
                return validates_interaction_factory(
                    polysphere["interaction"].as<std::shared_ptr<SoftInteractionFactory>>(),
                    extract_polysphere_type_labels(centres)
                );
            })
            .describe("interaction parameters must cover exactly the explicit or implicit polysphere type labels")
            .filter(ShapeMatcherCommon::validate_axes)
            .describe("primary_axis and secondary_axis must be orthogonal")
            .mapTo([](const DataclassData &polysphere) -> std::shared_ptr<ShapeTraits> {
                auto centres = polysphere["spheres"].as<std::vector<PolysphereCentreData>>();
                PolysphereLayoutBuilder layoutBuilder;
                for (const auto &centre : centres)
                    layoutBuilder.consume(centre);

                auto geometry = create_polysphere_geometry(polysphere, layoutBuilder.releaseLayoutWithMetadata());

                auto interactionFactory = polysphere["interaction"].as<std::shared_ptr<SoftInteractionFactory>>();
                if (interactionFactory == nullptr)
                    return std::make_shared<PolysphereTraits>(std::move(geometry));

                constexpr bool allowUniformPairDataBroadcast = true;
                auto centralInteraction = interactionFactory->createForTypes(layoutBuilder.releaseTypeLabels());
                return std::make_shared<PolysphereTraits>(
                    std::move(geometry), std::move(centralInteraction), allowUniformPairDataBroadcast
                );
            });
    }
}


MatcherAlternative PolysphereMatcher::create() {
    return create_sphere_matcher()
        | create_kmer_matcher()
        | create_polysphere_banana_matcher()
        | create_polysphere_lollipop_matcher()
        | create_polysphere_wedge_matcher()
        | create_polysphere_matcher();
}
