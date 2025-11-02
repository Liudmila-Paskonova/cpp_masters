#include "gtest/gtest.h"
#include "type_inference.hpp"
#include "parser.hpp"
#include "lexer.hpp"

class TypeInferenceTest : public ::testing::Test
{
  protected:
    InferState state;

    void
    SetUp() override
    {
        state.InitBuiltins();
    }

    TypePtr
    infer(const std::string &input)
    {
        Lexer lexer(input);
        auto tokens = lexer.Tokenize();
        Parser parser(tokens);
        auto expr = parser.Parse();

        auto [subst, type] = InferExpr(state, expr);
        return ApplySubstitution(subst, type);
    }

    TypePrinter printer;
};

TEST_F(TypeInferenceTest, InferSymbol)
{
    state.environment["x"] = Scheme{{}, Type::make_const("Int")};

    auto type = infer("x");
    EXPECT_EQ(printer.Print(type), "Int");
}

TEST_F(TypeInferenceTest, InferNil)
{
    auto type = infer("nil");
    EXPECT_TRUE(type->tag == TypeTag::TAPP);
    EXPECT_EQ(type->name, "List");
}

TEST_F(TypeInferenceTest, InferIdentityFunction)
{
    auto type = infer("(lambda (x) x)");
    EXPECT_EQ(type->tag, TypeTag::TARR);
    EXPECT_EQ(printer.Print(type->from), printer.Print(type->to));
}

TEST_F(TypeInferenceTest, InferDefine)
{
    infer("(define id (lambda (x) x))");

    auto it = state.environment.find("id");
    ASSERT_NE(it, state.environment.end());
    EXPECT_EQ(it->second.vars.size(), 1);
}
