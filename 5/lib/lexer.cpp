#include "lexer.hpp"
#include <vector>
#include <string>

Lexer::Lexer(const std::string &input) : input_(input), pos_(0), line_(1), column_(1) {}

std::vector<Token>
Lexer::Tokenize()
{
    std::vector<Token> tokens;
    while (!IsAtEnd()) {
        SkipWhitespace();
        if (IsAtEnd()) {
            break;
        }
        char c = ReadChar();
        switch (c) {
        case '(':
            tokens.push_back({TokenType::LPAREN, "(", line_, column_ - 1});
            break;
        case ')':
            tokens.push_back({TokenType::RPAREN, ")", line_, column_ - 1});
            break;
        case ';':
            SkipComment();
            break;
        default:
            if (isdigit(c) || c == '-' || c == '+') {
                tokens.push_back(ReadNumber(c));
            } else {
                tokens.push_back(ReadSymbol(c));
            }
            break;
        }
    }
    return tokens;
}

bool
Lexer::IsAtEnd() const
{
    return pos_ >= input_.length();
}

char
Lexer::ReadChar()
{
    char c = input_[pos_++];
    if (c == '\n') {
        line_++;
        column_ = 1;
    } else {
        column_++;
    }
    return c;
}

char
Lexer::Peek() const
{
    return IsAtEnd() ? '\0' : input_[pos_];
}

void
Lexer::SkipWhitespace()
{
    while (!IsAtEnd() && isspace(Peek())) {
        ReadChar();
    }
}

void
Lexer::SkipComment()
{
    while (!IsAtEnd() && Peek() != '\n') {
        ReadChar();
    }
}

Token
Lexer::ReadNumber(char first)
{
    size_t start_line = line_;
    size_t start_column = column_ - 1;
    std::string value;
    value += first;
    bool has_decimal = (first == '.');
    while (!IsAtEnd()) {
        char c = Peek();
        if (c == '.' && !has_decimal) {
            has_decimal = true;
            value += ReadChar();
        } else if (isdigit(c)) {
            value += ReadChar();
        } else {
            break;
        }
    }
    return {TokenType::NUMBER, value, start_line, start_column};
}

Token
Lexer::ReadSymbol(char first)
{
    size_t start_line = line_;
    size_t start_column = column_ - 1;
    std::string value;
    value += first;
    while (!IsAtEnd()) {
        char c = Peek();
        if (c == '(' || c == ')' || isspace(c) || c == ';') {
            break;
        }
        value += ReadChar();
    }
    return {TokenType::SYMBOL, value, start_line, start_column};
}
