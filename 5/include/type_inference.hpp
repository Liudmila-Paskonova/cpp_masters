#pragma once
#include "ast.hpp"
#include "types.hpp"

struct InferState {
    TVarGenerator generator;
    TypeEnv environment;
    /// Init nil and cons at the beginning
    void InitBuiltins();
};

std::pair<Subst, TypePtr> InferExpr(InferState &state, const Expr &expr);

std::pair<Subst, TypePtr> InferSymbol(InferState &state, const Symbol &symbol);
std::pair<Subst, TypePtr> InferLambda(InferState &state, const List &list);
std::pair<Subst, TypePtr> InferDefine(InferState &state, const List &list);
std::pair<Subst, TypePtr> InferCase(InferState &state, const List &list);
std::pair<Subst, TypePtr> InferApplication(InferState &state, const List &list);
std::pair<Subst, TypePtr> InferList(InferState &state, const List &list);
