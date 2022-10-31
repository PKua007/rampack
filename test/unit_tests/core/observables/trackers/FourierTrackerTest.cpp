//
// Created by pkua on 24.10.22.
//

#include <catch2/catch.hpp>
#include <random>

#include "matchers/VectorApproxMatcher.h"

#include "core/observables/trackers/FourierTracker.h"
#include "core/PeriodicBoundaryConditions.h"
#include "core/shapes/SphereTraits.h"


TEST_CASE("FourierTracker") {
    double linearSize = 5;
    TriclinicBox box(linearSize);

    // Hardcoded seed gives deterministic sequence
    std::mt19937 mt(1234ul);
    std::uniform_real_distribution<double> dist(0, linearSize);
    std::vector<Shape> shapes;
    shapes.reserve(1000);
    for (std::size_t i{}; i < 1000; i++)
        shapes.emplace_back(Vector<3>{dist(mt), dist(mt), dist(mt)});

    SphereTraits traits(0.5);
    auto pbc = std::make_unique<PeriodicBoundaryConditions>();
    Packing packing(box, std::move(shapes), std::move(pbc), traits.getInteraction());

    SECTION("1D") {
        Vector<3> origin{0, 3.5, 0};
        auto function = [box, origin](const Shape &shape, const ShapeTraits &) {
            double y = box.absoluteToRelative(shape.getPosition())[1];
            double y0 = box.absoluteToRelative(origin)[1];
            return std::cos(2 * 2*M_PI*(y - y0));
        };
        FourierTracker tracker({0, 2, 0}, function, "test");

        tracker.calculateOrigin(packing, traits);
        // original maximum is in {0, 3.5, 0}, but this one is closer to {0, 0, 0}
        CHECK_THAT(tracker.getOriginPos(), IsApproxEqual({0, 1, 0}, 0.05));
    }

    SECTION("2D") {
        Vector<3> origin{0, 1, 0.5};
        auto function = [box, origin](const Shape &shape, const ShapeTraits &) {
            double y = box.absoluteToRelative(shape.getPosition())[1];
            double y0 = box.absoluteToRelative(origin)[1];
            double z = box.absoluteToRelative(shape.getPosition())[2];
            double z0 = box.absoluteToRelative(origin)[2];
            return std::cos(2 * 2*M_PI*(y - y0)) * std::cos(3 * 2*M_PI*(z - z0));
        };
        FourierTracker tracker({0, 2, 3}, function, "test");

        tracker.calculateOrigin(packing, traits);
        // original maximum is in {0, 1, 0.5}, but this one is closer to {0, 0, 0}
        CHECK_THAT(tracker.getOriginPos(), IsApproxEqual({0, 4.75, 4+2./3}, 0.05));
    }

    SECTION("3D") {
        Vector<3> origin{0.5, 1, 1.5};
        auto function = [box, origin](const Shape &shape, const ShapeTraits &) {
            double x = box.absoluteToRelative(shape.getPosition())[0];
            double x0 = box.absoluteToRelative(origin)[0];
            double y = box.absoluteToRelative(shape.getPosition())[1];
            double y0 = box.absoluteToRelative(origin)[1];
            double z = box.absoluteToRelative(shape.getPosition())[2];
            double z0 = box.absoluteToRelative(origin)[2];
            return std::cos(1 * 2*M_PI*(x - x0)) * std::cos(2 * 2*M_PI*(y - y0)) * std::cos(3 * 2*M_PI*(z - z0));
        };
        FourierTracker tracker({1, 2, 3}, function, "test");

        tracker.calculateOrigin(packing, traits);
        // original maximum is in {0.5, 1, 1.5}, but this one is closer to {0, 0, 0}
        CHECK_THAT(tracker.getOriginPos(), IsApproxEqual({0.5, 4.75, 2./3}, 0.05));
    }

    SECTION("name") {
        FourierTracker tracker({0, 2, 0}, [](const Shape &, const ShapeTraits &) { return 0.0; }, "test");
        CHECK(tracker.getName() == "test_fourier_tracker");
    }
}