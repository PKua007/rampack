//
// Created by pkua on 22.11.22.
//

#include "catch2/catch.hpp"

#include "core/io/WolframWriter.h"
#include "core/PeriodicBoundaryConditions.h"
#include "core/shapes/SphereTraits.h"


TEST_CASE("WolframWriter") {
    using SphereData = SphereTraits::HardData;
    const auto NO_ROT = Matrix<3, 3>::identity();

    SphereTraits traits;
    std::vector<Shape> shapes;
    auto pbc = std::make_unique<PeriodicBoundaryConditions>();

    std::ostringstream out;
    out.precision(1);
    out << std::fixed;

    SECTION("monodisperse") {
        shapes.emplace_back(Vector<3>{0.5, 0.5, 0.5}, NO_ROT, SphereData{0.5});
        shapes.emplace_back(Vector<3>{2.5, 2.5, 4.0}, NO_ROT, SphereData{0.5});
        Packing packing(TriclinicBox(5), std::move(shapes), std::move(pbc), traits.getInteraction(),
                        traits.getDataManager());

        SECTION("WolframStyle::STANDARD") {
            WolframWriter writer(WolframWriter::WolframStyle::STANDARD);
            writer.write(out, packing, traits, {});

            auto expectedOut =
R"(Graphics3D[{
Sphere[{0.5, 0.5, 0.5},0.5],
Sphere[{2.5, 2.5, 4},0.5]}]
)";
            CHECK(out.str() == expectedOut);
        }

        SECTION("WolframStyle::AFFINE_TRANSFORM") {
            WolframWriter writer(WolframWriter::WolframStyle::AFFINE_TRANSFORM);
            writer.write(out, packing, traits, {});

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

    SECTION("polidysperse") {
        shapes.emplace_back(Vector<3>{0.5, 0.5, 0.5}, NO_ROT, SphereData{0.5});
        shapes.emplace_back(Vector<3>{1.5, 1.5, 0.5}, NO_ROT, SphereData{0.5});
        shapes.emplace_back(Vector<3>{2.5, 2.5, 4.0}, NO_ROT, SphereData{0.25});
        shapes.emplace_back(Vector<3>{3.5, 3.5, 4.0}, NO_ROT, SphereData{0.25});
        Packing packing(TriclinicBox(5), std::move(shapes), std::move(pbc), traits.getInteraction(),
                        traits.getDataManager());

        SECTION("WolframStyle::AFFINE_TRANSFORM") {
            WolframWriter writer(WolframWriter::WolframStyle::AFFINE_TRANSFORM);
            writer.write(out, packing, traits, {});

            auto expectedOut =
R"(Graphics3D[{
GeometricTransformation[
Sphere[{0, 0, 0},0.5],AffineTransform@#]& /@ {
{{{1.0, 0.0, 0.0}, {0.0, 1.0, 0.0}, {0.0, 0.0, 1.0}}, {0.5, 0.5, 0.5}},
{{{1.0, 0.0, 0.0}, {0.0, 1.0, 0.0}, {0.0, 0.0, 1.0}}, {1.5, 1.5, 0.5}}
},
GeometricTransformation[
Sphere[{0, 0, 0},0.25],AffineTransform@#]& /@ {
{{{1.0, 0.0, 0.0}, {0.0, 1.0, 0.0}, {0.0, 0.0, 1.0}}, {2.5, 2.5, 4.0}},
{{{1.0, 0.0, 0.0}, {0.0, 1.0, 0.0}, {0.0, 0.0, 1.0}}, {3.5, 3.5, 4.0}}
}
}]
)";
            CHECK(out.str() == expectedOut);
        }
    }
}