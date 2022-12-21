//
// Created by PKua on 22.09.18.
//

#include <catch2/catch.hpp>
#include "geometry/Vector.h"

TEST_CASE("Vector: constructors") {
    SECTION("zero") {
        Vector<2> vec;

        REQUIRE(vec == Vector<2>{{0, 0}});
    }

    SECTION("fill") {
        Vector<2> vec(5);

        REQUIRE(vec == Vector<2>{{5, 5}});
    }

    SECTION("array") {
        Vector<2> vec = {{1, 2}};

        REQUIRE(vec[0] == 1);
        REQUIRE(vec[1] == 2);
    }

    SECTION("c-array") {
        double array[] = {1, 2};

        Vector<2> vec(array);

        REQUIRE(vec == Vector<2>{{1, 2}});
    }
}

TEST_CASE("Vector: arithmetic") {
    SECTION("addition") {
        Vector<2> vec1{{1, 2}};
        Vector<2> vec2{{3, 4}};

        auto sum = vec1 + vec2;
        vec1 += vec2;

        REQUIRE(sum == Vector<2>{{4, 6}});
        REQUIRE(vec1 == Vector<2>{{4, 6}});
    }

    SECTION("subtraction") {
        Vector<2> vec1{{1, 2}};
        Vector<2> vec2{{3, 5}};

        auto diff = vec1 - vec2;
        vec1 -= vec2;

        REQUIRE(diff == Vector<2>{{-2, -3}});
        REQUIRE(vec1 == Vector<2>{{-2, -3}});
    }

    SECTION("multiplication by a constant") {
        Vector<2> vec{{1, 2}};

        auto res1 = vec * 2.;
        auto res2 = 2. * vec;
        auto res3 = vec * 2;
        auto res4 = 2 * vec;
        vec *= 2.;

        REQUIRE(res1 == Vector<2>{{2, 4}});
        REQUIRE(res2 == Vector<2>{{2, 4}});
        REQUIRE(res3 == Vector<2>{{2, 4}});
        REQUIRE(res4 == Vector<2>{{2, 4}});
        REQUIRE(vec == Vector<2>{{2, 4}});
    }

    SECTION("division by a constant") {
        Vector<2> vec{{2, 4}};

        auto res1 = vec / 2.;
        auto res2 = vec / 2;
        vec /= 2.;

        REQUIRE(res1 == Vector<2>{{1, 2}});
        REQUIRE(res2 == Vector<2>{{1, 2}});
        REQUIRE(vec == Vector<2>{{1, 2}});
    }

    SECTION("unary minus") {
        Vector<2> vec = {{1, 2}};

        auto res = -vec;

        REQUIRE(res == Vector<2>{{-1, -2}});
    }
}

TEST_CASE("Vector: vector operations") {
    SECTION("scalar product") {
        Vector<2> vec1 = {{1, 2}};
        Vector<2> vec2 = {{3, 4}};

        double dot = vec1 * vec2;

        REQUIRE(dot == 11);
    }

    SECTION("vector product") {
        Vector<3> vec1 = {{1, 2, 3}};
        Vector<3> vec2 = {{4, 5, 6}};

        auto cross = vec1 ^vec2;

        REQUIRE(cross == Vector<3>{{-3, 6, -3}});
    }

    SECTION("linear transformation") {
        Vector<2> vec = {{1, 3}};
        Matrix<3, 2> mat = {{1, 2, 4, -2, 3, 0}};

        auto res = mat * vec;

        REQUIRE(res == Vector<3>{{7, -2, 3}});
    }

    SECTION("projection") {
        Vector<2> vec = {{1, 1}};
        Vector<2> axis = {{3, 4}};

        auto proj = vec.projectOn(axis);

        REQUIRE(proj[0] == Approx(0.84));
        REQUIRE(proj[1] == Approx(1.12));
    }
}

TEST_CASE("Vector: relations") {
    SECTION("equality") {
        Vector<2> vec1 = {{1, 2}};
        Vector<2> vec2 = {{1, 2}};

        REQUIRE(vec1 == vec2);
    }

    SECTION("inequality") {
        Vector<2> vec1 = {{1, 2}};
        Vector<2> vec2 = {{3, 2}};

        REQUIRE(vec1 != vec2);
    }
}

TEST_CASE("Vector: normalization") {
    SECTION("norm") {
        Vector<2> vec{{3, 4}};

        double norm = vec.norm();

        REQUIRE(norm == 5);
    }

    SECTION("norm2") {
        Vector<2> vec{{3, 4}};

        double norm2 = vec.norm2();

        REQUIRE(norm2 == 25);
    }

    SECTION("normalized") {
        Vector<2> vec = {{3, 4}};

        auto normalized = vec.normalized();

        REQUIRE(normalized == Vector<2>{{0.6, 0.8}});
    }
}

TEST_CASE("Vector: gettest & setters") {
    SECTION("range check") {
        Vector<2> vec;

        vec[0];
        vec[1];
        REQUIRE_THROWS(vec[2]);
    }

    SECTION("const range check") {
        const Vector<2> vec;

        vec[0];
        vec[1];
        REQUIRE_THROWS(vec[2]);
    }

    SECTION("element write") {
        Vector<2> vec;

        vec[0] = 1;
        vec[1] = 2;

        REQUIRE(vec == Vector<2>{{1, 2}});
    }

    SECTION("array output") {
        Vector<2> vec = {{1, 2}};

        double array[2];
        vec.copyToArray(array);

        REQUIRE(array[0] == 1);
        REQUIRE(array[1] == 2);
    }

    SECTION("getDimension") {
        Vector<2> vec;

        REQUIRE(vec.getDimension() == 2);
    }
}

TEST_CASE("Vector: output") {
    SECTION("toString") {
        Vector<2> vec = {{1, 2}};

        REQUIRE(vec.toString() == "{1, 2}");
    }

    SECTION("stream insertion") {
        Vector<2> vec = {{1, 2}};

        std::ostringstream ostr;
        ostr << vec;

        REQUIRE(ostr.str() == "{1, 2}");
    }
}