//
// Created by pkua on 22.11.22.
//

#include <catch2/catch.hpp>

#include "core/io/WolframWriter.h"
#include "core/PeriodicBoundaryConditions.h"
#include "core/shapes/SphereTraits.h"


TEST_CASE("WolframWriter") {
    SphereTraits traits(0.5);
    auto pbc = std::make_unique<PeriodicBoundaryConditions>();
    std::vector<Shape> shapes;
    shapes.emplace_back(Vector<3>{0.5, 0.5, 0.5});
    shapes.emplace_back(Vector<3>{2.5, 2.5, 4.0});
    Packing packing(TriclinicBox(5), std::move(shapes), std::move(pbc), traits.getInteraction());
    std::ostringstream out;
    out.precision(1);
    out << std::fixed;

    std::map<std::string, std::string> auxInfo = {{"key1", "value 1"}, {"key2", "value2"}};

    SECTION("WolframStyle::STANDARD") {
        WolframWriter writer(WolframWriter::WolframStyle::STANDARD);
        writer.write(out, packing, traits, auxInfo);

        auto expectedOut =
R"(Graphics3D[{
Sphere[{0.5, 0.5, 0.5},0.5],
Sphere[{2.5, 2.5, 4},0.5]}]
)";
        CHECK(out.str() == expectedOut);
    }

    SECTION("WolframStyle::AFFINE_TRANSFORM") {
        WolframWriter writer(WolframWriter::WolframStyle::AFFINE_TRANSFORM);
        writer.write(out, packing, traits, auxInfo);

        auto expectedOut =
R"(Graphics3D[GeometricTransformation[
Sphere[{0, 0, 0},0.5],AffineTransform@#]& /@ {
{{{1.0, 0.0, 0.0}, {0.0, 1.0, 0.0}, {0.0, 0.0, 1.0}}, {0.5, 0.5, 0.5}},
{{{1.0, 0.0, 0.0}, {0.0, 1.0, 0.0}, {0.0, 0.0, 1.0}}, {2.5, 2.5, 4.0}}
}]
)";
        CHECK(out.str() == expectedOut);
    }
}