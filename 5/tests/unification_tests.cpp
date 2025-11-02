#include "gtest/gtest.h"
#include "type_inference.hpp"

class UnificationTest : public ::testing::Test
{
  protected:
    TVarGenerator gen;
};

TEST_F(UnificationTest, SameType)
{
    auto t1 = Type::make_const("Int");
    auto t2 = Type::make_const("Int");

    EXPECT_NO_THROW(Unify(t1, t2));
}

TEST_F(UnificationTest, DifferentConstants)
{
    auto t1 = Type::make_const("Int");
    auto t2 = Type::make_const("Bool");

    EXPECT_THROW(Unify(t1, t2), std::runtime_error);
}

TEST_F(UnificationTest, VariableWithType)
{
    auto var = Type::make_var(1);
    auto intType = Type::make_const("Int");

    auto substitution = Unify(var, intType);
    EXPECT_EQ(substitution.size(), 1);
    EXPECT_EQ(substitution[1]->name, "Int");
}

TEST_F(UnificationTest, OccursCheck)
{
    auto var = Type::make_var(1);
    auto arrow = Type::make_arrow(Type::make_const("Int"), var);

    EXPECT_THROW(Unify(var, arrow), std::runtime_error);
}
