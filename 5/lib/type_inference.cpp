#include "type_inference.hpp"

void
InferState::InitBuiltins()
{
    // nil : forall a. List[a]
    {
        int v = generator.Fresh();
        Scheme s;
        s.vars = {v};
        s.type = Type::make_app("List", {Type::make_var(v)});
        environment["nil"] = s;
    }

    // cons : forall a. a -> List[a] -> List[a]
    {
        int v = generator.Fresh();
        TypePtr a = Type::make_var(v);
        TypePtr cons_ty = Arrows({a, Type::make_app("List", {a})}, Type::make_app("List", {a}));
        Scheme s;
        s.vars = {v};
        s.type = cons_ty;
        environment["cons"] = s;
    }
}

std::pair<Subst, TypePtr>
InferExpr(InferState &state, const Expr &expr)
{
    if (std::holds_alternative<Number>(expr)) {
        return {Subst{}, Type::make_const("Number")};
    }
    if (std::holds_alternative<Nil>(expr)) {
        TypePtr t = Type::make_var(state.generator.Fresh());
        return {Subst{}, Type::make_app("List", {t})};
    }
    if (std::holds_alternative<Symbol>(expr)) {
        return InferSymbol(state, std::get<Symbol>(expr));
    }
    if (std::holds_alternative<List>(expr)) {
        return InferList(state, std::get<List>(expr));
    }
    throw std::runtime_error("Unknown expr kind");
}

std::pair<Subst, TypePtr>
InferSymbol(InferState &state, const Symbol &symbol)
{
    auto it = state.environment.find(symbol.name);
    if (it == state.environment.end()) {
        throw std::runtime_error("Unbound variable: " + symbol.name);
    }
    TypePtr type = Instantiate(it->second, state.generator);
    return {Subst{}, type};
}

std::pair<Subst, TypePtr>
InferLambda(InferState &state, const List &list)
{
    // (lambda (param1 param2 ...) body1 body2 ...)
    if (list.elements.size() < 3) {
        throw std::runtime_error("lambda requires parameters and a body expression");
    }
    // Parameters
    if (!std::holds_alternative<List>(list.elements[1])) {
        throw std::runtime_error("lambda parameters must be a list");
    }
    const List &params_list = std::get<List>(list.elements[1]);
    std::vector<std::string> param_names;
    std::vector<TypePtr> param_types;

    for (const auto &param_expr : params_list.elements) {
        if (!std::holds_alternative<Symbol>(param_expr)) {
            throw std::runtime_error("lambda parameter must be a symbol");
        }
        std::string param_name = std::get<Symbol>(param_expr).name;
        param_names.push_back(param_name);
        param_types.push_back(Type::make_var(state.generator.Fresh()));
    }

    // Save current environment
    TypeEnv saved_env = state.environment;

    // Extend environment with parameter types
    for (size_t i = 0; i < param_names.size(); ++i) {
        state.environment[param_names[i]] = Scheme{{}, param_types[i]};
    }

    // Infer types of body expressions
    Subst substitution;
    TypePtr body_type = nullptr;

    for (size_t i = 2; i < list.elements.size(); ++i) {
        auto [body_subst, inferred_type] = InferExpr(state, list.elements[i]);
        substitution = Compose(body_subst, substitution);
        body_type = ApplySubstitution(substitution, inferred_type);
    }

    // Restore original environment
    state.environment = saved_env;

    // Resulting function type: (param1_type -> param2_type -> ... -> body_type)
    TypePtr function_type = Arrows(param_types, body_type);

    return {substitution, ApplySubstitution(substitution, function_type)};
}

std::pair<Subst, TypePtr>
InferDefine(InferState &state, const List &list)
{
    // (define name expression)
    if (list.elements.size() != 3) {
        throw std::runtime_error("define requires exactly 2 arguments");
    }

    if (!std::holds_alternative<Symbol>(list.elements[1])) {
        throw std::runtime_error("define first argument must be a symbol");
    }

    std::string name = std::get<Symbol>(list.elements[1]).name;

    TypePtr recursive_ref = Type::make_var(state.generator.Fresh());

    // Save current environment
    TypeEnv saved_env = state.environment;

    state.environment[name] = Scheme{{}, recursive_ref};
    auto [expr_subst, expr_type] = InferExpr(state, list.elements[2]);
    TypePtr substituted = ApplySubstitution(expr_subst, recursive_ref);
    Subst unify_subst;

    try {
        unify_subst = Unify(substituted, expr_type);
    } catch (const std::exception &e) {
        state.environment = saved_env;
        throw std::runtime_error("In definition of " + name + ": " + e.what());
    }

    // Compose all substitutions
    Subst final_subst = Compose(unify_subst, expr_subst);

    // Apply final substitution to get the actual type
    TypePtr actual_type = ApplySubstitution(final_subst, expr_type);

    // Generalize the type
    Scheme generalized_scheme = Generalize(saved_env, actual_type);

    // Update environment with generalized type
    state.environment = saved_env;
    state.environment[name] = generalized_scheme;

    return {final_subst, generalized_scheme.type};
}

