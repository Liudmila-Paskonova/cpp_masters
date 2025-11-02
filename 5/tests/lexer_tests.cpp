#include "gtest/gtest.h"
#include <string>
#include "lexer.hpp"

class LexerTest : public ::testing::Test
{
  protected:
    std::vector<Token>
    tokenize(const std::string &input)
    {
        Lexer lexer(input);
        return lexer.Tokenize();
    }
};

TEST_F(LexerTest, BasicTokens)
{
    auto tokens = tokenize("( ) abc");
    ASSERT_EQ(tokens.size(), 3);
    EXPECT_EQ(tokens[0].type, TokenType::LPAREN);
    EXPECT_EQ(tokens[1].type, TokenType::RPAREN);
    EXPECT_EQ(tokens[2].type, TokenType::SYMBOL);
    EXPECT_EQ(tokens[2].value, "abc");
}

TEST_F(LexerTest, SExpression)
{
    std::string input = "(print (add 1 2))";

    auto tokens = tokenize(input);
    ASSERT_EQ(tokens.size(), 8);

    std::vector<std::pair<TokenType, std::string>> expected = {
        {TokenType::LPAREN, "("}, {TokenType::SYMBOL, "print"}, {TokenType::LPAREN, "("}, {TokenType::SYMBOL, "add"},
        {TokenType::NUMBER, "1"}, {TokenType::NUMBER, "2"},     {TokenType::RPAREN, ")"}, {TokenType::RPAREN, ")"}};

    for (size_t i = 0; i < tokens.size(); ++i) {
        EXPECT_EQ(tokens[i].type, expected[i].first);
        EXPECT_EQ(tokens[i].value, expected[i].second);
    }
}
