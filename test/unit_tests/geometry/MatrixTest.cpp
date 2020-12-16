//
// Created by PKua on 21.09.18.
//

#include <iostream>
#include <catch2/catch.hpp>
#include "geometry/Matrix.h"
#include "geometry/Vector.h"

TEST_CASE("Matrix: constructors and generators") {
    SECTION("zero") {
        Matrix<2, 2> mat2x2;
        Matrix<2, 3> mat2x3;

        REQUIRE(Matrix<2, 2>{{0, 0, 0, 0}} == mat2x2);
        REQUIRE(Matrix<2, 3>{{0, 0, 0, 0, 0, 0}} == mat2x3);
    }

    SECTION("fill") {
        Matrix<2, 2> mat(4);

        REQUIRE(Matrix<2, 2>{{4, 4, 4, 4}} == mat);
    }

    SECTION("array") {
        Matrix<2, 2> mat = {{0, 1, 2, 3}};

        REQUIRE(0 == mat(0, 0));
        REQUIRE(1 == mat(0, 1));
        REQUIRE(2 == mat(1, 0));
        REQUIRE(3 == mat(1, 1));
    }

    SECTION("c-array") {
        double arr[] = {0, 1, 2, 3};
        Matrix<2, 2> mat(arr);

        REQUIRE(mat == Matrix<2, 2>{{0, 1, 2, 3}});
    }

    SECTION("2-dim c-array") {
        auto **arr = new double *[2];
        arr[0] = new double[2];
        arr[1] = new double[2];
        arr[0][0] = 0;
        arr[0][1] = 1;
        arr[1][0] = 2;
        arr[1][1] = 3;

        Matrix<2, 2> mat(arr);

        REQUIRE(mat == Matrix<2, 2>{{0, 1, 2, 3}});

        delete[] arr[0];
        delete[] arr[1];
        delete[] arr;
    }

    SECTION("identity") {
        auto mat2x2 = Matrix<2, 2>::identity();
        auto mat3x3 = Matrix<3, 3>::identity();

        REQUIRE(Matrix<2, 2>{{1, 0, 0, 1}} == mat2x2);
        REQUIRE(Matrix<3, 3>{{1, 0, 0, 0, 1, 0, 0, 0, 1}} == mat3x3);
    }

    SECTION("rotation2x2") {
        Matrix<2, 2> matRes = {{0.5, -0.8660254037844386, 0.8660254037844386, 0.5}};

        auto mat = Matrix<2, 2>::rotation(M_PI / 3);

        REQUIRE(mat(0, 0) == Approx(matRes(0, 0)));
        REQUIRE(mat(0, 1) == Approx(matRes(0, 1)));
        REQUIRE(mat(1, 0) == Approx(matRes(1, 0)));
        REQUIRE(mat(1, 1) == Approx(matRes(1, 1)));
    }

    SECTION("rotation3x3") {
        Matrix<3, 3> matRes = {{0.4330127018922193, 0.6495190528383290, 0.625, 0.75, 0.125, -0.6495190528383290, -0.5,
                                       0.75, -0.4330127018922193}};

        auto mat = Matrix<3, 3>::rotation(2 * M_PI / 3, M_PI / 6, M_PI / 3);

        for (std::size_t i = 0; i < 3; i++)
            for (std::size_t j = 0; j < 3; j++)
                REQUIRE(mat(i, j) == Approx(matRes(i, j)));
    }
}

