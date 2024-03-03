//
// Created by Piotr Kubala on 16/12/2022.
//

#include "ShapeMatcher.h"

#include "core/shapes/SphereTraits.h"
#include "core/shapes/KMerTraits.h"
#include "core/shapes/PolysphereBananaTraits.h"
#include "core/shapes/PolysphereLollipopTraits.h"
#include "core/shapes/PolysphereWedgeTraits.h"
#include "core/shapes/SpherocylinderTraits.h"
#include "core/shapes/PolyspherocylinderBananaTraits.h"
#include "core/shapes/SmoothWedgeTraits.h"
#include "core/shapes/GenericXenoCollideTraits.h"

#include "core/interactions/CentralInteraction.h"
#include "core/interactions/LennardJonesInteraction.h"
#include "core/interactions/RepulsiveLennardJonesInteraction.h"
#include "core/interactions/SquareInverseCoreInteraction.h"

#include "geometry/xenocollide/XCBodyBuilder.h"
#include "core/shapes/PolyhedralWedgeTraits.h"

#include "GenericConvexGeometryMatcher.h"


using namespace pyon::matcher;

namespace {
    MatcherDataclass create_lj_matcher();
    MatcherDataclass create_wca_matcher();
    MatcherDataclass create_square_inverse_core_matcher();

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

    bool validate_axes(const DataclassData &dataclass);


    auto hardInteraction = MatcherDataclass("hard")
        .mapTo([](const auto &) -> std::shared_ptr<CentralInteraction> { return nullptr; });
    auto softInteraction = create_lj_matcher() | create_wca_matcher() | create_square_inverse_core_matcher();
    auto sphereInteraction = hardInteraction | softInteraction;

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

    auto namedPoints = MatcherDictionary{}.valuesMatch(vector).mapToStdMap<Vector<3>>();

    auto optionalPositiveDouble = MatcherNone{}.mapTo<std::optional<double>>()
                                | MatcherFloat{}.positive().mapTo<std::optional<double>>();
    auto optionalNonNegativeDouble = MatcherNone{}.mapTo<std::optional<double>>()
                                   | MatcherFloat{}.nonNegative().mapTo<std::optional<double>>();


    MatcherDataclass create_lj_matcher() {
        return MatcherDataclass("lj")
            .arguments({{"epsilon", MatcherFloat{}.positive()},
                        {"sigma", MatcherFloat{}.positive()}})
            .mapTo([](const DataclassData &lj) -> std::shared_ptr<CentralInteraction> {
                return std::make_shared<LennardJonesInteraction>(
                    lj["epsilon"].as<double>(), lj["sigma"].as<double>()
                );
            });
    }

    MatcherDataclass create_wca_matcher() {
        return MatcherDataclass("wca")
            .arguments({{"epsilon", MatcherFloat{}.positive()},
                        {"sigma", MatcherFloat{}.positive()}})
            .mapTo([](const DataclassData &wca) -> std::shared_ptr<CentralInteraction> {
                return std::make_shared<RepulsiveLennardJonesInteraction>(
                    wca["epsilon"].as<double>(), wca["sigma"].as<double>()
                );
            });
    }

    MatcherDataclass create_square_inverse_core_matcher() {
        return MatcherDataclass("square_inverse_core")
            .arguments({{"epsilon", MatcherFloat{}.positive()},
                        {"sigma", MatcherFloat{}.positive()}})
            .mapTo([](const DataclassData &square_inverse_core) -> std::shared_ptr<CentralInteraction> {
                return std::make_shared<SquareInverseCoreInteraction>(
                    square_inverse_core["epsilon"].as<double>(), square_inverse_core["sigma"].as<double>()
                );
            });
    }

