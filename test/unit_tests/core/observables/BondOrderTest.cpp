//
// Created by pkua on 07.04.2022.
//

#include <cmath>

#include <catch2/catch.hpp>

#include "core/observables/BondOrder.h"
#include "core/lattice/OrthorhombicArrangingModel.h"
#include "core/FreeBoundaryConditions.h"
#include "core/PeriodicBoundaryConditions.h"
#include "core/shapes/SphereTraits.h"

#include <iostream>

TEST_CASE("BondOrder: hexatic") {
    // We are creating a packing with 2 layers. Each layer has (n1 x n2) unit cells and is a perfect honeycomb which
    // has been squashed 0.8 times in one direction. The 2 layer are stacked on one another distant by shift[2] and also
    // shifted in XY direction by {shift[0], shift[1]} to form a triclinic box

    // cell dimensions
    double a = 1;
    double b = std::sqrt(3.0) * 0.8;    // * 1.0 would be perfect hexagonal lattice
    double c = 5;

    // translation of second particle in the cell
    double r1 = a / 2.;
    double r2 = b / 2.;

    // second layer shift
    Vector<3> shift = {1, 1, 5};

    std::size_t n1 = 4, n2 = 3;

    // Create half of hexagonal lattice
    OrthorhombicArrangingModel arrangingModel;
    std::array<double, 3> boxDimensions{(n1 - 1)*a + r1, (n2 - 1)*b + r2, shift[2]};
    auto shapes1 = arrangingModel.arrange(n1*n2, {n1, n2, 1}, {a, b, c}, boxDimensions);

    // Create the rest
    FreeBoundaryConditions fbc;
    auto shapes2 = shapes1;
    for (auto &shape : shapes2)
        shape.translate({r1, r2, 0}, fbc);
    shapes1.insert(shapes1.end(), shapes2.begin(), shapes2.end());

    // Create two layers, one above but offset to create triclinic box
    auto shapes3 = shapes1;
    for (auto &shape : shapes3)
        shape.translate({0, 0, shift[2]}, fbc);
    shapes1.insert(shapes1.end(), shapes3.begin(), shapes3.end());

    // Create box
    Vector<3> boxVector1{a*n1, 0, 0};
    Vector<3> boxVector2{0, b*n2, 0};
    Vector<3> boxVector3{0, 0, 2*shift[2]};
    TriclinicBox box({boxVector1, boxVector2, boxVector3});

    // Finally, create the packing
    auto pbc = std::make_unique<PeriodicBoundaryConditions>();
    SphereTraits sphereTraits(0.5);
    Packing packing(box, shapes1, std::move(pbc), sphereTraits.getInteraction(), 1, 1);

    // ... tilt it to create triclinic box
    packing.tryScaling(TriclinicBox({boxVector1, boxVector2, 2.*shift}), sphereTraits.getInteraction());

    //std::cout << packing.getBox().getDimensions() << std::endl;
    //packing.toWolfram(std::cout, sphereTraits.getPrinter());

    // ... and test the order parameter
    BondOrder bondOrder({4, 6}, {0, 0, 2});

    bondOrder.calculate(packing, 1, 1, sphereTraits);

    CHECK(bondOrder.getName() == "bond order");
    CHECK(bondOrder.getIntervalHeader() == std::vector<std::string>{"psi4", "psi6"});
    CHECK(bondOrder.getNominalHeader().empty());
    CHECK(bondOrder.getNominalValues().empty());

    // Values calculated in mathematica
    CHECK(bondOrder.getIntervalValues()[0] == Approx(0.8014636892475136));
    CHECK(bondOrder.getIntervalValues()[1] == Approx(0.8800669379487272));
}

TEST_CASE("BondOrder: distance within layer") {
    // We create two tetragonal layers shifted against each other. If the distance is calculated correctly - as a
    // projection onto a layer - order should be perfect.
    std::vector<Shape> shapes{Shape({1, 1, 0.25}), Shape({1, 3, 0.25}), Shape({1, 5, 0.25}),
                              Shape({3, 1, 0.25}), Shape({3, 3, 0.25}), Shape({3, 5, 0.25}),
                              Shape({5, 1, 0.25}), Shape({5, 3, 0.25}), Shape({5, 5, 0.25}),

                              Shape({1.1, 1.1, 0.75}), Shape({1.1, 3.1, 0.75}), Shape({1.1, 5.1, 0.75}),
                              Shape({3.1, 1.1, 0.75}), Shape({3.1, 3.1, 0.75}), Shape({3.1, 5.1, 0.75}),
                              Shape({5.1, 1.1, 0.75}), Shape({5.1, 3.1, 0.75}), Shape({5.1, 5.1, 0.75})};
    SphereTraits sphereTraits(0.1);
    auto pbc = std::make_unique<PeriodicBoundaryConditions>();
    TriclinicBox box(std::array<double, 3>{6, 6, 1});
    Packing packing(box, shapes, std::move(pbc), sphereTraits.getInteraction(), 1, 1);
    BondOrder bondOrder(4, {0, 0, 2});

    bondOrder.calculate(packing, 1, 1, sphereTraits);

    CHECK(bondOrder.getIntervalValues()[0] == Approx(1));
}