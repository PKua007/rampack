//
// Created by Piotr Kubala on 30/01/2024.
//

#include <catch2/catch.hpp>

#include "core/ShapeData.h"


namespace {
    template <typename T>
    void check_that_contains_unmanaged_data(const ShapeData &shapeData, const T &theData) {
        CHECK_FALSE(shapeData.isManaged());
        CHECK(shapeData.getSize() == sizeof(T));
        CHECK(shapeData.raw() == reinterpret_cast<const std::byte *>(&theData));
        CHECK(shapeData.as<T>() == theData);
    }

    template <typename T>
    void check_that_contains_managed_data(const ShapeData &shapeData, const T &theData) {
        CHECK(shapeData.isManaged());
        CHECK(shapeData.getSize() == sizeof(T));
        CHECK(shapeData.raw() != reinterpret_cast<const std::byte *>(&theData));
        CHECK(shapeData.as<T>() == theData);
    }

    void check_that_is_empty(const ShapeData &shapeData) {
        CHECK_FALSE(shapeData.isManaged());
        CHECK(shapeData.getSize() == 0);
        CHECK(shapeData.raw() == nullptr);
    }
}


TEST_CASE("ShapeData: unmanaged construction") {
    SECTION("empty") {
        ShapeData shapeData;

        check_that_is_empty(shapeData);
    }

    SECTION("from bytes") {
        double data = 5;

        ShapeData shapeData(reinterpret_cast<const std::byte *>(&data), sizeof(data));

        check_that_contains_unmanaged_data(shapeData, data);
    }

    SECTION("from type") {
        double data = 5;

        ShapeData shapeData(data, false);

        check_that_contains_unmanaged_data(shapeData, data);
    }

    SECTION("operations") {
        double data = 5;
        ShapeData shapeData(reinterpret_cast<const std::byte *>(&data), sizeof(data));

        SECTION("prone to modification") {
            data = 7;

            CHECK(shapeData.as<double>() == 7);
        }

        SECTION("move construction") {
            ShapeData newShapeData(std::move(shapeData));

            check_that_is_empty(shapeData);
            check_that_contains_unmanaged_data(newShapeData, data);
        }

        SECTION("copy construction") {
            ShapeData newShapeData(shapeData);

            check_that_contains_unmanaged_data(shapeData, data);
            check_that_contains_managed_data(newShapeData, data);
        }

        SECTION("move assignment") {
            ShapeData newShapeData;

            newShapeData = std::move(shapeData);

            check_that_is_empty(shapeData);
            check_that_contains_unmanaged_data(newShapeData, data);
        }

        SECTION("copy assignment") {
            ShapeData newShapeData;

            newShapeData = shapeData;

            check_that_contains_unmanaged_data(shapeData, data);
            check_that_contains_managed_data(newShapeData, data);
        }

        SECTION("rvalue data assignment") {
            shapeData = double{7};
            check_that_contains_managed_data(shapeData, double{7});
        }

        SECTION("lvalue data assignment") {
            double newData = 8;
            shapeData = newData;
            check_that_contains_managed_data(shapeData, double{8});
        }
    }
}

TEST_CASE("ShapeData: managed construction") {
    SECTION("from bytes") {
        double data = 5;

        ShapeData shapeData(reinterpret_cast<const std::byte *>(&data), sizeof(data), true);

        check_that_contains_managed_data(shapeData, data);
    }

    SECTION("from type") {
        double data = 5;

        ShapeData shapeData(data);

        check_that_contains_managed_data(shapeData, data);
    }

    SECTION("operations") {
        double data = 5;
        ShapeData shapeData(data);

        SECTION("not prone to modification") {
            data = 7;

            CHECK(shapeData.as<double>() == 5);
        }

        SECTION("move construction") {
            ShapeData newShapeData(std::move(shapeData));

            check_that_is_empty(shapeData);
            check_that_contains_managed_data(newShapeData, data);
        }

        SECTION("copy construction") {
            ShapeData newShapeData(shapeData);

            check_that_contains_managed_data(shapeData, data);
            check_that_contains_managed_data(newShapeData, data);
        }

        SECTION("move assignment") {
            ShapeData newShapeData;

            newShapeData = std::move(shapeData);

            check_that_is_empty(shapeData);
            check_that_contains_managed_data(newShapeData, data);
        }

        SECTION("copy assignment") {
            ShapeData newShapeData;

            newShapeData = shapeData;

            check_that_contains_managed_data(shapeData, data);
            check_that_contains_managed_data(newShapeData, data);
        }

        SECTION("rvalue data assignment") {
            shapeData = double{7};
            check_that_contains_managed_data(shapeData, double{7});
        }

        SECTION("lvalue data assignment") {
            double newData = 8;
            shapeData = newData;
            check_that_contains_managed_data(shapeData, double{8});
        }
    }
}

TEST_CASE("ShapeData: equality") {
    double data1 = 5;
    int data2 = 5;
    ShapeData firstEqual(data1, true);
    ShapeData secondEqual(data1, false);
    ShapeData notEqual(data2);

    CHECK(firstEqual == secondEqual);
    CHECK_FALSE(firstEqual != secondEqual);
    CHECK_FALSE(firstEqual == notEqual);
    CHECK(firstEqual != notEqual);
}

TEST_CASE("ShapeData: exceptions") {
    SECTION("unmanaged from rvalue reference") {
        REQUIRE_THROWS_WITH(ShapeData(double{5}, false), Catch::Contains("unmanaged rvalue reference"));
    }

    SECTION("extracting too large type") {
        ShapeData uint16Data(std::uint16_t{5});

        REQUIRE_NOTHROW(uint16Data.as<std::uint8_t>());
        REQUIRE_THROWS(uint16Data.as<std::uint32_t>(), Catch::Contains("extracting too large type"));
    }
}