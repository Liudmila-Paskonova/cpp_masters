#pragma once
#include <variant>
#include <vector>
#include <string>

struct Symbol;
struct Number;
struct List;
struct Nil;

using Expr = std::variant<Symbol, Number, List, Nil>;

struct Symbol {
    std::string name;
};

struct Number {
    double value;
};

struct List {
    std::vector<Expr> elements;
};

struct Nil {
};
