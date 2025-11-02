#pragma once
#include "tokens.hpp"
#include <vector>
#include <string>

class Lexer
{
  public:
    Lexer(const std::string &input);
    std::vector<Token> Tokenize();

  private:
    std::string input_;
    size_t pos_;
    size_t line_;
    size_t column_;

    bool IsAtEnd() const;
    char ReadChar();
    char Peek() const;
    void SkipWhitespace();
    void SkipComment();
    Token ReadNumber(char first);
    Token ReadSymbol(char first);
};