TEST_CASE("Matrix: arithmetic") {
    SECTION("addition") {
        Matrix<2, 2> mat1 = {{1, 3, 5, 7}};
        Matrix<2, 2> mat2 = {{3, 5, 2, -1}};

        auto sum = mat1 + mat2;
        mat1 += mat2;

        REQUIRE(sum == Matrix<2, 2>{{4, 8, 7, 6}});
        REQUIRE(mat1 == Matrix<2, 2>{{4, 8, 7, 6}});
    }

    SECTION("subtraction") {
        Matrix<2, 2> mat1 = {{1, 3, 5, 7}};
        Matrix<2, 2> mat2 = {{0, 5, 2, -1}};

        auto diff = mat1 - mat2;
        mat1 -= mat2;

        REQUIRE(diff == Matrix<2, 2>{{1, -2, 3, 8}});
        REQUIRE(mat1 == Matrix<2, 2>{{1, -2, 3, 8}});
    }

    SECTION("multiplication by a constant") {
        Matrix<2, 2> mat = {{1, 3, 5, 7}};

        auto times1 = mat * 2.;
        auto times2 = 2. * mat;
        mat *= 2;

        REQUIRE(times1 == Matrix<2, 2>{{2, 6, 10, 14}});
        REQUIRE(times2 == Matrix<2, 2>{{2, 6, 10, 14}});
        REQUIRE(mat == Matrix<2, 2>{{2, 6, 10, 14}});
    }

    SECTION("division by a constant") {
        Matrix<2, 2> mat = {{2, 6, 10, 14}};

        auto div = mat / 2.;
        mat /= 2.;

        REQUIRE(div == Matrix<2, 2>{{1, 3, 5, 7}});
        REQUIRE(mat == Matrix<2, 2>{{1, 3, 5, 7}});
    }

    SECTION("matrix multiplication") {
        Matrix<2, 2> mat1 = {{1, -5, 10, 7}};
        Matrix<2, 2> mat2 = {{6, 0, 1, 3}};
        Matrix<2, 3> mat3 = {{5, 4, -4, 3, 0, 2}};

        auto res1 = mat1 * mat2;
        auto res2 = mat2 * mat3;
        mat1 *= mat2;

        REQUIRE(res1 == Matrix<2, 2>{{1, -15, 67, 21}});
        REQUIRE(res2 == Matrix<2, 3>{{30, 24, -24, 14, 4, 2}});
        REQUIRE(mat1 == Matrix<2, 2>{{1, -15, 67, 21}});
    }

    SECTION("unary minus") {
        Matrix<2, 2> mat = {{1, 2, 3, 4}};

        auto res = -mat;

        REQUIRE(res == Matrix<2, 2>{{-1, -2, -3, -4}});
    }
}

TEST_CASE("Matrix: determinant") {
    SECTION("1x1") {
        Matrix<1, 1> mat(1);

        REQUIRE(mat.det() == 1);
    }

    SECTION("2x2") {
        Matrix<2, 2> mat = {{1, 2, 3, 4}};

        REQUIRE(mat.det() == -2);
    }

    SECTION("3x3") {
        Matrix<3, 3> mat = {{1, 2, 3, 5, -6, 5, 4, 7, 0}};

        REQUIRE(mat.det() == 182);
    }

    SECTION("4x4") {
        Matrix<4, 4> mat = {{1, 2, 3, 5, -6, 5, 4, 7, 0, 0, 4, -6, 2, 4, 6, -2}};

        REQUIRE(mat.det() == -816);
    }
}

TEST_CASE("Matrix: inverse") {
    SECTION("1x1") {
        Matrix<1, 1> mat(1);
        Matrix<1, 1> real(1);

        REQUIRE(mat.inverse() == real);
    }

    SECTION("2x2") {
        Matrix<2, 2> mat = {{1, 2, 3, 4}};
        Matrix<2, 2> real = {{-2, 1, 1.5, -0.5}};

        REQUIRE(mat.inverse() == real);
    }

    SECTION("3x3") {
        Matrix<3, 3> mat = {{1, 2, 0, 5, -6, 5, 4, 7, 0}};
        Matrix<3, 3> real = {{-7, 0, 2, 4, 0, -1, 11.8, 0.2, -3.2}};

        REQUIRE(mat.inverse() == real);
    }

    SECTION("4x4") {
        Matrix<4, 4> mat = {{1, 2, 4, 5, -3, 5, 2, 1, 1, 0, 2, -6, 2, 4, 6, -2}};
        Matrix<4, 4> real = {{-0.416, -0.288, -0.584, 0.568, -0.456, -0.008, -0.544, 0.488, 0.472, 0.096, 0.528,
                                     -0.356, 0.088, -0.016, -0.088, -0.024}};

        REQUIRE(mat.inverse() == real);
    }

    SECTION("zero det") {
        Matrix<3, 3> mat = {{1, 2, 3, 4, 5, 6, 7, 8, 9}};

        REQUIRE_THROWS(mat.inverse());
    }
}

