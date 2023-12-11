//
// Created by Piotr Kubala on 11/12/2023.
//

#include <catch2/catch.hpp>

#include "mocks/MockShapeTraits.h"

#include "core/observables/NematicOrder.h"
#include "core/PeriodicBoundaryConditions.h"


TEST_CASE("NematicOrder") {
    using trompeloeil::_;

    MockShapeTraits mockShapeTraits;
    ALLOW_CALL(mockShapeTraits, getPrimaryAxis(_)).RETURN(_1.getOrientation() * Vector<3>{1, 0, 0});
    ALLOW_CALL(mockShapeTraits, getSecondaryAxis(_)).RETURN(_1.getOrientation() * Vector<3>{0, 1, 0});
    ALLOW_CALL(mockShapeTraits, getRangeRadius()).RETURN(std::numeric_limits<double>::infinity());
    ALLOW_CALL(mockShapeTraits, getInteractionCentres()).RETURN(std::vector<Vector<3>>{});
    auto pbc = std::make_unique<PeriodicBoundaryConditions>();

    SECTION("positive order with Q-tensor dump") {
        NematicOrder nematicOrder(true);
        Shape s1({1, 1, 1}, Matrix<3, 3>::rotation(M_PI/4, 0, 0));
        Shape s2({2, 3, 3}, Matrix<3, 3>::rotation(0, M_PI/4, 0));
        Shape s3({2.5, 3, 1}, Matrix<3, 3>::rotation(0, 0, M_PI/4));
        Packing packing({3, 4, 5}, {s1, s2, s3}, std::move(pbc), mockShapeTraits.getInteraction(), 1, 1);

        nematicOrder.calculate(packing, 1, 1, mockShapeTraits);

        REQUIRE(nematicOrder.getIntervalValues().size() == 7);
        CHECK(nematicOrder.getIntervalValues()[0] == Approx(0.6403882032022076)); // Mathematica
        CHECK(nematicOrder.getIntervalValues()[1] == Approx(0.5));
        CHECK(nematicOrder.getIntervalValues()[2] == Approx(0.25));
        CHECK(nematicOrder.getIntervalValues()[3] == Approx(-0.25));
        CHECK(nematicOrder.getIntervalValues()[4] == Approx(-0.25));
        CHECK(nematicOrder.getIntervalValues()[5] == Approx(0));
        CHECK(nematicOrder.getIntervalValues()[6] == Approx(-0.25));
    }

    SECTION("different axes + negative order") {
        NematicOrder nematicOrderPrimary(false, ShapeGeometry::Axis::PRIMARY);
        NematicOrder nematicOrderSecondary(false, ShapeGeometry::Axis::SECONDARY);
        Shape s1({1, 1, 1});
        Shape s2({2, 3, 3}, Matrix<3, 3>::rotation(M_PI/2, 0, 0));
        auto pbc2 = std::make_unique<PeriodicBoundaryConditions>();
        Packing packing({3, 4, 5}, {s1, s2}, std::move(pbc2), mockShapeTraits.getInteraction(), 1, 1);

        nematicOrderPrimary.calculate(packing, 1, 1, mockShapeTraits);
        nematicOrderSecondary.calculate(packing, 1, 1, mockShapeTraits);

        REQUIRE(nematicOrderPrimary.getIntervalValues().size() == 1);
        CHECK(nematicOrderPrimary.getIntervalValues()[0] == Approx(1));
        REQUIRE(nematicOrderSecondary.getIntervalValues().size() == 1);
        CHECK(nematicOrderSecondary.getIntervalValues()[0] == Approx(-1./2));
    }

    SECTION("name + header") {
        using Axis = ShapeGeometry::Axis;

        CHECK(NematicOrder().getName() == "nematic order");
        CHECK(NematicOrder(false).getIntervalHeader() == std::vector<std::string>{"P2"});
        CHECK(NematicOrder(true).getIntervalHeader()
              == std::vector<std::string>{"P2", "Q_11", "Q_12", "Q_13", "Q_22", "Q_23", "Q_33"});
        CHECK(NematicOrder(false, Axis::PRIMARY).getIntervalHeader() == std::vector<std::string>{"P2_pa"});
        CHECK(NematicOrder(true, Axis::PRIMARY).getIntervalHeader()
              == std::vector<std::string>{"P2_pa", "Q_pa_11", "Q_pa_12", "Q_pa_13", "Q_pa_22", "Q_pa_23", "Q_pa_33"});
        CHECK(NematicOrder(false, Axis::SECONDARY).getIntervalHeader() == std::vector<std::string>{"P2_sa"});
        CHECK(NematicOrder(false, Axis::AUXILIARY).getIntervalHeader() == std::vector<std::string>{"P2_aa"});
    }

    SECTION("bug: numerical stability of eigenvalues") {
        Matrix<3, 3> problematicMatrix{0.99999999906141479, -1.3709884701805501e-07, 1.7691093732719609e-07,
                                       -1.3709884701805501e-07, -0.49999999981461402, 3.6863738798600002e-10,
                                       -1.7691093732719609e-07, 3.6863738798600002e-10, -0.49999999924680083};

        // It would throw because an argument of cos is 1 + 2e-16 due to a numerical error
        CHECK_NOTHROW(NematicOrder::calculateEigenvalues(problematicMatrix));
    }
}