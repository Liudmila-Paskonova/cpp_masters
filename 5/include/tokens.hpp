#pragma once
#include <string>
#include <cstddef>

enum class TokenType { LPAREN, RPAREN, SYMBOL, NUMBER, STRING, END };

struct Token {
    TokenType type;
    std::string value;
    size_t line;
    size_t column;
};
