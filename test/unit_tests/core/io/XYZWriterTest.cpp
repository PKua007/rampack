//
// Created by pkua on 22.11.22.
//

#include "catch2/catch.hpp"

#include "core/io/XYZWriter.h"
#include "core/PeriodicBoundaryConditions.h"
#include "core/shapes/SphereTraits.h"


TEST_CASE("XYZWriter") {
    SphereTraits traits(0.5);
    auto pbc = std::make_unique<PeriodicBoundaryConditions>();
    std::vector<Shape> shapes;
    shapes.emplace_back(Vector<3>{0.5, 0.5, 0.5});
    shapes.emplace_back(Vector<3>{4.5, 0.5, 0.5});
    shapes.emplace_back(Vector<3>{2.5, 2.5, 4.0});
    TriclinicBox box(std::array<Vector<3>, 3>{{{5, 0, 0.1}, {0, 5, 0}, {0, 0, 5}}});
    Packing packing(box, std::move(shapes), std::move(pbc), traits.getInteraction(), traits.getDataManager());
    std::ostringstream out;
    out.precision(1);
    out << std::fixed;

    std::map<std::string, std::string> auxInfo = {{"key1", "value 1"}, {"key2", "value2"}};
    XYZWriter writer;
    writer.write(out, packing, traits, auxInfo);

    auto expectedOut =
R"(3
Lattice="5.0 0.0 0.1 0.0 5.0 0.0 0.0 0.0 5.0" Properties=species:S:1:pos:R:3:orientation:R:4 key1="value 1" key2=value2
A 0.5 0.5 0.5 0.0 0.0 0.0 1.0
A 4.5 0.5 0.5 0.0 0.0 0.0 1.0
A 2.5 2.5 4.0 0.0 0.0 0.0 1.0
)";
    CHECK(out.str() == expectedOut);
}