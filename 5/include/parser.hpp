#pragma once
#include "ast.hpp"
#include "tokens.hpp"
#include <vector>

class Parser
{

  public:
    Parser(const std::vector<Token> &tokens);
    Expr Parse();

  private:
    const std::vector<Token> &tokens_;
    size_t pos_;

    bool IsAtEnd() const;
    const Token &Peek() const;
    const Token &ReadChar();
    bool Check(TokenType type) const;
    bool Match(TokenType type);
    Expr ParseList();
    Expr ParseSymbol();
    Expr ParseNumber();
};
