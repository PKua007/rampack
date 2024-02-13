//
// Created by pkua on 22.11.22.
//

#include "catch2/catch.hpp"

#include "mocks/MockShapeTraits.h"
#include "matchers/PackingApproxMatcher.h"

#include "core/io/RamsnapReader.h"
#include "core/io/RamsnapWriter.h"
#include "core/PeriodicBoundaryConditions.h"
#include "core/shapes/SphereTraits.h"
#include "core/io/ShapeDataSerializer.h"
#include "core/io/ShapeDataDeserializer.h"


namespace {
    struct TheData {
        int i;
        double d;
    };

    TextualShapeData do_serialize(const ShapeData &data) {
        const auto &theData = data.as<TheData>();
        ShapeDataSerializer serializer;
        serializer["i"] = theData.i;
        serializer["d"] = theData.d;
        return serializer.toTextualShapeData();
    }

    ShapeData do_deserialize(const TextualShapeData &textData) {
        ShapeDataDeserializer deserializer(textData);
        TheData theData{deserializer.as<int>("i"), deserializer.as<double>("d")};
        deserializer.throwIfNotAccessed();
        return ShapeData(theData);
    }

    auto get_rid_of_shape_data(MockShapeTraits &traits, Packing &packing) {
        using trompeloeil::_;

        // Use NAMED_ALLOW_CALL to extend expectations lifetime
        auto lifetimeGuard1 = NAMED_ALLOW_CALL(traits, serialize(_)).RETURN(TextualShapeData{});
        auto lifetimeGuard2 = NAMED_ALLOW_CALL(traits, deserialize(_)).RETURN(ShapeData{});
        auto lifetimeGuard3 = NAMED_ALLOW_CALL(traits, getShapeDataSize()).RETURN(0);

        std::vector<Shape> shapes(packing.begin(), packing.end());
        for (auto &shape: shapes)
            shape.setData(ShapeData{});

        packing.reset(std::move(shapes), packing.getBox(), traits.getInteraction(), traits.getDataManager());

        // Return expectation lifetime guards to extend expectations lifetime outside the function
        return std::make_tuple(std::move(lifetimeGuard1), std::move(lifetimeGuard2), std::move(lifetimeGuard3));
    }
}

#define GET_RID_OF_SHAPE_DATA(traits, packing) \
    [[maybe_unused]] auto lifetimeGuard = get_rid_of_shape_data(traits, packing)


TEST_CASE("RamsnapReader and RamsnapWriter") {
    using trompeloeil::_;

    MockShapeTraits traits;
    ALLOW_CALL(traits, getShapeDataSize()).RETURN(sizeof(TheData));
    ALLOW_CALL(traits, getInteractionCentres(_)).RETURN(std::vector<Vector<3>>{});
    ALLOW_CALL(traits, validateShapeData(_));
    ALLOW_CALL(traits, getRangeRadius(_)).RETURN(1);
    ALLOW_CALL(traits, getTotalRangeRadius(_)).RETURN(1);
    ALLOW_CALL(traits, serialize(_)).RETURN(do_serialize(_1));
    ALLOW_CALL(traits, deserialize(_)).RETURN(do_deserialize(_1));

    auto pbc = std::make_unique<PeriodicBoundaryConditions>();
    std::vector<Shape> shapes;
    shapes.emplace_back(Vector<3>{0.5, 0.5, 0.5}, Matrix<3, 3>{1, 0, 0, 0, 1, 0, 0, 0, 1}, TheData{1, 2.5});
    shapes.emplace_back(Vector<3>{4.5, 0.5, 0.5}, Matrix<3, 3>{0, 1, 0, 0, 0, 1, 1, 0, 0}, TheData{3, 4.5});
    shapes.emplace_back(Vector<3>{2.5, 2.5, 4.0}, Matrix<3, 3>{0, 0, 1, 1, 0, 0, 0, 1, 0}, TheData{5, 6.5});
    Packing expectedPacking({5, 5, 5}, std::move(shapes), std::move(pbc), traits.getInteraction(), traits.getDataManager());

    std::map<std::string, std::string> expectedAuxInfo = {{"key1", "value 1"}, {"key2", "value 2"}};

    auto pbc2 = std::make_unique<PeriodicBoundaryConditions>();
    Packing restoredPacking(std::move(pbc2));

    SECTION("[curr format] RamsnapWriter -> RamsnapReader") {
        std::stringstream inOut;
        RamsnapWriter writer;
        RamsnapReader reader;

        writer.write(inOut, expectedPacking, traits, expectedAuxInfo);
        auto restoredAuxInfo = reader.read(inOut, restoredPacking, traits);

        CHECK(restoredAuxInfo == expectedAuxInfo);
        CHECK_THAT(restoredPacking, IsApproxEqual(expectedPacking, 1e-12));
    }

    SECTION("[old format] orthorhombic box -> RamsnapReader") {
        GET_RID_OF_SHAPE_DATA(traits, expectedPacking);
        RamsnapReader reader;
        std::istringstream in(R"(2
key1 value 1
key2 value 2
5 5 5
3
0.5 0.5 0.5        1 0 0 0 1 0 0 0 1
4.5 0.5 0.5        0 1 0 0 0 1 1 0 0
2.5 2.5 4        0 0 1 1 0 0 0 1 0)");

        auto restoredAuxInfo = reader.read(in, restoredPacking, traits);

        CHECK(restoredAuxInfo == expectedAuxInfo);
        CHECK_THAT(restoredPacking, IsApproxEqual(expectedPacking, 1e-12));
    }

    SECTION("[old format] triclinic box -> RamsnapReader") {
        GET_RID_OF_SHAPE_DATA(traits, expectedPacking);
        RamsnapReader reader;
        std::istringstream in(R"(2
key1 value 1
key2 value 2
5 0 0 0 5 0 0 0 5
3
0.5 0.5 0.5        1 0 0 0 1 0 0 0 1
4.5 0.5 0.5        0 1 0 0 0 1 1 0 0
2.5 2.5 4        0 0 1 1 0 0 0 1 0)");

        auto restoredAuxInfo = reader.read(in, restoredPacking, traits);

        CHECK(restoredAuxInfo == expectedAuxInfo);
        CHECK_THAT(restoredPacking, IsApproxEqual(expectedPacking, 1e-12));
    }
}