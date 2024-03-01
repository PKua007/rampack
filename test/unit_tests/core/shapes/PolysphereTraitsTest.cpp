//
// Created by Piotr Kubala on 23/12/2020.
//

#include <catch2/catch.hpp>

#include "matchers/VectorApproxMatcher.h"

#include "mocks/MockCentralInteraction.h"

#include "core/shapes/PolysphereTraits.h"
#include "core/PeriodicBoundaryConditions.h"


namespace {
    class DummyInteraction : public CentralInteraction {
    protected:
        [[nodiscard]] double calculateEnergyForDistance2([[maybe_unused]] double distance) const override { return 0; }
    };

    const ShapeData defaultData(PolysphereTraits::Data{0});
    const Shape defaultShape({}, Matrix<3, 3>::identity(), defaultData);

    const PolysphereShape dimer({{{0, 0, 0}, 0.5}, {{3, 0, 0}, 1}},
                                {1, 0, 0}, {0, 1, 0}, {0.5, 0, 0});
    const PolysphereShape trimer({{{0, 0, 0}, 0.5}, {{1.5, 0, 0}, 0.5}, {{3, 0, 0}, 1}},
                                 {-1, 0, 0}, {0, -1, 0}, {-0.5, 0, 0});
}

TEST_CASE("PolysphereTraits: basic") {
    PolysphereTraits traits;
    auto dimerData = traits.addSpecies("dimer", dimer);
    auto trimerData = traits.addSpecies("trimer", trimer);

    SECTION("hard interactions") {
        // Particles look and are placed like this [x - first sphere (center), o - last sphere]:
        //
        //       1   4   6   9
        //   ||  x-->o   o<--x  ||
        //
        // 4, 6 are tangent

        const Interaction &interaction = traits.getInteraction();
        PeriodicBoundaryConditions pbc(10);

        CHECK(interaction.hasHardPart());
        CHECK_FALSE(interaction.hasSoftPart());

        SECTION("overlap") {
            Shape shape1({1.01, 5, 5}, Matrix<3, 3>::identity(), dimerData);
            Shape shape2({9, 5, 5}, Matrix<3, 3>::rotation(0, M_PI, 0), trimerData);
            CHECK(interaction.overlapBetweenShapes(shape1, shape2, pbc));
        }

        SECTION("no overlap") {
            Shape shape1({0.99, 5, 5}, Matrix<3, 3>::identity(), dimerData);
            Shape shape2({9, 5, 5}, Matrix<3, 3>::rotation(0, M_PI, 0), trimerData);
            CHECK_FALSE(interaction.overlapBetweenShapes(shape1, shape2, pbc));
        }
    }

    SECTION("overlap with wall") {
        const Interaction &interaction = traits.getInteraction();

        CHECK(interaction.hasWallPart());

        SECTION("overlapping") {
            Shape shape({1.1, 1.1, 5}, Matrix<3, 3>::rotation({0, 0, 1}, M_PI/2), dimerData);
            CHECK(interaction.overlapWithWallForShape(shape, {0, 5, 0}, {0, -1, 0}));
        }

        SECTION("non-overlapping") {
            Shape shape({0.9, 0.9, 5}, Matrix<3, 3>::rotation({0, 0, 1}, M_PI/2), dimerData);
            CHECK_FALSE(interaction.overlapWithWallForShape(shape, {0, 5, 0}, {0, -1, 0}));
        }
    }

    SECTION("toWolfram") {
        Shape dimerShape({9, 5, 5}, Matrix<3, 3>::rotation(0, M_PI, 0), dimerData);
        Shape trimerShape({9, 5, 5}, Matrix<3, 3>::rotation(0, M_PI, 0), trimerData);
        auto printer = traits.getPrinter("wolfram", {});

        CHECK(printer->print(dimerShape) == "{Sphere[{9.000000, 5.000000, 5.000000},0.500000],"
                                            "Sphere[{6.000000, 5.000000, 5.000000},1.000000]}");
        CHECK(printer->print(trimerShape) == "{Sphere[{9.000000, 5.000000, 5.000000},0.500000],"
                                             "Sphere[{7.500000, 5.000000, 5.000000},0.500000],"
                                             "Sphere[{6.000000, 5.000000, 5.000000},1.000000]}");
    }

    SECTION("geometry") {
        const auto &geometry = traits.getGeometry();
        Shape dimerShape({}, Matrix<3, 3>::rotation(0, 0, M_PI_2), dimerData);
        Shape trimerShape({}, Matrix<3, 3>::rotation(0, 0, M_PI_2), trimerData);

        SECTION("getVolume") {
            CHECK(geometry.getVolume(dimerShape) == Approx(4.71238898038469));
            CHECK(geometry.getVolume(trimerShape) == Approx(5.235987755982988));
        }

        SECTION("primary axis") {
            // primary axis X rotated 90 deg around z axis => primary axis is Y (for dimer, -(...) for trimer)
            CHECK_THAT(geometry.getPrimaryAxis(dimerShape), IsApproxEqual({0, 1, 0}, 1e-8));
            CHECK_THAT(geometry.getPrimaryAxis(trimerShape), IsApproxEqual({0, -1, 0}, 1e-8));
        }

        SECTION("secondary axis") {
            // secondary axis Y rotated 90 deg around z axis => secondary axis is -X (for dimer, -(...) for trimer)
            CHECK_THAT(geometry.getSecondaryAxis(dimerShape), IsApproxEqual({-1, 0, 0}, 1e-8));
            CHECK_THAT(geometry.getSecondaryAxis(trimerShape), IsApproxEqual({1, 0, 0}, 1e-8));
        }

        SECTION("geometric origin") {
            CHECK_THAT(geometry.getGeometricOrigin(dimerShape), IsApproxEqual({0, 0.5, 0}, 1e-8));
            CHECK_THAT(geometry.getGeometricOrigin(trimerShape), IsApproxEqual({0, -0.5, 0}, 1e-8));
        }
    }

    SECTION("PolysphereShape query") {
        CHECK(traits.getSpecies(0) == dimer);
        CHECK(traits.getSpecies(1) == trimer);

        CHECK(traits.getSpecies("dimer") == dimer);
        CHECK(traits.getSpecies("trimer") == trimer);

        CHECK(traits.getSpeciesName(0) == "dimer");
        CHECK(traits.getSpeciesName(1) == "trimer");

        CHECK(traits.getSpeciesIdx("dimer") == 0);
        CHECK(traits.getSpeciesIdx("trimer") == 1);

        CHECK(traits.hasSpecies("dimer"));
        CHECK(traits.hasSpecies("trimer"));
        CHECK_FALSE(traits.hasSpecies("tetramer"));

        CHECK(traits.shapeDataForSpecies("dimer") == dimerData);
        CHECK(traits.shapeDataForSpecies("trimer") == trimerData);
    }

    SECTION("default shape") {
        CHECK_THROWS_WITH(traits.getDefaultSpecies(), Catch::Contains("not defined"));

        traits.setDefaultShape("trimer");

        CHECK(traits.getDefaultSpecies() == trimer);
        CHECK(traits.shapeDataForDefaultSpecies() == traits.shapeDataForSpecies("trimer"));
    }
}

