//
// Created by Piotr Kubala on 16/12/2022.
//

#include "NewShapeFactory.h"

#include "core/shapes/SphereTraits.h"
#include "core/shapes/KMerTraits.h"
#include "core/shapes/PolysphereBananaTraits.h"
#include "core/shapes/PolysphereLollipopTraits.h"
#include "core/shapes/PolysphereWedgeTraits.h"
#include "core/shapes/SpherocylinderTraits.h"
#include "core/shapes/PolyspherocylinderBananaTraits.h"
#include "core/shapes/SmoothWedgeTraits.h"

#include "core/interactions/CentralInteraction.h"
#include "core/interactions/LennardJonesInteraction.h"
#include "core/interactions/RepulsiveLennardJonesInteraction.h"
#include "core/interactions/SquareInverseCoreInteraction.h"


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


    auto hardInteraction = MatcherNone{} | MatcherDataclass("hard");
    auto softInteraction = create_lj_matcher() | create_wca_matcher() | create_square_inverse_core_matcher();
    auto sphereInteraction = hardInteraction | softInteraction;


    MatcherDataclass create_lj_matcher() {
        return MatcherDataclass("lj")
            .arguments({{"epsilon", MatcherFloat{}.positive()},
                        {"sigma", MatcherFloat{}.positive()}})
            .mapTo([](const DataclassData &lj) {
                return std::make_shared<LennardJonesInteraction>(
                    lj["epsilon"].as<double>(), lj["sigma"].as<double>()
                );
            });
    }

    MatcherDataclass create_wca_matcher() {
        return MatcherDataclass("wca")
            .arguments({{"epsilon", MatcherFloat{}.positive()},
                        {"sigma", MatcherFloat{}.positive()}})
            .mapTo([](const DataclassData &wca) {
                return std::make_shared<RepulsiveLennardJonesInteraction>(
                    wca["epsilon"].as<double>(), wca["sigma"].as<double>()
                );
            });
    }

    MatcherDataclass create_square_inverse_core_matcher() {
        return MatcherDataclass("square_inverse_core")
            .arguments({{"epsilon", MatcherFloat{}.positive()},
                        {"sigma", MatcherFloat{}.positive()}})
            .mapTo([](const DataclassData &square_inverse_core) {
                return std::make_shared<SquareInverseCoreInteraction>(
                    square_inverse_core["epsilon"].as<double>(), square_inverse_core["sigma"].as<double>()
                );
            });
    }

    MatcherDataclass create_sphere_matcher() {
        return MatcherDataclass("sphere")
            .arguments({{"r", MatcherFloat{}.positive()},
                        {"interaction", sphereInteraction, std::shared_ptr<CentralInteraction>(nullptr)}})
            .mapTo([](const DataclassData &sphere) {
                auto r = sphere["r"].as<double>();
                auto interaction = sphere["interaction"].as<std::shared_ptr<CentralInteraction>>();
                if (interaction == nullptr)
                    return std::make_shared<SphereTraits>(r);
                else
                    return std::make_shared<SphereTraits>(r, interaction);
            });
    }

    MatcherDataclass create_kmer_matcher() {
        return MatcherDataclass("kmer")
            .arguments({{"k", MatcherInt{}.greaterEquals(2).mapTo<std::size_t>()},
                        {"r", MatcherFloat{}.positive()},
                        {"distance", MatcherFloat{}.positive()},
                        {"interaction", sphereInteraction, std::shared_ptr<CentralInteraction>(nullptr)}})
            .mapTo([](const DataclassData &kmer) {
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
                        {"arc_angle", MatcherFloat{}.positive()},
                        {"interaction", sphereInteraction, std::shared_ptr<CentralInteraction>(nullptr)}})
            .mapTo([](const DataclassData &banana) {
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
                        {"small_r", MatcherFloat{}.positive()},
                        {"large_r", MatcherFloat{}.positive()},
                        {"small_penetration", MatcherFloat{}.positive(), 0},
                        {"large_penetration", MatcherFloat{}.positive(), 0},
                        {"interaction", sphereInteraction, std::shared_ptr<CentralInteraction>(nullptr)}})
            .filter([](const DataclassData &lollipop) {
                return lollipop["small_penetration"].as<double>() < 2 * lollipop["small_r"].as<double>();
            })
            .filter([](const DataclassData &lollipop) {
                double smallerR = std::min(lollipop["small_r"].as<double>(), lollipop["large_r"].as<double>());
                return lollipop["small_penetration"].as<double>() < 2 * smallerR;
            })
            .mapTo([](const DataclassData &lollipop) {
                auto sphereN = lollipop["sphere_n"].as<std::size_t>();
                auto smallR = lollipop["small_r"].as<double>();
                auto largeR = lollipop["large_r"].as<double>();
                auto smallPenetration = lollipop["small_penetration"].as<double>();
                auto largePenetration = lollipop["large_penetration"].as<double>();
                auto interaction = lollipop["interaction"].as<std::shared_ptr<CentralInteraction>>();
                if (interaction == nullptr) {
                    return std::make_shared<PolysphereLollipopTraits>(
                        sphereN, smallR, largeR, smallPenetration, largePenetration
                    );
                } else {
                    return std::make_shared<PolysphereLollipopTraits>(
                        sphereN, smallR, largeR, smallPenetration,largePenetration, interaction
                    );
                }
            });
    }

    MatcherDataclass create_polysphere_wedge_matcher() {
        return MatcherDataclass("polysphere_wedge")
            .arguments({{"sphere_n", MatcherInt{}.greaterEquals(2).mapTo<std::size_t>()},
                        {"bottom_r", MatcherFloat{}.positive()},
                        {"top_r", MatcherFloat{}.positive()},
                        {"penetration", MatcherFloat{}.positive(), 0},
                        {"interaction", sphereInteraction, std::shared_ptr<CentralInteraction>(nullptr)}})
            .filter([](const DataclassData &wedge) {
                double smallerR = std::min(wedge["small_r"].as<double>(), wedge["large_r"].as<double>());
                return wedge["penetration"].as<double>() < 2 * smallerR;
            })
            .mapTo([](const DataclassData &wedge) {
                auto sphereN = wedge["sphere_n"].as<std::size_t>();
                auto bottomR = wedge["bottom_r"].as<double>();
                auto topR = wedge["top_r"].as<double>();
                auto penetration = wedge["penetration"].as<double>();
                auto interaction = wedge["interaction"].as<std::shared_ptr<CentralInteraction>>();
                if (interaction == nullptr)
                    return std::make_shared<PolysphereWedgeTraits>(sphereN, bottomR, topR, penetration);
                else
                    return std::make_shared<PolysphereWedgeTraits>(sphereN, bottomR, topR, penetration, interaction);
            });
    }

    MatcherDataclass create_spherocylinder_matcher() {
        return MatcherDataclass("spherocylinder")
            .arguments({{"l", MatcherFloat{}.positive()},
                        {"r", MatcherFloat{}.positive()}})
            .mapTo([](const DataclassData &sc) {
                return std::make_shared<SpherocylinderTraits>(sc["l"].as<double>(), sc["r"].as<double>());
            });
    }

    MatcherDataclass create_polyspherocylinder_banana_matcher() {
        return MatcherDataclass("polyspherocylinder_banana")
            .arguments({{"segment_n", MatcherInt{}.positive().mapTo<std::size_t>()},
                        {"sc_r", MatcherFloat{}.positive()},
                        {"arc_r", MatcherFloat{}.positive()},
                        {"arc_angle", MatcherFloat{}.positive()},
                        {"subdivisions", MatcherInt{}.positive().mapTo<std::size_t>(), 1}})
            .mapTo([](const DataclassData &banana) {
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
            .arguments({{"length", MatcherFloat{}.positive()},
                        {"bottom_r", MatcherFloat{}.positive()},
                        {"top_r", MatcherFloat{}.positive()},
                        {"subdivisions", MatcherInt{}.positive().mapTo<std::size_t>(), 1}})
            .filter([](const DataclassData &wedge){
                double rDiff = std::abs(wedge["bottom_r"].as<double>() - wedge["top_r"].as<double>());
                return wedge["length"].as<double>() >= rDiff;
            })
            .mapTo([](const DataclassData &wedge) {
                auto length = wedge["length"].as<double>();
                auto bottomR = wedge["bottom_r"].as<double>();
                auto topR = wedge["top_r"].as<double>();
                auto subdivisions = wedge["subdivisions"].as<std::size_t>();
                return std::make_shared<SmoothWedgeTraits>(bottomR, topR, length, subdivisions);
            });
    }
}


pyon::matcher::MatcherAlternative const NewShapeFactory::shape =
    create_sphere_matcher()
    | create_kmer_matcher()
    | create_polysphere_banana_matcher()
    | create_polysphere_lollipop_matcher()
    | create_polysphere_wedge_matcher()
    | create_spherocylinder_matcher()
    | create_polyspherocylinder_banana_matcher()
    | create_smooth_wedge_matcher();