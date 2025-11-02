#include "gtest/gtest.h"
#include "type_inference.hpp"

class TypeTest : public ::testing::Test
{
  protected:
    TVarGenerator generator;
    TypePrinter printer;
};

TEST_F(TypeTest, TypeConstructors)
{
    auto var = Type::make_var(1);
    EXPECT_EQ(var->tag, TypeTag::TVAR);
    EXPECT_EQ(var->var_id, 1);

    auto constant = Type::make_const("Int");
    EXPECT_EQ(constant->tag, TypeTag::TCONST);
    EXPECT_EQ(constant->name, "Int");

    auto arrow = Type::make_arrow(var, constant);
    EXPECT_EQ(arrow->tag, TypeTag::TARR);
}

TEST_F(TypeTest, TypePrinter)
{
    auto var = Type::make_var(0);
    auto constant = Type::make_const("Int");
    auto arrow = Type::make_arrow(var, constant);

    EXPECT_EQ(printer.Print(var), "t0");
    EXPECT_EQ(printer.Print(constant), "Int");
    EXPECT_EQ(printer.Print(arrow), "(func t0 Int)");
}

TEST_F(TypeTest, ApplySubstitution)
{
    auto var1 = Type::make_var(1);
    auto var2 = Type::make_var(2);
    auto intType = Type::make_const("Int");

    Subst substitution = {{1, intType}};

    auto result1 = ApplySubstitution(substitution, var1);
    EXPECT_EQ(result1->tag, TypeTag::TCONST);
    EXPECT_EQ(result1->name, "Int");

    auto result2 = ApplySubstitution(substitution, var2);
    EXPECT_EQ(result2->tag, TypeTag::TVAR);
    EXPECT_EQ(result2->var_id, 2);
}

TEST_F(TypeTest, Compose)
{
    Subst s1 = {{1, Type::make_const("Int")}};
    Subst s2 = {{2, Type::make_var(1)}};

    auto composed = Compose(s2, s1);

    auto result = ApplySubstitution(composed, Type::make_var(2));
    EXPECT_EQ(result->tag, TypeTag::TCONST);
    EXPECT_EQ(result->name, "Int");
}
