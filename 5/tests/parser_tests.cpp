#include "gtest/gtest.h"
#include <string>
#include "parser.hpp"
#include "lexer.hpp"

class ParserTest : public ::testing::Test
{
  protected:
    Expr
    parse(const std::string &input)
    {
        Lexer lexer(input);
        auto tokens = lexer.Tokenize();
        Parser parser(tokens);
        return parser.Parse();
    }
};

TEST_F(ParserTest, Symbol)
{
    auto expr = parse("x");
    ASSERT_TRUE(std::holds_alternative<Symbol>(expr));
    EXPECT_EQ(std::get<Symbol>(expr).name, "x");
}

TEST_F(ParserTest, Nil)
{
    auto expr = parse("()");
    ASSERT_TRUE(std::holds_alternative<Nil>(expr));
}

TEST_F(ParserTest, List)
{
    auto expr = parse("(cons b c)");
    ASSERT_TRUE(std::holds_alternative<List>(expr));
    auto &list = std::get<List>(expr);
    ASSERT_EQ(list.elements.size(), 3);

    EXPECT_TRUE(std::holds_alternative<Symbol>(list.elements[0]));
    EXPECT_TRUE(std::holds_alternative<Symbol>(list.elements[1]));
    EXPECT_TRUE(std::holds_alternative<Symbol>(list.elements[2]));
}