TEST_CASE("Matrix: getters & setters") {
    SECTION("range check") {
        Matrix<2, 3> mat;

        mat(0, 0);
        mat(0, 2);
        mat(1, 0);
        mat(1, 2);
        REQUIRE_THROWS(mat(0, 3));
        REQUIRE_THROWS(mat(2, 0));
        REQUIRE_THROWS(mat(1, 3));
        REQUIRE_THROWS(mat(2, 2));
    }

    SECTION("const range check") {
        const Matrix<2, 3> mat;

        mat(0, 0);
        mat(0, 2);
        mat(1, 0);
        mat(1, 2);
        REQUIRE_THROWS(mat(0, 3));
        REQUIRE_THROWS(mat(2, 0));
        REQUIRE_THROWS(mat(1, 3));
        REQUIRE_THROWS(mat(2, 2));
    }

    SECTION("element write") {
        Matrix<2, 2> mat;
        Matrix<2, 2> real = {{1, 2, 3, 4}};

        mat(0, 0) = 1;
        mat(0, 1) = 2;
        mat(1, 0) = 3;
        mat(1, 1) = 4;

        REQUIRE(mat == real);
    }

    SECTION("array output") {
        Matrix<2, 2> mat = {{0, 1, 2, 3}};
        double arr[4];

        mat.copyToArray(arr);

        REQUIRE(arr[0] == 0);
        REQUIRE(arr[1] == 1);
        REQUIRE(arr[2] == 2);
        REQUIRE(arr[3] == 3);
    }

    SECTION("getRows & getCols") {
        Matrix<2, 3> mat;

        REQUIRE(mat.getRows() == 2);
        REQUIRE(mat.getCols() == 3);
    }

    SECTION("row & column") {
        Matrix<2, 2> mat = {{1, 2, 3, 4}};

        REQUIRE(mat.row(0) == Vector<2>{{1, 2}});
        REQUIRE(mat.row(1) == Vector<2>{{3, 4}});
        REQUIRE(mat.column(0) == Vector<2>{{1, 3}});
        REQUIRE(mat.column(1) == Vector<2>{{2, 4}});
    }
}

TEST_CASE("Matrix: transpose") {
    SECTION("2x2") {
        Matrix<2, 2> mat = {{1, 2, 3, 4}};

        REQUIRE(mat.transpose() == Matrix<2, 2>{{1, 3, 2, 4}});
    }

    SECTION("2x3") {
        Matrix<2, 3> mat = {{1, 2, 3, 4, 5, 6}};

        REQUIRE(mat.transpose() == Matrix<3, 2>{{1, 4, 2, 5, 3, 6}});
    }
}

TEST_CASE("Matrix: relations") {
    SECTION("equality") {
        Matrix<2, 2> mat1 = {{1, 2, 3, 4}};
        Matrix<2, 2> mat2 = {{1, 2, 3, 4}};

        REQUIRE(mat1 == mat2);
    }

    SECTION("inequality") {
        Matrix<2, 2> mat1 = {{1, 2, 3, 4}};
        Matrix<2, 2> mat2 = {{1, 5, 3, 4}};

        REQUIRE(mat1 != mat2);
    }
}

TEST_CASE("Matrix: output") {
    SECTION("toString") {
        Matrix<2, 2> mat = {{1, 2, 3, 4}};

        REQUIRE(mat.toString() == "1 2 \n3 4 \n");
    }

    SECTION("stream insertion") {
        Matrix<2, 2> mat = {{1, 2, 3, 4}};
        std::ostringstream ostr;

        ostr << mat;

        REQUIRE(ostr.str() == "1 2 \n3 4 \n");
    }
}