#include <gtest/gtest.h>

#include "matrix/matrix.hpp"
#include "nth_power/nth_power.hpp"

TEST(SquareMatrixBasics, DefaultIsZero)
{
    matrix::SquareMatrix<int, 2> m;
    EXPECT_EQ(m(0, 0), 0);
    EXPECT_EQ(m(0, 1), 0);
    EXPECT_EQ(m(1, 0), 0);
    EXPECT_EQ(m(1, 1), 0);
}

TEST(SquareMatrixBasics, IdentityCtor)
{
    matrix::SquareMatrix<int, 3> id(1);
    for (size_t i = 0; i < 3; ++i) {
        for (size_t j = 0; j < 3; ++j) {
            EXPECT_EQ(id(i, j), (i == j ? 1 : 0));
        }
    }
}

TEST(SquareMatrixOps, Addition)
{
    matrix::SquareMatrix<int, 2> a{{{1, 2}, {3, 4}}};
    matrix::SquareMatrix<int, 2> b{{{5, 6}, {7, 8}}};
    a += b;
    EXPECT_EQ(a, (matrix::SquareMatrix<int, 2>{{{6, 8}, {10, 12}}}));
}

TEST(SquareMatrixOps, Multiplication)
{
    matrix::SquareMatrix<int, 2> a{{{1, 2}, {3, 4}}};
    matrix::SquareMatrix<int, 2> b{{{2, 0}, {1, 2}}};
    a *= b;
    EXPECT_EQ(a, (matrix::SquareMatrix<int, 2>{{{4, 4}, {10, 8}}}));
}

TEST(NthPowerScalar, IntPowers)
{
    EXPECT_EQ(nth_power::nth_power(2, 10u), 1024);
    EXPECT_EQ(nth_power::nth_power(3, 0u), 1);
    EXPECT_EQ(nth_power::nth_power(5, 1u), 5);
}

TEST(NthPowerMatrix, MatrixPower)
{
    matrix::SquareMatrix<int, 2> a{{{1, 2}, {3, 4}}};
    auto a2 = nth_power::nth_power(a, 2);
    EXPECT_EQ(a2, (matrix::SquareMatrix<int, 2>{{{7, 10}, {15, 22}}}));

    matrix::SquareMatrix<int, 2> id(1);
    auto id0 = nth_power::nth_power(id, 0);
    EXPECT_EQ(id0, (matrix::SquareMatrix<int, 2>{{{1, 0}, {0, 1}}}));
    auto id3 = nth_power::nth_power(id, 3);
    EXPECT_EQ(id3, id);
}
