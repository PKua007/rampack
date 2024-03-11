//
// Created by pkua on 22.11.22.
//

#include "catch2/catch.hpp"

#include "core/io/XYZWriter.h"
#include "core/PeriodicBoundaryConditions.h"
#include "core/shapes/SpherocylinderTraits.h"
#include "core/shapes/PolysphereTraits.h"


TEST_CASE("XYZWriter") {
    auto pbc = std::make_unique<PeriodicBoundaryConditions>();
    std::vector<Shape> shapes;
    shapes.emplace_back(Vector<3>{0.5, 0.5, 0.5});
    shapes.emplace_back(Vector<3>{4.5, 0.5, 0.5});
    TriclinicBox box(std::array<Vector<3>, 3>{{{5, 0, 0.1}, {0, 5, 0}, {0, 0, 5}}});
    std::map<std::string, std::string> auxInfo = {{"key1", "value 1"},
                                                  {"key2", "value2"}};
    std::ostringstream out;
    out.precision(1);
    out << std::fixed;

    SpherocylinderTraits scTraits;
    using ScData = SpherocylinderTraits::Data;

    PolysphereTraits polysphereTraits;
    auto monomerData = polysphereTraits.addSpecies("monomer", PolysphereShape({{{0, 0, 0}, 0.5}}));
    auto dimerData = polysphereTraits.addSpecies("dimer", PolysphereShape({{{0, 0, -0.5}, 0.5}, {{0, 0, 0.5}, 0.5}}));

    SECTION("monodisperse not named") {
        shapes[0].setData(ScData{1, 0.5});
        shapes[1].setData(ScData{1, 0.5});
        Packing packing(box, std::move(shapes), std::move(pbc), scTraits.getInteraction(), scTraits.getDataManager());

        XYZWriter writer;
        writer.write(out, packing, scTraits.getDataManager(), auxInfo);

        auto expectedOut =
                R"(2
Lattice="5.0 0.0 0.1 0.0 5.0 0.0 0.0 0.0 5.0" Properties=species:S:1:pos:R:3:orientation:R:4 key1="value 1" key2=value2
A 0.5 0.5 0.5 0.0 0.0 0.0 1.0
A 4.5 0.5 0.5 0.0 0.0 0.0 1.0
)";
        CHECK(out.str() == expectedOut);
    }

    SECTION("monodisperse named") {
        shapes[0].setData(ScData{1, 0.5});
        shapes[1].setData(ScData{1, 0.5});
        Packing packing(box, std::move(shapes), std::move(pbc), scTraits.getInteraction(), scTraits.getDataManager());

        XYZWriter writer(XYZWriter::SpeciesMap{{"sc", ShapeData(ScData{1, 0.5})}});
        writer.write(out, packing, scTraits.getDataManager(), auxInfo);

        auto expectedOut =
                R"(2
Lattice="5.0 0.0 0.1 0.0 5.0 0.0 0.0 0.0 5.0" Properties=species:S:1:pos:R:3:orientation:R:4 key1="value 1" key2=value2
sc 0.5 0.5 0.5 0.0 0.0 0.0 1.0
sc 4.5 0.5 0.5 0.0 0.0 0.0 1.0
)";
        CHECK(out.str() == expectedOut);
    }

    SECTION("monodisperse species") {
        shapes[0].setData(monomerData);
        shapes[1].setData(monomerData);
        Packing packing(box, std::move(shapes), std::move(pbc), polysphereTraits.getInteraction(),
                        polysphereTraits.getDataManager());

        XYZWriter writer;
        writer.write(out, packing, polysphereTraits.getDataManager(), auxInfo);

        auto expectedOut =
                R"(2
Lattice="5.0 0.0 0.1 0.0 5.0 0.0 0.0 0.0 5.0" Properties=species:S:1:pos:R:3:orientation:R:4 key1="value 1" key2=value2
monomer 0.5 0.5 0.5 0.0 0.0 0.0 1.0
monomer 4.5 0.5 0.5 0.0 0.0 0.0 1.0
)";
        CHECK(out.str() == expectedOut);
    }

    SECTION("polydisperse (named + not named)") {
        shapes[0].setData(ScData{1, 0.5});
        shapes[1].setData(ScData {2, 0.5});
        Packing packing(box, std::move(shapes), std::move(pbc), scTraits.getInteraction(), scTraits.getDataManager());
        XYZWriter::SpeciesMap speciesMap{{"short", ShapeData(ScData{1, 0.5})}};
        XYZWriter writer(std::move(speciesMap));

        writer.write(out, packing, scTraits.getDataManager(), auxInfo);

        auto expectedOut =
                R"(2
Lattice="5.0 0.0 0.1 0.0 5.0 0.0 0.0 0.0 5.0" Properties=species:S:1:pos:R:3:orientation:R:4 key1="value 1" key2=value2
short 0.5 0.5 0.5 0.0 0.0 0.0 1.0
l:2/r:0.5 4.5 0.5 0.5 0.0 0.0 0.0 1.0
)";
        CHECK(out.str() == expectedOut);
    }

    SECTION("polydisperse species (normal + renamed)") {
        shapes[0].setData(monomerData);
        shapes[1].setData(dimerData);
        Packing packing(box, std::move(shapes), std::move(pbc), polysphereTraits.getInteraction(),
                        polysphereTraits.getDataManager());
        XYZWriter::SpeciesMap speciesMap{{"dimer_renamed", dimerData}};
        XYZWriter writer(std::move(speciesMap));

        writer.write(out, packing, polysphereTraits.getDataManager(), auxInfo);

        auto expectedOut =
                R"(2
Lattice="5.0 0.0 0.1 0.0 5.0 0.0 0.0 0.0 5.0" Properties=species:S:1:pos:R:3:orientation:R:4 key1="value 1" key2=value2
monomer 0.5 0.5 0.5 0.0 0.0 0.0 1.0
dimer_renamed 4.5 0.5 0.5 0.0 0.0 0.0 1.0
)";
        CHECK(out.str() == expectedOut);
    }
}