//
// Created by Codex on 11/04/2026.
//

#include "XenoCollideMatcher.h"

#include <optional>

#include "GenericConvexGeometryMatcher.h"
#include "ShapeMatcherCommon.h"
#include "core/shapes/GenericXenoCollideTraits.h"
#include "core/shapes/PolyhedralWedgeTraits.h"
#include "core/shapes/SmoothWedgeTraits.h"
#include "geometry/xenocollide/XCBodyBuilder.h"


using namespace pyon::matcher;

namespace {
    MatcherDataclass create_smooth_wedge_matcher() {
        return MatcherDataclass("smooth_wedge")
            .arguments({{"l", MatcherFloat{}.positive()},
                        {"bottom_r", MatcherFloat{}.positive()},
                        {"top_r", MatcherFloat{}.positive()},
                        {"subdivisions", MatcherInt{}.positive().mapTo<std::size_t>(), "1"}})
            .filter([](const DataclassData &wedge) {
                return wedge["l"].as<double>()
                    >= std::abs(wedge["bottom_r"].as<double>() - wedge["top_r"].as<double>());
            })
            .describe("l >= |bottom_r - top_r|")
            .mapTo([](const DataclassData &wedge) -> std::shared_ptr<ShapeTraits> {
                return std::make_shared<SmoothWedgeTraits>(
                    wedge["bottom_r"].as<double>(), wedge["top_r"].as<double>(),
                    wedge["l"].as<double>(), wedge["subdivisions"].as<std::size_t>()
                );
            });
    }

    MatcherDataclass create_generic_convex_matcher() {
        return MatcherDataclass("generic_convex")
            .arguments({{"geometry", GenericConvexGeometryMatcher::script},
                        {"volume", MatcherFloat{}.positive()},
                        {"geometric_center", ShapeMatcherCommon::create_vector_matcher(), "[0, 0, 0]"},
                        {"primary_axis", ShapeMatcherCommon::create_axis_matcher() | MatcherNone{}, "None"},
                        {"secondary_axis", ShapeMatcherCommon::create_axis_matcher() | MatcherNone{}, "None"},
                        {"named_points", ShapeMatcherCommon::create_named_points_matcher(), "{}"}})
            .filter(ShapeMatcherCommon::validate_axes)
            .describe("primary_axis and secondary_axis must be orthogonal")
            .mapTo([](const DataclassData &convex) -> std::shared_ptr<ShapeTraits> {
                std::optional<Vector<3>> primaryAxis;
                if (!convex["primary_axis"].isEmpty())
                    primaryAxis = convex["primary_axis"].as<Vector<3>>();
                std::optional<Vector<3>> secondaryAxis;
                if (!convex["secondary_axis"].isEmpty())
                    secondaryAxis = convex["secondary_axis"].as<Vector<3>>();

                XCBodyBuilder builder;
                convex["geometry"].as<XCBodyBuilderScript>()(builder);
                auto geometry = builder.releaseCollideGeometry();

                return std::make_shared<GenericXenoCollideTraits>(
                    geometry, primaryAxis, secondaryAxis, convex["geometric_center"].as<Vector<3>>(),
                    convex["volume"].as<double>(), convex["named_points"].as<ShapeGeometry::NamedPoints>()
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
                auto bottomAx = wedge["bottom_ax"].as<double>();
                auto bottomAy = wedge["bottom_ay"].as<double>();
                auto topAx = wedge["top_ax"].as<double>();
                auto topAy = wedge["top_ay"].as<double>();
                return (topAx != 0 && bottomAy != 0) || (topAy != 0 && bottomAx != 0);
            })
            .describe("with at least one pair of non-zero orthogonal a-s, one at the top, one at the bottom")
            .mapTo([](const DataclassData &wedge) -> std::shared_ptr<ShapeTraits> {
                return std::make_shared<PolyhedralWedgeTraits>(
                    wedge["bottom_ax"].as<double>(), wedge["bottom_ay"].as<double>(),
                    wedge["top_ax"].as<double>(), wedge["top_ay"].as<double>(),
                    wedge["l"].as<double>(), wedge["subdivisions"].as<std::size_t>()
                );
            });
    }
}


MatcherAlternative XenoCollideMatcher::create() {
    return create_smooth_wedge_matcher()
        | create_generic_convex_matcher()
        | create_polyhedral_wedge_matcher();
}
