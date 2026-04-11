//
// Created by Codex on 11/04/2026.
//

#include "PolyspherocylinderMatcher.h"

#include <optional>
#include <vector>

#include "ShapeMatcherCommon.h"
#include "core/shapes/PolyspherocylinderBananaTraits.h"
#include "core/shapes/PolyspherocylinderTraits.h"
#include "core/shapes/SpherocylinderTraits.h"


using namespace pyon::matcher;

namespace {
    MatcherAlternative create_polyspherocylinder_scs_matcher() {
        auto chain = MatcherArray()
            .elementsMatch(ShapeMatcherCommon::create_vector_matcher())
            .sizeAtLeast(2)
            .mapToStdVector<Vector<3>>();
        auto spherocylinder = MatcherDataclass("sc")
            .arguments({{"chain", chain},
                        {"r", MatcherFloat{}.positive()}})
            .mapTo([](const DataclassData &spherocylinder) {
                using SpherocylinderData = PolyspherocylinderTraits::SpherocylinderData;
                std::vector<SpherocylinderData> result;
                auto chainPoints = spherocylinder["chain"].as<std::vector<Vector<3>>>();
                auto radius = spherocylinder["r"].as<double>();
                for (std::size_t i{}; i < chainPoints.size() - 1; i++) {
                    Vector<3> origin = (chainPoints[i] + chainPoints[i + 1])/2;
                    Vector<3> halfAxis = (chainPoints[i + 1] - chainPoints[i])/2;
                    result.emplace_back(origin, halfAxis, radius);
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
                    for (const auto &segmentData : scData)
                        allScDatas.push_back(segmentData);
                return allScDatas;
            });

        return spherocylinder | spherocylinderArray;
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
                return PolyspherocylinderBananaTraits::isArcOpen(
                    banana["arc_r"].as<double>(), banana["arc_angle"].as<double>(),
                    banana["segment_n"].as<std::size_t>(), banana["sc_r"].as<double>()
                );
            })
            .describe("end cups must not overlap")
            .filter([](const DataclassData &banana) {
                if (banana["segment_n"].as<std::size_t>() == 2)
                    return true;
                return PolyspherocylinderBananaTraits::isArcOriginOutside(
                    banana["arc_r"].as<double>(), banana["arc_angle"].as<double>(),
                    banana["segment_n"].as<std::size_t>(), banana["sc_r"].as<double>()
                );
            })
            .describe("for segment_n >= 3, arc origin must lies outside of the shape")
            .mapTo([](const DataclassData &banana) -> std::shared_ptr<ShapeTraits> {
                return std::make_shared<PolyspherocylinderBananaTraits>(
                    banana["arc_r"].as<double>(), banana["arc_angle"].as<double>(),
                    banana["segment_n"].as<std::size_t>(), banana["sc_r"].as<double>(),
                    banana["subdivisions"].as<std::size_t>()
                );
            });
    }

    MatcherDataclass create_polyspherocylinder_matcher() {
        return MatcherDataclass("polyspherocylinder")
            .arguments({{"scs", create_polyspherocylinder_scs_matcher()},
                        {"volume", MatcherFloat{}.positive()},
                        {"geometric_center", ShapeMatcherCommon::create_vector_matcher(), "[0, 0, 0]"},
                        {"primary_axis", ShapeMatcherCommon::create_axis_matcher() | MatcherNone{}, "None"},
                        {"secondary_axis", ShapeMatcherCommon::create_axis_matcher() | MatcherNone{}, "None"},
                        {"named_points", ShapeMatcherCommon::create_named_points_matcher(), "{}"}})
            .filter(ShapeMatcherCommon::validate_axes)
            .describe("primary_axis and secondary_axis must be orthogonal")
            .mapTo([](const DataclassData &polysc) -> std::shared_ptr<ShapeTraits> {
                using SpherocylinderData = PolyspherocylinderTraits::SpherocylinderData;
                std::optional<Vector<3>> primaryAxis;
                if (!polysc["primary_axis"].isEmpty())
                    primaryAxis = polysc["primary_axis"].as<Vector<3>>();
                std::optional<Vector<3>> secondaryAxis;
                if (!polysc["secondary_axis"].isEmpty())
                    secondaryAxis = polysc["secondary_axis"].as<Vector<3>>();

                PolyspherocylinderTraits::PolyspherocylinderGeometry geometry(
                    polysc["scs"].as<std::vector<SpherocylinderData>>(), primaryAxis, secondaryAxis,
                    polysc["geometric_center"].as<Vector<3>>(), polysc["volume"].as<double>(),
                    polysc["named_points"].as<ShapeGeometry::NamedPoints>()
                );

                return std::make_shared<PolyspherocylinderTraits>(std::move(geometry));
            });
    }
}


MatcherAlternative PolyspherocylinderMatcher::create() {
    return create_spherocylinder_matcher()
        | create_polyspherocylinder_banana_matcher()
        | create_polyspherocylinder_matcher();
}
