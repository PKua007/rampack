//
// Created by pkua on 21.05.22.
//

#include <catch2/catch.hpp>

#include "mocks/MockLayerWiseTransformer.h"

#include "core/lattice/LayerWiseTransformer.h"


TEST_CASE("LayerWiseTransformer: 1 layer in cell, 1 requested") {
    Lattice lattice(UnitCell(TriclinicBox(1), {Shape({0, 0.5, 0.5}), Shape({0.5, 0.5, 0.5})}), {2, 2, 2});
    MockLayerWiseTransformer transformer(LatticeTraits::Axis::Z);
    ALLOW_CALL(transformer, getRequestedNumOfLayers()).RETURN(1);
    REQUIRE_CALL(transformer, transformShape(Shape({0.5, 0.5, 0.5}), 0ul)).RETURN(Shape({0.5, 0.5, 0}));
    REQUIRE_CALL(transformer, transformShape(Shape({0, 0.5, 0.5}), 0ul)).RETURN(Shape({0, 0.5, 0}));

    transformer.transform(lattice);

    auto expectedMolecules = std::vector<Shape>{Shape({0.5, 0.5, 0}), Shape({0, 0.5, 0})};
    CHECK_THAT(lattice.getCell(0, 0, 0).getMolecules(), Catch::UnorderedEquals(expectedMolecules));
    CHECK(lattice.getCellBox() == TriclinicBox(1));
    CHECK(lattice.getLatticeBox() == TriclinicBox(2));
    CHECK(lattice.getDimensions() == std::array<std::size_t, 3>{2, 2, 2});
}

TEST_CASE("LayerWiseTransformer: 1 layer in cell, 2 requested") {
    Lattice lattice(UnitCell(TriclinicBox(1), {Shape({0, 0.5, 0.5}), Shape({0.5, 0.5, 0.5})}), {2, 2, 4});
    MockLayerWiseTransformer transformer(LatticeTraits::Axis::Z);
    ALLOW_CALL(transformer, getRequestedNumOfLayers()).RETURN(2);
    REQUIRE_CALL(transformer, transformShape(Shape({0, 0.5, 0.25}), 0ul)).RETURN(Shape({0, 0, 0.25}));
    REQUIRE_CALL(transformer, transformShape(Shape({0.5, 0.5, 0.25}), 0ul)).RETURN(Shape({0.5, 0, 0.25}));
    REQUIRE_CALL(transformer, transformShape(Shape({0, 0.5, 0.75}), 1ul)).RETURN(Shape({0, 0, 0.75}));
    REQUIRE_CALL(transformer, transformShape(Shape({0.5, 0.5, 0.75}), 1ul)).RETURN(Shape({0.5, 0, 0.75}));

    transformer.transform(lattice);

    auto expectedMolecules = std::vector<Shape>{Shape({0, 0, 0.25}), Shape({0.5, 0, 0.25}),
                                                Shape({0, 0, 0.75}), Shape({0.5, 0, 0.75})};
    CHECK_THAT(lattice.getCell(0, 0, 0).getMolecules(), Catch::UnorderedEquals(expectedMolecules));
    CHECK(lattice.getCellBox() == TriclinicBox(std::array<double, 3>{1, 1, 2}));
    CHECK(lattice.getLatticeBox() == TriclinicBox(std::array<double, 3>{2, 2, 4}));
    CHECK(lattice.getDimensions() == std::array<std::size_t, 3>{2, 2, 2});
}

TEST_CASE("LayerWiseTransformer: 2 layers in cell, 2 requested") {
    Lattice lattice(UnitCell(TriclinicBox(1),
                             {Shape({0, 0.5, 0.25}), Shape({0.5, 0.5, 0.25}),
                              Shape({0, 0.5, 0.75}), Shape({0.5, 0.5, 0.75})}),
                    {2, 2, 2});
    MockLayerWiseTransformer transformer(LatticeTraits::Axis::Z);
    ALLOW_CALL(transformer, getRequestedNumOfLayers()).RETURN(2);
    REQUIRE_CALL(transformer, transformShape(Shape({0, 0.5, 0.25}), 0ul)).RETURN(Shape({0, 0, 0.25}));
    REQUIRE_CALL(transformer, transformShape(Shape({0.5, 0.5, 0.25}), 0ul)).RETURN(Shape({0.5, 0, 0.25}));
    REQUIRE_CALL(transformer, transformShape(Shape({0, 0.5, 0.75}), 1ul)).RETURN(Shape({0, 0, 0.75}));
    REQUIRE_CALL(transformer, transformShape(Shape({0.5, 0.5, 0.75}), 1ul)).RETURN(Shape({0.5, 0, 0.75}));

    transformer.transform(lattice);

    auto expectedMolecules = std::vector<Shape>{Shape({0, 0, 0.25}), Shape({0.5, 0, 0.25}),
                                                Shape({0, 0, 0.75}), Shape({0.5, 0, 0.75})};
    CHECK_THAT(lattice.getCell(0, 0, 0).getMolecules(), Catch::UnorderedEquals(expectedMolecules));
    CHECK(lattice.getCellBox() == TriclinicBox(1));
    CHECK(lattice.getLatticeBox() == TriclinicBox(2));
    CHECK(lattice.getDimensions() == std::array<std::size_t, 3>{2, 2, 2});
}

TEST_CASE("LayerWiseTransformer: 3 layers in cell, 2 requested") {
    Lattice lattice(UnitCell(TriclinicBox(1), {Shape({0, 0.5, 0.25}), Shape({0, 0.5, 0.5}), Shape({0, 0.5, 0.75})}),
                    {2, 2, 2});
    MockLayerWiseTransformer transformer(LatticeTraits::Axis::Z);
    ALLOW_CALL(transformer, getRequestedNumOfLayers()).RETURN(2);
    REQUIRE_CALL(transformer, transformShape(Shape({0, 0.5, 0.125}), 0ul)).RETURN(Shape({0, 0, 0.125}));
    REQUIRE_CALL(transformer, transformShape(Shape({0, 0.5, 0.25}), 1ul)).RETURN(Shape({0, 0, 0.25}));
    REQUIRE_CALL(transformer, transformShape(Shape({0, 0.5, 0.375}), 0ul)).RETURN(Shape({0, 0, 0.375}));
    REQUIRE_CALL(transformer, transformShape(Shape({0, 0.5, 0.625}), 1ul)).RETURN(Shape({0, 0, 0.625}));
    REQUIRE_CALL(transformer, transformShape(Shape({0, 0.5, 0.75}), 0ul)).RETURN(Shape({0, 0, 0.75}));
    REQUIRE_CALL(transformer, transformShape(Shape({0, 0.5, 0.875}), 1ul)).RETURN(Shape({0, 0, 0.875}));

    transformer.transform(lattice);

    auto expectedMolecules = std::vector<Shape>{Shape({0, 0, 0.125}), Shape({0, 0, 0.25}),
                                                Shape({0, 0, 0.375}), Shape({0, 0, 0.625}),
                                                Shape({0, 0, 0.75}), Shape({0, 0, 0.875})};
    CHECK_THAT(lattice.getCell(0, 0, 0).getMolecules(), Catch::UnorderedEquals(expectedMolecules));
    CHECK(lattice.getCellBox() == TriclinicBox(std::array<double, 3>{1, 1, 2}));
    CHECK(lattice.getLatticeBox() == TriclinicBox(std::array<double, 3>{2, 2, 2}));
    CHECK(lattice.getDimensions() == std::array<std::size_t, 3>{2, 2, 1});
}