TEST_CASE("PolysphereTraits: soft interactions") {
    PolysphereShape shape({{{0, 0, 0}, 0.5}, {{3, 0, 0}, 1}}, {1, 0, 0}, {0, 1, 0}, {0, 0, 0});
    PolysphereTraits traits(shape, std::make_unique<DummyInteraction>());
    const auto &interaction = dynamic_cast<const CentralInteraction &>(traits.getInteraction());

    CHECK(interaction.getInteractionCentres(defaultData.raw()) == std::vector<Vector<3>>{{0, 0, 0}, {3, 0, 0}});
}

TEST_CASE("PolysphereTraits: mass centre normalization") {
    double volume = 1;     // Volume is not important here, we are lazy and choose an arbitrary number
    PolysphereShape shape({{{0, 0, 0}, 1}, {{1, 0, 0}, std::cbrt(3)}},
                          {1, 0, 0}, {0, 1, 0}, {1, 0, 0}, volume, {{"point1", {1, 0, 0}}});
    shape.normalizeMassCentre();
    PolysphereTraits traits(shape);

    const auto &sphereData = traits.getDefaultSpecies().getSphereData();
    CHECK(sphereData == std::vector<PolysphereTraits::SphereData>{{{-0.75, 0, 0}, 1}, {{0.25, 0, 0}, std::cbrt(3)}});
    const auto &geometry = traits.getGeometry();
    CHECK_THAT(geometry.getGeometricOrigin(defaultShape), IsApproxEqual(Vector<3>{0.25, 0, 0}, 1e-12));
    CHECK_THAT(geometry.getNamedPointForShape("point1", defaultShape), IsApproxEqual(Vector<3>{0.25, 0, 0}, 1e-12));
}

TEST_CASE("PolysphereTraits: named points") {
    double volume = 1;     // Volume is not important here, we are lazy and choose an arbitrary number
    PolysphereShape polysphereShape({{{0, 0, 0}, 1}, {{1, 0, 0}, 1}},
                                    {1, 0, 0}, {0, 1, 0}, {1, 0, 0}, volume,{{"named1", {0, 2, 0}}});
    PolysphereTraits traits(polysphereShape);
    const auto &geometry = traits.getGeometry();

    Shape shape({1, 2, 3}, Matrix<3, 3>::rotation(0, 0, M_PI/2), defaultData);
    CHECK_THAT(geometry.getNamedPointForShape("s0", shape), IsApproxEqual(Vector<3>{1, 2, 3} + Vector<3>{0, 0, 0}, 1e-12));
    CHECK_THAT(geometry.getNamedPointForShape("s1", shape), IsApproxEqual(Vector<3>{1, 2, 3} + Vector<3>{0, 1, 0}, 1e-12));
    CHECK_THAT(geometry.getNamedPointForShape("named1", shape), IsApproxEqual(Vector<3>{1, 2, 3} + Vector<3>{-2, 0, 0}, 1e-12));
    CHECK_THAT(geometry.getNamedPointForShape("o", shape), IsApproxEqual(Vector<3>{1, 2, 3} + Vector<3>{0, 1, 0}, 1e-12));
}

TEST_CASE("PolysphereTraits: serialization") {
    SECTION("default data") {
        PolysphereTraits traits(dimer);
        const auto &manager = traits.getDataManager();

        CHECK(manager.defaultDeserialize({}) == defaultData);
        CHECK(manager.getDefaultShapeData() == std::map<std::string, std::string>{{"species", "A"}});
    }

    SECTION("serialization & deserialization") {
        PolysphereTraits traits;
        traits.addSpecies("dimer", dimer);
        auto trimerData = traits.addSpecies("trimer", trimer);
        const auto &manager = traits.getDataManager();

        TextualShapeData textualData{{"species", "trimer"}};
        CHECK(manager.serialize(trimerData) == textualData);
        CHECK(manager.deserialize(textualData) == trimerData);
    }
}