    MatcherDataclass create_sphere_matcher() {
        return MatcherDataclass("sphere")
            .arguments({{"r", optionalPositiveDouble, "None"},
                        {"interaction", sphereInteraction, "hard"}})
            .filter([](const DataclassData &sphere) {
                return sphere["interaction"].as<std::shared_ptr<CentralInteraction>>() == nullptr
                       || sphere["r"].as<std::optional<double>>().has_value();
            })
            .describe("for interaction != hard, argument 'r' is required")
            .mapTo([](const DataclassData &sphere) -> std::shared_ptr<ShapeTraits> {
                auto r = sphere["r"].as<std::optional<double>>();
                auto interaction = sphere["interaction"].as<std::shared_ptr<CentralInteraction>>();
                if (interaction == nullptr)
                    return std::make_shared<SphereTraits>(r);
                else
                    return std::make_shared<SphereTraits>(*r, interaction);
            });
    }

    MatcherDataclass create_kmer_matcher() {
        return MatcherDataclass("kmer")
            .arguments({{"k", MatcherInt{}.greaterEquals(2).mapTo<std::size_t>()},
                        {"r", MatcherFloat{}.positive()},
                        {"distance", MatcherFloat{}.positive()},
                        {"interaction", sphereInteraction, "hard"}})
            .mapTo([](const DataclassData &kmer) -> std::shared_ptr<ShapeTraits> {
                auto k = kmer["k"].as<std::size_t>();
                auto r = kmer["r"].as<double>();
                auto distance = kmer["distance"].as<double>();
                auto interaction = kmer["interaction"].as<std::shared_ptr<CentralInteraction>>();
                if (interaction == nullptr)
                    return std::make_shared<KMerTraits>(k, r, distance);
                else
                    return std::make_shared<KMerTraits>(k, r, distance, interaction);
            });
    }

    MatcherDataclass create_polysphere_banana_matcher() {
        return MatcherDataclass("polysphere_banana")
            .arguments({{"sphere_n", MatcherInt{}.greaterEquals(2).mapTo<std::size_t>()},
                        {"sphere_r", MatcherFloat{}.positive()},
                        {"arc_r", MatcherFloat{}.positive()},
                        {"arc_angle", MatcherFloat{}.greaterEquals(0).less(2*M_PI)},
                        {"interaction", sphereInteraction, "hard"}})
            .mapTo([](const DataclassData &banana) -> std::shared_ptr<ShapeTraits> {
                auto sphereN = banana["sphere_n"].as<std::size_t>();
                auto sphereR = banana["sphere_r"].as<double>();
                auto arcR = banana["arc_r"].as<double>();
                auto argAngle = banana["arc_angle"].as<double>();
                auto interaction = banana["interaction"].as<std::shared_ptr<CentralInteraction>>();
                if (interaction == nullptr)
                    return std::make_shared<PolysphereBananaTraits>(arcR, argAngle, sphereN, sphereR);
                else
                    return std::make_shared<PolysphereBananaTraits>(arcR, argAngle, sphereN, sphereR, interaction);
            });
    }

    MatcherDataclass create_polysphere_lollipop_matcher() {
        return MatcherDataclass("polysphere_lollipop")
            .arguments({{"sphere_n", MatcherInt{}.greaterEquals(2).mapTo<std::size_t>()},
                        {"stick_r", MatcherFloat{}.positive()},
                        {"tip_r", MatcherFloat{}.positive()},
                        {"stick_penetration", MatcherFloat{}.nonNegative(), "0"},
                        {"tip_penetration", MatcherFloat{}.nonNegative(), "0"}})
            .filter([](const DataclassData &lollipop) {
                return lollipop["stick_penetration"].as<double>() < 2 * lollipop["stick_r"].as<double>();
            })
            .describe("stick_penetration < 2 * stick_r")
            .filter([](const DataclassData &lollipop) {
                double smallerR = std::min(lollipop["stick_r"].as<double>(), lollipop["tip_r"].as<double>());
                return lollipop["tip_penetration"].as<double>() < 2 * smallerR;
            })
            .describe("tip_penetration < 2 * min(stick_r, tip_r)")
            .mapTo([](const DataclassData &lollipop) -> std::shared_ptr<ShapeTraits> {
                auto sphereN = lollipop["sphere_n"].as<std::size_t>();
                auto stickR = lollipop["stick_r"].as<double>();
                auto tipR = lollipop["tip_r"].as<double>();
                auto stickPenetration = lollipop["stick_penetration"].as<double>();
                auto tipPenetration = lollipop["tip_penetration"].as<double>();
                return std::make_shared<PolysphereLollipopTraits>(
                    sphereN, stickR, tipR, stickPenetration, tipPenetration
                );
            });
    }

