//
// Created by Piotr Kubala on 16/12/2022.
//

#include "ShapeMatcher.h"

#include <map>
#include <optional>
#include <set>
#include <string>
#include <vector>

#include "core/shapes/SphereTraits.h"
#include "core/shapes/KMerTraits.h"
#include "core/shapes/PolysphereBananaTraits.h"
#include "core/shapes/PolysphereLollipopTraits.h"
#include "core/shapes/PolysphereWedgeTraits.h"
#include "core/shapes/SpherocylinderTraits.h"
#include "core/shapes/PolyspherocylinderBananaTraits.h"
#include "core/shapes/SmoothWedgeTraits.h"
#include "core/shapes/GenericXenoCollideTraits.h"

#include "geometry/xenocollide/XCBodyBuilder.h"
#include "core/shapes/PolyhedralWedgeTraits.h"

#include "GenericConvexGeometryMatcher.h"
#include "SoftInteractionMatcher.h"
#include "utils/Utils.h"


using namespace pyon::matcher;

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

    template <typename Traits, typename... Args>
    std::shared_ptr<ShapeTraits>
    create_traits_with_central_interaction(const std::shared_ptr<SoftInteractionFactory> &interactionFactory,
                                           const std::vector<std::string>& typeLabels, Args&&... args);

    bool validates_interaction_factory(const std::shared_ptr<SoftInteractionFactory> &interactionFactory,
                                      const std::vector<std::string> &typeLabels);
    bool validate_axes(const DataclassData &dataclass);
    std::vector<PolysphereCentreData> flatten_polysphere_entries(const std::vector<ParsedPolysphereEntry> &entries);
    bool polysphere_centres_have_consistent_type_radii(const std::vector<PolysphereCentreData> &centres);
    std::vector<std::string> extract_polysphere_type_labels(const std::vector<PolysphereCentreData> &centres);
    std::vector<std::string> build_polysphere_wedge_type_labels(std::size_t sphereN);

    MatcherDataclass create_sphere_matcher();
    MatcherDataclass create_kmer_matcher();
    MatcherDataclass create_polysphere_banana_matcher();
    MatcherDataclass create_polysphere_lollipop_matcher();
    MatcherDataclass create_polysphere_wedge_matcher();
    MatcherDataclass create_spherocylinder_matcher();
    MatcherDataclass create_polyspherocylinder_banana_matcher();
    MatcherDataclass create_smooth_wedge_matcher();
    MatcherDataclass create_polysphere_matcher();
    MatcherDataclass create_polyspherocylinder_matcher();
    MatcherDataclass create_generic_convex_matcher();
    MatcherDataclass create_polyhedral_wedge_matcher();


    const InteractionCentreTypeCoverage singleCentreTypeCoverage{{"s"}};

    auto centralInteraction = SoftInteractionMatcher::create();

    auto vector = MatcherArray(MatcherFloat{}.mapTo<double>(), 3).mapToVector<3>();

    auto axis = MatcherArray(MatcherFloat{}.mapTo<double>(), 3)
        .filter([](const ArrayData &data) {
            return std::any_of(data.begin(), data.end(), [](const Any &comp) {
                return std::abs(comp.as<double>()) > 1e-10;
            });
        })
        .describe("non-zero norm")
        .mapTo([](const ArrayData &data){
            return data.asVector<3>().normalized();
        });

    auto namedPoints = MatcherDictionary{}.valuesMatch(vector)
        .mapTo([](const DictionaryData &dict) {
            auto map = dict.asStdMap<Vector<3>>();
            ShapeGeometry::NamedPoints points(map.begin(), map.end());
            return points;
        });


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

    bool validates_interaction_factory(const std::shared_ptr<SoftInteractionFactory> &interactionFactory,
                                       const std::vector<std::string> &typeLabels)
    {
        const bool isHardInteraction = (interactionFactory == nullptr);
        if (isHardInteraction)
            return true;
        return interactionFactory->supportsTypes(typeLabels);
    }

    template <typename Traits, typename... Args>
    std::shared_ptr<ShapeTraits>
    create_traits_with_central_interaction(const std::shared_ptr<SoftInteractionFactory> &interactionFactory,
                                           const std::vector<std::string>& typeLabels, Args&&... args)
    {
        const bool isHardInteraction = (interactionFactory == nullptr);
        if (isHardInteraction)
            return std::make_shared<Traits>(std::forward<Args>(args)...);

        return std::make_shared<Traits>(std::forward<Args>(args)..., interactionFactory->createForTypes(typeLabels));
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

            const double previousRadius = it->second;
            const double newRadius = centre.radius;
            if (std::abs(newRadius - previousRadius) > 1e-10)
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

    std::vector<std::string> build_polysphere_wedge_type_labels(std::size_t sphereN) {
        std::vector<std::string> typeLabels;
        typeLabels.reserve(sphereN);
        for (std::size_t i{}; i < sphereN; i++)
            typeLabels.push_back("s" + std::to_string(i));

        return typeLabels;
    }

    MatcherDataclass create_sphere_matcher() {
        return MatcherDataclass("sphere")
            .arguments({{"r", MatcherFloat{}.positive()},
                        {"interaction", centralInteraction, "hard"}})
            .filter(singleCentreTypeCoverage.generateFilter())
            .describe(singleCentreTypeCoverage.generateDescription())
            .mapTo([typeLabels = singleCentreTypeCoverage.typeLabels](const DataclassData &sphere) -> std::shared_ptr<ShapeTraits> {
                auto r = sphere["r"].as<double>();
                auto interactionFactory = sphere["interaction"].as<std::shared_ptr<SoftInteractionFactory>>();
                return create_traits_with_central_interaction<SphereTraits>(
                    interactionFactory, typeLabels, r
                );
            });
    }

    MatcherDataclass create_kmer_matcher() {
        return MatcherDataclass("kmer")
            .arguments({{"k", MatcherInt{}.greaterEquals(2).mapTo<std::size_t>()},
                        {"r", MatcherFloat{}.positive()},
                        {"distance", MatcherFloat{}.positive()},
                        {"interaction", centralInteraction, "hard"}})
            .filter(singleCentreTypeCoverage.generateFilter())
            .describe(singleCentreTypeCoverage.generateDescription())
            .mapTo([typeLabels = singleCentreTypeCoverage.typeLabels](const DataclassData &kmer) -> std::shared_ptr<ShapeTraits> {
                auto k = kmer["k"].as<std::size_t>();
                auto r = kmer["r"].as<double>();
                auto distance = kmer["distance"].as<double>();
                auto interactionFactory = kmer["interaction"].as<std::shared_ptr<SoftInteractionFactory>>();
                return create_traits_with_central_interaction<KMerTraits>(
                    interactionFactory, typeLabels, k, r, distance
                );
            });
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
            .mapTo([typeLabels = singleCentreTypeCoverage.typeLabels](const DataclassData &banana) -> std::shared_ptr<ShapeTraits> {
                auto sphereN = banana["sphere_n"].as<std::size_t>();
                auto sphereR = banana["sphere_r"].as<double>();
                auto arcR = banana["arc_r"].as<double>();
                auto argAngle = banana["arc_angle"].as<double>();
                auto interactionFactory = banana["interaction"].as<std::shared_ptr<SoftInteractionFactory>>();
                return create_traits_with_central_interaction<PolysphereBananaTraits>(
                    interactionFactory, typeLabels, arcR, argAngle, sphereN, sphereR
                );
            });
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
            .mapTo([typeLabels = lollipopCentreTypeCoverage.typeLabels](const DataclassData &lollipop) -> std::shared_ptr<ShapeTraits> {
                auto sphereN = lollipop["sphere_n"].as<std::size_t>();
                auto stickR = lollipop["stick_r"].as<double>();
                auto tipR = lollipop["tip_r"].as<double>();
                auto stickPenetration = lollipop["stick_penetration"].as<double>();
                auto tipPenetration = lollipop["tip_penetration"].as<double>();
                auto interactionFactory = lollipop["interaction"].as<std::shared_ptr<SoftInteractionFactory>>();
                return create_traits_with_central_interaction<PolysphereLollipopTraits>(
                    interactionFactory, typeLabels, sphereN, stickR, tipR, stickPenetration, tipPenetration
                );
            });
    }

    MatcherDataclass create_polysphere_wedge_matcher() {
        return MatcherDataclass("polysphere_wedge")
            .arguments({{"sphere_n", MatcherInt{}.greaterEquals(2).mapTo<std::size_t>()},
                        {"bottom_r", MatcherFloat{}.positive()},
                        {"top_r", MatcherFloat{}.positive()},
                        {"penetration", MatcherFloat{}.nonNegative(), "0"},
                        {"interaction", centralInteraction, "hard"}})
            .filter([](const DataclassData &wedge) {
                auto sphereN = wedge["sphere_n"].as<std::size_t>();
                return validates_interaction_factory(
                    wedge["interaction"].as<std::shared_ptr<SoftInteractionFactory>>(),
                    build_polysphere_wedge_type_labels(sphereN)
                );
            })
            .describe(R"(interaction parameters must cover the implicit type labels: "s0", "s1", ..., "s[n-1]")")
            .filter([](const DataclassData &wedge) {
                double smallerR = std::min(wedge["bottom_r"].as<double>(), wedge["top_r"].as<double>());
                return wedge["penetration"].as<double>() < 2 * smallerR;
            })
            .describe("penetration < 2 * min(bottom_r, top_r)")
            .mapTo([](const DataclassData &wedge) -> std::shared_ptr<ShapeTraits> {
                auto sphereN = wedge["sphere_n"].as<std::size_t>();
                auto bottomR = wedge["bottom_r"].as<double>();
                auto topR = wedge["top_r"].as<double>();
                auto penetration = wedge["penetration"].as<double>();
                auto interactionFactory = wedge["interaction"].as<std::shared_ptr<SoftInteractionFactory>>();
                auto wedgeTypeLabels = build_polysphere_wedge_type_labels(sphereN);
                return create_traits_with_central_interaction<PolysphereWedgeTraits>(
                    interactionFactory, wedgeTypeLabels, sphereN, bottomR, topR, penetration
                );
            });
    }

    MatcherDataclass create_spherocylinder_matcher() {
        return MatcherDataclass("spherocylinder")
                .arguments({{"l", MatcherFloat{}.positive()},
                            {"r", MatcherFloat{}.positive()}})
                .mapTo([](const DataclassData &sc) -> std::shared_ptr<ShapeTraits> {
                    return std::make_shared<SpherocylinderTraits>(sc["l"].as<double>(), sc["r"].as<double>());
                });
    }

    MatcherDataclass create_polyspherocylinder_banana_matcher() {
        return MatcherDataclass("polyspherocylinder_banana")
            .arguments({{"segment_n", MatcherInt{}.greaterEquals(2).mapTo<std::size_t>()},
                        {"sc_r", MatcherFloat{}.positive()},
                        {"arc_r", MatcherFloat{}.positive()},
                        {"arc_angle", MatcherFloat{}.greaterEquals(0).less(M_PI)},
                        {"subdivisions", MatcherInt{}.positive().mapTo<std::size_t>(), "1"}})
            .filter([](const DataclassData &banana) {
                auto segmentN = banana["segment_n"].as<std::size_t>();
                auto spherocylinderR = banana["sc_r"].as<double>();
                auto arcR = banana["arc_r"].as<double>();
                auto argAngle = banana["arc_angle"].as<double>();
                return PolyspherocylinderBananaTraits::isArcOpen(arcR, argAngle, segmentN, spherocylinderR);
            })
            .describe("end cups must not overlap")
            .filter([](const DataclassData &banana) {
                auto segmentN = banana["segment_n"].as<std::size_t>();
                auto spherocylinderR = banana["sc_r"].as<double>();
                auto arcR = banana["arc_r"].as<double>();
                auto argAngle = banana["arc_angle"].as<double>();
                if (segmentN == 2)
                    return true;
                return PolyspherocylinderBananaTraits::isArcOriginOutside(arcR, argAngle, segmentN, spherocylinderR);
            })
            .describe("for segment_n >= 3, arc origin must lies outside of the shape")
            .mapTo([](const DataclassData &banana) -> std::shared_ptr<ShapeTraits> {
                auto segmentN = banana["segment_n"].as<std::size_t>();
                auto spherocylinderR = banana["sc_r"].as<double>();
                auto arcR = banana["arc_r"].as<double>();
                auto argAngle = banana["arc_angle"].as<double>();
                auto subdivisions = banana["subdivisions"].as<std::size_t>();
                return std::make_shared<PolyspherocylinderBananaTraits>(
                    arcR, argAngle, segmentN, spherocylinderR, subdivisions
                );
            });
    }

    MatcherDataclass create_smooth_wedge_matcher() {
        return MatcherDataclass("smooth_wedge")
            .arguments({{"l", MatcherFloat{}.positive()},
                        {"bottom_r", MatcherFloat{}.positive()},
                        {"top_r", MatcherFloat{}.positive()},
                        {"subdivisions", MatcherInt{}.positive().mapTo<std::size_t>(), "1"}})
            .filter([](const DataclassData &wedge){
                double rDiff = std::abs(wedge["bottom_r"].as<double>() - wedge["top_r"].as<double>());
                return wedge["l"].as<double>() >= rDiff;
            })
            .describe("l >= |bottom_r - top_r|")
            .mapTo([](const DataclassData &wedge) -> std::shared_ptr<ShapeTraits> {
                auto length = wedge["l"].as<double>();
                auto bottomR = wedge["bottom_r"].as<double>();
                auto topR = wedge["top_r"].as<double>();
                auto subdivisions = wedge["subdivisions"].as<std::size_t>();
                return std::make_shared<SmoothWedgeTraits>(bottomR, topR, length, subdivisions);
            });
    }

    MatcherDataclass create_polysphere_matcher() {
        auto singleSpherePos = vector.copy()
            .mapTo([](const ArrayData &array) {
                return std::vector<Vector<3>>{array.asVector<3>()};
            });
        auto multiSpherePos = MatcherArray{}
            .elementsMatch(vector)
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
                ParsedPolysphereEntry entry;
                entry.positions = sphere["pos"].as<std::vector<Vector<3>>>();
                entry.radius = sphere["r"].as<double>();
                entry.typeLabel = sphere["type"].as<std::optional<std::string>>();
                return entry;
            });
        auto singleSphere = sphere.copy()
            .mapTo([](const DataclassData &sphere) {
                ParsedPolysphereEntry entry;
                entry.positions = sphere["pos"].as<std::vector<Vector<3>>>();
                entry.radius = sphere["r"].as<double>();
                entry.typeLabel = sphere["type"].as<std::optional<std::string>>();
                return flatten_polysphere_entries({entry});
            });
        auto sphereArray = MatcherArray{}.elementsMatch(sphere)
            .nonEmpty()
            .mapTo([](const ArrayData &array) {
                return flatten_polysphere_entries(array.asStdVector<ParsedPolysphereEntry>());
            });

        return MatcherDataclass("polysphere")
            .arguments({{"spheres", singleSphere | sphereArray},
                        {"volume", MatcherFloat{}.positive()},
                        {"geometric_center", vector, "[0, 0, 0]"},
                        {"primary_axis", axis | MatcherNone{}, "None"},
                        {"secondary_axis", axis | MatcherNone{}, "None"},
                        {"named_points", namedPoints, "{}"},
                        {"interaction", centralInteraction, "hard"}})
            .filter([](const DataclassData &polysphere) {
                auto centres = polysphere["spheres"].as<std::vector<PolysphereCentreData>>();
                return polysphere_centres_have_consistent_type_radii(centres);
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
            .filter(validate_axes)
            .describe("primary_axis and secondary_axis must be orthogonal")
            .mapTo([](const DataclassData &polysphere) -> std::shared_ptr<ShapeTraits> {
                auto centres = polysphere["spheres"].as<std::vector<PolysphereCentreData>>();
                auto volume = polysphere["volume"].as<double>();
                auto geometricOrigin = polysphere["geometric_center"].as<Vector<3>>();
                std::optional<Vector<3>> primaryAxis;
                if (!polysphere["primary_axis"].isEmpty())
                    primaryAxis = polysphere["primary_axis"].as<Vector<3>>();
                std::optional<Vector<3>> secondaryAxis;
                if (!polysphere["secondary_axis"].isEmpty())
                    secondaryAxis = polysphere["secondary_axis"].as<Vector<3>>();
                auto namedPoints = polysphere["named_points"].as<ShapeGeometry::NamedPoints>();
                auto interactionFactory = polysphere["interaction"].as<std::shared_ptr<SoftInteractionFactory>>();

                PolysphereLayoutBuilder layoutBuilder;
                for (const auto &centre : centres)
                    layoutBuilder.consume(centre);

                auto layoutWithMetadata = layoutBuilder.releaseLayoutWithMetadata();
                PolysphereTraits::PolysphereGeometry geometry(
                    std::move(layoutWithMetadata), primaryAxis, secondaryAxis, geometricOrigin, volume, namedPoints
                );

                if (interactionFactory == nullptr) {
                    return std::make_shared<PolysphereTraits>(std::move(geometry));
                } else {
                    constexpr bool allowUniformPairDataBroadcast = true;
                    auto centralInteraction = interactionFactory->createForTypes(layoutBuilder.releaseTypeLabels());
                    return std::make_shared<PolysphereTraits>(
                        std::move(geometry), std::move(centralInteraction), allowUniformPairDataBroadcast
                    );
                }
            });
    }

    MatcherDataclass create_polyspherocylinder_matcher() {
        auto chain = MatcherArray()
            .elementsMatch(vector)
            .sizeAtLeast(2)
            .mapToStdVector<Vector<3>>();
        auto spherocylinder = MatcherDataclass("sc")
            .arguments({{"chain", chain},
                        {"r", MatcherFloat{}.positive()}})
            .mapTo([](const DataclassData &spherocylinder) {
                using SpherocylinderData = PolyspherocylinderTraits::SpherocylinderData;
                std::vector<SpherocylinderData> result;
                auto chain = spherocylinder["chain"].as<std::vector<Vector<3>>>();
                auto r = spherocylinder["r"].as<double>();
                for (std::size_t i{}; i < chain.size() - 1; i++) {
                    const Vector<3> &pos1 = chain[i];
                    const Vector<3> &pos2 = chain[i + 1];
                    Vector<3> origin = (pos1 + pos2)/2;
                    Vector<3> halfAxis = (pos2 - pos1)/2;
                    result.emplace_back(origin, halfAxis, r);
                }
                return result;
            });

        auto spherocylinderArray = MatcherArray()
            .elementsMatch(spherocylinder)
            .nonEmpty()
            .mapTo([](const ArrayData &array) {
                using SpherocylinderData = PolyspherocylinderTraits::SpherocylinderData;
                std::vector<SpherocylinderData> allScDatas;
                for (const auto &scData : array.asStdVector<std::vector<SpherocylinderData>>())
                    for (const auto &sphereData : scData)
                        allScDatas.push_back(sphereData);
                return allScDatas;
            });

        return MatcherDataclass("polyspherocylinder")
            .arguments({{"scs", spherocylinder | spherocylinderArray},
                        {"volume", MatcherFloat{}.positive()},
                        {"geometric_center", vector, "[0, 0, 0]"},
                        {"primary_axis", axis | MatcherNone{}, "None"},
                        {"secondary_axis", axis | MatcherNone{}, "None"},
                        {"named_points", namedPoints, "{}"}})
            .filter(validate_axes)
            .describe("primary_axis and secondary_axis must be orthogonal")
            .mapTo([](const DataclassData &polysc) -> std::shared_ptr<ShapeTraits> {
                using SpherocylinderData = PolyspherocylinderTraits::SpherocylinderData;
                auto sc = polysc["scs"].as<std::vector<SpherocylinderData>>();
                auto volume = polysc["volume"].as<double>();
                auto geometricOrigin = polysc["geometric_center"].as<Vector<3>>();
                std::optional<Vector<3>> primaryAxis;
                if (!polysc["primary_axis"].isEmpty())
                    primaryAxis = polysc["primary_axis"].as<Vector<3>>();
                std::optional<Vector<3>> secondaryAxis;
                if (!polysc["secondary_axis"].isEmpty())
                    secondaryAxis = polysc["secondary_axis"].as<Vector<3>>();
                auto namedPoints = polysc["named_points"].as<ShapeGeometry::NamedPoints>();

                PolyspherocylinderTraits::PolyspherocylinderGeometry geometry(
                    std::move(sc), primaryAxis, secondaryAxis, geometricOrigin, volume, namedPoints
                );

                return std::make_shared<PolyspherocylinderTraits>(std::move(geometry));
            });
    }

    MatcherDataclass create_generic_convex_matcher() {
        return MatcherDataclass("generic_convex")
            .arguments({{"geometry", GenericConvexGeometryMatcher::script},
                        {"volume", MatcherFloat{}.positive()},
                        {"geometric_center", vector, "[0, 0, 0]"},
                        {"primary_axis", axis | MatcherNone{}, "None"},
                        {"secondary_axis", axis | MatcherNone{}, "None"},
                        {"named_points", namedPoints, "{}"}})
            .filter(validate_axes)
            .describe("primary_axis and secondary_axis must be orthogonal")
            .mapTo([](const DataclassData &convex) -> std::shared_ptr<ShapeTraits> {
                auto script = convex["geometry"].as<XCBodyBuilderScript>();
                auto volume = convex["volume"].as<double>();
                auto geometricOrigin = convex["geometric_center"].as<Vector<3>>();
                std::optional<Vector<3>> primaryAxis;
                if (!convex["primary_axis"].isEmpty())
                    primaryAxis = convex["primary_axis"].as<Vector<3>>();
                std::optional<Vector<3>> secondaryAxis;
                if (!convex["secondary_axis"].isEmpty())
                    secondaryAxis = convex["secondary_axis"].as<Vector<3>>();
                auto namedPoints = convex["named_points"].as<ShapeGeometry::NamedPoints>();

                XCBodyBuilder builder;
                script(builder);
                auto geometry = builder.releaseCollideGeometry();

                return std::make_shared<GenericXenoCollideTraits>(
                    geometry, primaryAxis, secondaryAxis, geometricOrigin, volume, namedPoints
                );
            });
    }

    MatcherDataclass create_polyhedral_wedge_matcher() {
        return MatcherDataclass("polyhedral_wedge")
            .arguments({{"bottom_ax", MatcherFloat{}.nonNegative()},
                        {"bottom_ay", MatcherFloat{}.nonNegative()},
                        {"top_ax", MatcherFloat{}.nonNegative()},
                        {"top_ay", MatcherFloat{}.nonNegative()},
                        {"l", MatcherFloat{}.positive()},
                        {"subdivisions", MatcherInt{}.positive().mapTo<std::size_t>(), "1"}})
            .filter([](const DataclassData &wedge) {
                auto bottomRx = wedge["bottom_ax"].as<double>();
                auto bottomRy = wedge["bottom_ay"].as<double>();
                auto topRx = wedge["top_ax"].as<double>();
                auto topRy = wedge["top_ay"].as<double>();
                return (topRx != 0 && bottomRy != 0) || (topRy != 0 && bottomRx != 0);
            })
            .describe("with at least one pair of non-zero orthogonal a-s, one at the top, one at the bottom")
            .mapTo([](const DataclassData &wedge) -> std::shared_ptr<ShapeTraits> {
                return std::make_shared<PolyhedralWedgeTraits>(
                    wedge["bottom_ax"].as<double>(),
                    wedge["bottom_ay"].as<double>(),
                    wedge["top_ax"].as<double>(),
                    wedge["top_ay"].as<double>(),
                    wedge["l"].as<double>(),
                    wedge["subdivisions"].as<std::size_t>()
                );
            });
    }

    bool validate_axes(const DataclassData &dataclass) {
        if (dataclass["primary_axis"].isEmpty()) {
            return dataclass["secondary_axis"].isEmpty();
        } else if (!dataclass["secondary_axis"].isEmpty()) {
            auto pa = dataclass["primary_axis"].as<Vector<3>>();
            auto sa = dataclass["secondary_axis"].as<Vector<3>>();
            return pa * sa < 1e-12;
        } else {
            return true;
        }
    }
}


std::shared_ptr<ShapeTraits> ShapeMatcher::match(const std::string &expression) {
    Any shapeTraits;
    auto shapeAST = pyon::Parser::parse(expression);
    auto shapeMatcher = ShapeMatcher::create();
    auto matchReport = shapeMatcher.match(shapeAST, shapeTraits);
    if (!matchReport)
        throw ValidationException(matchReport.getReason());

    return shapeTraits.as<std::shared_ptr<ShapeTraits>>();
}

pyon::matcher::MatcherAlternative ShapeMatcher::create() {
    return create_sphere_matcher()
        | create_kmer_matcher()
        | create_polysphere_banana_matcher()
        | create_polysphere_lollipop_matcher()
        | create_polysphere_wedge_matcher()
        | create_spherocylinder_matcher()
        | create_polyspherocylinder_banana_matcher()
        | create_smooth_wedge_matcher()
        | create_polysphere_matcher()
        | create_polyspherocylinder_matcher()
        | create_generic_convex_matcher()
        | create_polyhedral_wedge_matcher();
}
