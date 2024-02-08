//
// Created by pkua on 22.11.22.
//

#include "catch2/catch.hpp"

#include "core/io/XYZRecorder.h"

#include "core/PeriodicBoundaryConditions.h"
#include "core/shapes/SphereTraits.h"


TEST_CASE("XYZRecorder") {
    SphereTraits traits(0.5);
    auto pbc = std::make_unique<PeriodicBoundaryConditions>();
    std::vector<Shape> shapes;
    shapes.emplace_back(Vector<3>{0.5, 0.5, 0.5});
    shapes.emplace_back(Vector<3>{4.5, 0.5, 0.5});
    shapes.emplace_back(Vector<3>{2.5, 2.5, 4.0});
    TriclinicBox box(std::array<Vector<3>, 3>{{{5, 0, 0.1}, {0, 5, 0}, {0, 0, 5}}});
    Packing packing(box, std::move(shapes), std::move(pbc), traits.getInteraction(), traits.getDataManager());
    std::stringbuf outBuf;

    {
        auto out = std::make_unique<std::iostream>(&outBuf);
        out->precision(1);
        *out << std::fixed;
        XYZRecorder recorder(std::move(out), false);
        recorder.recordSnapshot(packing, 1000);
        packing.tryScaling(2, traits.getInteraction());
        recorder.recordSnapshot(packing, 2000);
    }

    auto expectedOut =
R"(3
Lattice="5.0 0.0 0.1 0.0 5.0 0.0 0.0 0.0 5.0" Properties=species:S:1:pos:R:3:orientation:R:4 cycles=1000
A 0.5 0.5 0.5 0.0 0.0 0.0 1.0
A 4.5 0.5 0.5 0.0 0.0 0.0 1.0
A 2.5 2.5 4.0 0.0 0.0 0.0 1.0
3
Lattice="10.0 0.0 0.2 0.0 10.0 0.0 0.0 0.0 10.0" Properties=species:S:1:pos:R:3:orientation:R:4 cycles=2000
A 1.0 1.0 1.0 0.0 0.0 0.0 1.0
A 9.0 1.0 1.0 0.0 0.0 0.0 1.0
A 5.0 5.0 8.0 0.0 0.0 0.0 1.0
)";
    CHECK(outBuf.str() == expectedOut);

    SECTION("last cycle number when appending") {
        auto out = std::make_unique<std::iostream>(&outBuf);
        XYZRecorder recorder(std::move(out), true);
        CHECK(recorder.getLastCycleNumber() == 2000);
    }
}