    MatcherDataclass create_polysphere_wedge_matcher() {
        return MatcherDataclass("polysphere_wedge")
            .arguments({{"sphere_n", MatcherInt{}.greaterEquals(2).mapTo<std::size_t>()},
                        {"bottom_r", MatcherFloat{}.positive()},
                        {"top_r", MatcherFloat{}.positive()},
                        {"penetration", MatcherFloat{}.nonNegative(), "0"}})
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
                return std::make_shared<PolysphereWedgeTraits>(sphereN, bottomR, topR, penetration);
            });
    }

    MatcherDataclass create_spherocylinder_matcher() {
        return MatcherDataclass("spherocylinder")
            .arguments({{"l", optionalPositiveDouble, "None"},
                        {"r", optionalPositiveDouble, "None"}})
            .mapTo([](const DataclassData &sc) -> std::shared_ptr<ShapeTraits> {
                return std::make_shared<SpherocylinderTraits>(
                    sc["l"].as<std::optional<double>>(),
                    sc["r"].as<std::optional<double>>()
                );
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

        auto sphere = MatcherDataclass("sphere")
            .arguments({{"pos", spherePos},
                        {"r", MatcherFloat{}.positive()}})
            .mapTo([](const DataclassData &sphere) {
                auto posVector = sphere["pos"].as<std::vector<Vector<3>>>();
                auto r = sphere["r"].as<double>();
                std::vector<PolysphereTraits::SphereData> sphereData;
                sphereData.reserve(posVector.size());
                auto sphereDataCreator = [r](const Vector<3> &pos) {
                    return PolysphereTraits::SphereData(pos, r);
                };
                std::transform(posVector.begin(), posVector.end(), std::back_inserter(sphereData), sphereDataCreator);
                return sphereData;
            });

        auto sphereArray = MatcherArray{}.elementsMatch(sphere)
            .nonEmpty()
            .mapTo([](const ArrayData &array) {
                std::vector<PolysphereTraits::SphereData> allSphereDatas;
                for (const auto &sphereDatas : array.asStdVector<std::vector<PolysphereTraits::SphereData>>())
                    for (const auto &sphereData : sphereDatas)
                        allSphereDatas.push_back(sphereData);
                return allSphereDatas;
            });

        // TODO: polydispersity
        return MatcherDataclass("polysphere")
            .arguments({{"spheres", sphere | sphereArray},
                        {"volume", MatcherFloat{}.positive()},
                        {"geometric_center", vector, "[0, 0, 0]"},
                        {"primary_axis", axis | MatcherNone{}, "None"},
                        {"secondary_axis", axis | MatcherNone{}, "None"},
                        {"named_points", namedPoints, "{}"},
                        {"interaction", sphereInteraction, "hard"}})
            .filter(validate_axes)
            .describe("primary_axis and secondary_axis must be orthogonal")
            .mapTo([](const DataclassData &polysphere) -> std::shared_ptr<ShapeTraits> {
                auto spheres = polysphere["spheres"].as<std::vector<PolysphereTraits::SphereData>>();
                auto volume = polysphere["volume"].as<double>();
                auto geometricOrigin = polysphere["geometric_center"].as<Vector<3>>();
                std::optional<Vector<3>> primaryAxis;
                if (!polysphere["primary_axis"].isEmpty())
                    primaryAxis = polysphere["primary_axis"].as<Vector<3>>();
                std::optional<Vector<3>> secondaryAxis;
                if (!polysphere["secondary_axis"].isEmpty())
                    secondaryAxis = polysphere["secondary_axis"].as<Vector<3>>();
                auto namedPoints = polysphere["named_points"].as<std::map<std::string, Vector<3>>>();
                auto interaction = polysphere["interaction"].as<std::shared_ptr<CentralInteraction>>();

                PolysphereShape shape(
                    std::move(spheres), primaryAxis, secondaryAxis, geometricOrigin, volume, namedPoints
                );

                if (interaction == nullptr)
                    return std::make_shared<PolysphereTraits>(shape);
                else
                    return std::make_shared<PolysphereTraits>(shape, interaction);
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
                auto namedPoints = polysc["named_points"].as<std::map<std::string, Vector<3>>>();

                std::vector<NamedPoint> properNamedPoints;
                properNamedPoints.reserve(namedPoints.size());
                for (const auto &[pointName, point] : namedPoints)
                    properNamedPoints.emplace_back(pointName, point);

                PolyspherocylinderTraits::PolyspherocylinderGeometry geometry(
                    std::move(sc), primaryAxis, secondaryAxis, geometricOrigin, volume, properNamedPoints
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
                auto namedPoints = convex["named_points"].as<std::map<std::string, Vector<3>>>();

                XCBodyBuilder builder;
                script(builder);
                auto geometry = builder.releaseCollideGeometry();

                GenericXenoCollideShape shape(
                    geometry, volume, primaryAxis, secondaryAxis, geometricOrigin, namedPoints
                );
                return std::make_shared<GenericXenoCollideTraits>(std::move(shape));
            });
    }

    MatcherDataclass create_smooth_wedge_matcher() {
        return MatcherDataclass("smooth_wedge")
            .arguments({{"l", optionalPositiveDouble, "None"},
                        {"bottom_r", optionalPositiveDouble, "None"},
                        {"top_r", optionalPositiveDouble, "None"},
                        {"subdivisions", MatcherInt{}.positive().mapTo<std::size_t>(), "1"}})
            .filter([](const DataclassData &wedge){
                auto length = wedge["l"].as<std::optional<double>>();
                auto bottomR = wedge["bottom_r"].as<std::optional<double>>();
                auto topR = wedge["top_r"].as<std::optional<double>>();
                if (!length || !bottomR || !topR)
                    return true;
                return *length >= std::abs(*bottomR - *topR);
            })
            .describe("l >= |bottom_r - top_r|")
            .mapTo([](const DataclassData &wedge) -> std::shared_ptr<ShapeTraits> {
                auto length = wedge["l"].as<std::optional<double>>();
                auto bottomR = wedge["bottom_r"].as<std::optional<double>>();
                auto topR = wedge["top_r"].as<std::optional<double>>();
                auto subdivisions = wedge["subdivisions"].as<std::size_t>();
                return std::make_shared<SmoothWedgeTraits>(bottomR, topR, length, subdivisions);
            });
    }

    MatcherDataclass create_polyhedral_wedge_matcher() {
        return MatcherDataclass("polyhedral_wedge")
            .arguments({{"bottom_ax", optionalNonNegativeDouble, "None"},
                        {"bottom_ay", optionalNonNegativeDouble, "None"},
                        {"top_ax", optionalNonNegativeDouble, "None"},
                        {"top_ay", optionalNonNegativeDouble, "None"},
                        {"l", optionalPositiveDouble, "None"},
                        {"subdivisions", MatcherInt{}.positive().mapTo<std::size_t>(), "1"}})
            .filter([](const DataclassData &wedge) {
                auto bottomAx = wedge["bottom_ax"].as<std::optional<double>>();
                auto bottomAy = wedge["bottom_ay"].as<std::optional<double>>();
                auto topAx = wedge["top_ax"].as<std::optional<double>>();
                auto topAy = wedge["top_ay"].as<std::optional<double>>();
                if (!bottomAx || !bottomAy || !topAx || !topAy)
                    return true;

                return (topAx != 0 && bottomAy != 0) || (topAy != 0 && bottomAx != 0);
            })
            .describe("with at least one pair of non-zero orthogonal a-s, one at the top, one at the bottom")
            .mapTo([](const DataclassData &wedge) -> std::shared_ptr<ShapeTraits> {
                return std::make_shared<PolyhedralWedgeTraits>(
                    wedge["bottom_ax"].as<std::optional<double>>(),
                    wedge["bottom_ay"].as<std::optional<double>>(),
                    wedge["top_ax"].as<std::optional<double>>(),
                    wedge["top_ay"].as<std::optional<double>>(),
                    wedge["l"].as<std::optional<double>>(),
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
