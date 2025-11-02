#include "parser.hpp"
#include <stdexcept>

Parser::Parser(const std::vector<Token> &tokens) : tokens_(tokens), pos_(0) {}

Expr
Parser::Parse()
{
    if (Match(TokenType::LPAREN)) {
        return ParseList();
    } else if (Match(TokenType::SYMBOL)) {
        return ParseSymbol();
    } else if (Match(TokenType::NUMBER)) {
        return ParseNumber();
    } else {
        throw std::runtime_error("Unexpected token at Parse()");
    }
}

bool
Parser::IsAtEnd() const
{
    return pos_ >= tokens_.size();
}

const Token &
Parser::Peek() const
{
    return tokens_[pos_];
}

const Token &
Parser::ReadChar()
{
    return tokens_[pos_++];
}

bool
Parser::Check(TokenType type) const
{
    return !IsAtEnd() && Peek().type == type;
}

bool
Parser::Match(TokenType type)
{
    if (Check(type)) {
        ReadChar();
        return true;
    }
    return false;
}

Expr
Parser::ParseList()
{
    List list;
    if (Match(TokenType::RPAREN)) {
        // empty list -> Nil
        return Nil{};
    }
    while (!Check(TokenType::RPAREN) && !IsAtEnd()) {
        list.elements.push_back(Parse());
    }
    if (!Match(TokenType::RPAREN)) {
        throw std::runtime_error("Expected ')'");
    }
    return list;
}

Expr
Parser::ParseSymbol()
{
    const Token &token = tokens_[pos_ - 1];
    return Symbol{token.value};
}

Expr
Parser::ParseNumber()
{
    const Token &token = tokens_[pos_ - 1];
    try {
        return Number{std::stod(token.value)};
    } catch (...) {
        throw std::runtime_error(std::string("Invalid number: ") + token.value);
    }
}