std::pair<Subst, TypePtr>
InferCase(InferState &state, const List &list)
{
    // (case expr (nil nil_expr) ((cons x xs) cons_expr))
    if (list.elements.size() != 4) {
        throw std::runtime_error("case requires exactly 3 arguments");
    }

    // Expression type
    auto [expr_subst, expr_type] = InferExpr(state, list.elements[1]);
    Subst substitution = expr_subst;
    TypePtr applied_expr_type = ApplySubstitution(substitution, expr_type);

    // Unify expr with List[a] for fresh a
    TypePtr elem_type = Type::make_var(state.generator.Fresh());
    TypePtr list_type = Type::make_app("List", {elem_type});

    Subst unify_subst1 = Unify(applied_expr_type, list_type);
    substitution = Compose(unify_subst1, substitution);

    // Apply updated substitution to element type
    elem_type = ApplySubstitution(substitution, elem_type);

    // Process nil branch
    if (!std::holds_alternative<List>(list.elements[2])) {
        throw std::runtime_error("nil branch must be a list");
    }

    const List &nil_branch = std::get<List>(list.elements[2]);
    if (nil_branch.elements.size() != 2) {
        throw std::runtime_error("nil branch must have 2 elements");
    }

    if (!std::holds_alternative<Symbol>(nil_branch.elements[0]) ||
        std::get<Symbol>(nil_branch.elements[0]).name != "nil") {
        throw std::runtime_error("nil branch must start with 'nil'");
    }

    auto [nil_subst, nil_type] = InferExpr(state, nil_branch.elements[1]);
    substitution = Compose(nil_subst, substitution);
    TypePtr nil_branch_type = ApplySubstitution(substitution, nil_type);

    // Process cons branch
    if (!std::holds_alternative<List>(list.elements[3])) {
        throw std::runtime_error("cons branch must be a list");
    }

    const List &cons_branch = std::get<List>(list.elements[3]);
    if (cons_branch.elements.size() != 2) {
        throw std::runtime_error("cons branch must have 2 elements");
    }

    if (!std::holds_alternative<List>(cons_branch.elements[0])) {
        throw std::runtime_error("cons pattern must be a list");
    }

    const List &cons_pattern = std::get<List>(cons_branch.elements[0]);
    if (cons_pattern.elements.size() != 3) {
        throw std::runtime_error("cons pattern must have 3 elements");
    }

    if (!std::holds_alternative<Symbol>(cons_pattern.elements[0]) ||
        std::get<Symbol>(cons_pattern.elements[0]).name != "cons") {
        throw std::runtime_error("cons pattern must start with 'cons'");
    }

    if (!std::holds_alternative<Symbol>(cons_pattern.elements[1]) ||
        !std::holds_alternative<Symbol>(cons_pattern.elements[2])) {
        throw std::runtime_error("cons pattern variables must be symbols");
    }

    std::string x_name = std::get<Symbol>(cons_pattern.elements[1]).name;
    std::string xs_name = std::get<Symbol>(cons_pattern.elements[2]).name;

    // Save environment
    TypeEnv saved_env = state.environment;

    // Extend environment with pattern variables
    state.environment[x_name] = Scheme{{}, elem_type};
    state.environment[xs_name] = Scheme{{}, Type::make_app("List", {elem_type})};

    // Infer cons branch body
    auto [cons_subst, cons_type] = InferExpr(state, cons_branch.elements[1]);
    substitution = Compose(cons_subst, substitution);
    TypePtr cons_branch_type = ApplySubstitution(substitution, cons_type);

    // Restore environment
    state.environment = saved_env;

    // Unify branch types
    Subst unify_subst2 = Unify(nil_branch_type, cons_branch_type);
    substitution = Compose(unify_subst2, substitution);

    TypePtr final_type = ApplySubstitution(substitution, nil_branch_type);
    return {substitution, final_type};
}

std::pair<Subst, TypePtr>
InferApplication(InferState &state, const List &list)
{
    // (function arg1 arg2 ...)

    // Infer function type
    auto [fun_subst, fun_type] = InferExpr(state, list.elements[0]);
    Subst substitution = fun_subst;
    TypePtr applied_fun_type = ApplySubstitution(substitution, fun_type);

    // Infer argument types
    std::vector<TypePtr> arg_types;
    for (size_t i = 1; i < list.elements.size(); ++i) {
        auto [arg_subst, arg_type] = InferExpr(state, list.elements[i]);
        substitution = Compose(arg_subst, substitution);
        TypePtr applied_arg_type = ApplySubstitution(substitution, arg_type);
        arg_types.push_back(applied_arg_type);
    }

    // Generate fresh type variable for result
    TypePtr result_type = Type::make_var(state.generator.Fresh());

    // Build expected function type
    TypePtr expected_fun_type = Arrows(arg_types, result_type);

    // Unify actual function type with expected type
    TypePtr applied_current_fun_type = ApplySubstitution(substitution, applied_fun_type);
    Subst unify_subst = Unify(applied_current_fun_type, expected_fun_type);

    // Compose substitutions and get final resulting type
    substitution = Compose(unify_subst, substitution);
    TypePtr final_result_type = ApplySubstitution(substitution, result_type);

    return {substitution, final_result_type};
}

std::pair<Subst, TypePtr>
InferList(InferState &state, const List &list)
{
    if (list.elements.empty()) {
        TypePtr elem_type = Type::make_var(state.generator.Fresh());
        return {Subst{}, Type::make_app("List", {elem_type})};
    }

    if (std::holds_alternative<Symbol>(list.elements[0])) {
        const Symbol &head = std::get<Symbol>(list.elements[0]);

        if (head.name == "lambda") {
            return InferLambda(state, list);
        } else if (head.name == "define") {
            return InferDefine(state, list);
        } else if (head.name == "case") {
            return InferCase(state, list);
        }
    }

    // (f arg1 arg2 ...)
    return InferApplication(state, list);
}
