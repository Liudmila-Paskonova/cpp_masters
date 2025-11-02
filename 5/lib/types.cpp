#include "types.hpp"

#include <cstddef>
#include <sstream>

Type::Type(int id) : tag(TypeTag::TVAR), var_id(id) {}
Type::Type(const std::string &c) : tag(TypeTag::TCONST), name(c) {}
Type::Type(const std::string &c, const std::vector<TypePtr> &a) : tag(TypeTag::TAPP), name(c), args(a) {}
Type::Type(TypePtr f, TypePtr t) : tag(TypeTag::TARR), from(f), to(t) {}

TypePtr
Type::make_var(int id)
{
    return std::make_shared<Type>(id);
}
TypePtr
Type::make_const(const std::string &n)
{
    return std::make_shared<Type>(n);
}
TypePtr
Type::make_app(const std::string &n, const std::vector<TypePtr> &a)
{
    return std::make_shared<Type>(n, a);
}
TypePtr
Type::make_arrow(TypePtr a, TypePtr b)
{
    return std::make_shared<Type>(a, b);
}

int
TVarGenerator::Fresh()
{
    return next++;
}

std::string
TypePrinter::Print(const TypePtr &t)
{
    std::ostringstream os;
    PrintImpl(t, os);
    return os.str();
}

void
TypePrinter::PrintImpl(const TypePtr &t, std::ostringstream &os)
{
    switch (t->tag) {
    case TypeTag::TVAR:
        os << "t" << t->var_id;
        break;
    case TypeTag::TCONST:
        os << t->name;
        break;
    case TypeTag::TAPP:
        if (t->name == "List" && t->args.size() == 1) {
            os << "(list ";
            PrintImpl(t->args[0], os);
            os << ")";
        } else {
            os << "(" << t->name;
            for (size_t i = 0; i < t->args.size(); ++i) {
                os << " ";
                PrintImpl(t->args[i], os);
            }
            os << ")";
        }
        break;
    case TypeTag::TARR:
        if ((t->from)->tag == TypeTag::TVAR || (t->to)->tag == TypeTag::TVAR) {
            os << "(func ";
            PrintImpl(t->from, os);
            os << " ";
            PrintImpl(t->to, os);
            os << ")";
        } else {
            PrintImpl(t->from, os);
            os << " ";
            PrintImpl(t->to, os);
        }
        break;
    }
}

std::string
TypePrinter::PrintScheme(const Scheme &scheme)
{
    std::ostringstream os;

    if (scheme.vars.empty()) {
        // No quantification, just print the type
        os << Print(scheme.type);
    } else {
        // Print with forall quantification
        os << "(forall (";
        for (size_t i = 0; i < scheme.vars.size(); ++i) {
            if (i > 0) {
                os << " ";
            }
            os << "t" << scheme.vars[i];
        }
        os << ") (";
        os << Print(scheme.type);
        os << "))";
    }
    return os.str();
}

TypePtr
ApplySubstitution(const Subst &substitution, const TypePtr &type)
{
    if (type->tag == TypeTag::TVAR) {
        auto it = substitution.find(type->var_id);
        if (it != substitution.end()) {
            return ApplySubstitution(substitution, it->second);
        }
        return type;
    } else if (type->tag == TypeTag::TAPP) {
        std::vector<TypePtr> new_args;
        new_args.reserve(type->args.size());
        for (auto &t : type->args) {
            new_args.push_back(ApplySubstitution(substitution, t));
        }
        return Type::make_app(type->name, new_args);
    } else if (type->tag == TypeTag::TARR) {
        return Type::make_arrow(ApplySubstitution(substitution, type->from), ApplySubstitution(substitution, type->to));
    } else {
        // TCONST
        return type;
    }
}

Subst
Compose(const Subst &substitution2, const Subst &substitution1)
{
    Subst s = substitution2;
    for (auto &[var, type] : substitution1) {
        s[var] = ApplySubstitution(substitution2, type);
    }
    return s;
}

void
FTVType(const TypePtr &type, std::unordered_set<int> &free_vars)
{
    if (type->tag == TypeTag::TVAR) {
        free_vars.insert(type->var_id);
    } else if (type->tag == TypeTag::TAPP) {
        for (auto &t : type->args) {
            FTVType(t, free_vars);
        }
    }
    if (type->tag == TypeTag::TARR) {
        FTVType(type->from, free_vars);
        FTVType(type->to, free_vars);
    }
}

std::unordered_set<int>
FTVScheme(const Scheme &scheme)
{
    std::unordered_set<int> result;
    FTVType(scheme.type, result);
    // exclude quantified
    for (int v : scheme.vars) {
        result.erase(v);
    }
    return result;
}

std::unordered_set<int>
FTVEnv(const TypeEnv &environment)
{
    std::unordered_set<int> result;
    for (auto &[name, scheme] : environment) {
        auto ftv_scheme_vars = FTVScheme(scheme);
        result.insert(ftv_scheme_vars.begin(), ftv_scheme_vars.end());
    }
    return result;
}

TypePtr
Instantiate(const Scheme &scheme, TVarGenerator &generator)
{
    Subst s;
    for (int v : scheme.vars) {
        s[v] = Type::make_var(generator.Fresh());
    }
    return ApplySubstitution(s, scheme.type);
}

Scheme
Generalize(const TypeEnv &environment, const TypePtr &type)
{
    std::unordered_set<int> free_vars;
    FTVType(type, free_vars);
    std::unordered_set<int> free_vars_env = FTVEnv(environment);
    std::vector<int> result;
    for (int v : free_vars) {
        if (free_vars_env.find(v) == free_vars_env.end()) {
            result.push_back(v);
        }
    }

    return Scheme{result, type};
}

bool
OccursInType(int var_id, const TypePtr &type, const Subst &substitution)
{
    TypePtr applied_subst = ApplySubstitution(substitution, type);
    std::unordered_set<int> free_vars;
    FTVType(applied_subst, free_vars);
    return (free_vars.find(var_id) != free_vars.end());
}

Subst
Unify(TypePtr type1, TypePtr type2)
{
    if (type1->tag == TypeTag::TVAR) {
        if (type2->tag == TypeTag::TVAR && type1->var_id == type2->var_id) {
            // same variable
            return {};
        }
        if (OccursInType(type1->var_id, type2, {})) {
            throw std::runtime_error("Occurs check failed");
        }
        return Subst{{type1->var_id, type2}};
    }

    if (type2->tag == TypeTag::TVAR) {
        return Unify(type2, type1);
    }

    if (type1->tag == TypeTag::TCONST && type2->tag == TypeTag::TCONST) {
        if (type1->name == type2->name) {
            return {};
        }
        throw std::runtime_error("Type mismatch: " + type1->name + " vs " + type2->name);
    }

    if (type1->tag == TypeTag::TAPP && type2->tag == TypeTag::TAPP && type1->name == type2->name &&
        type1->args.size() == type2->args.size()) {
        Subst s;
        for (size_t i = 0; i < type2->args.size(); ++i) {
            TypePtr type1_subst = ApplySubstitution(s, type1->args[i]);
            TypePtr type2_subst = ApplySubstitution(s, type2->args[i]);
            Subst s2 = Unify(type1_subst, type2_subst);
            s = Compose(s2, s);
        }
        return s;
    }

    if (type1->tag == TypeTag::TARR && type2->tag == TypeTag::TARR) {
        Subst s1 = Unify(type1->from, type2->from);
        Subst s2 = Unify(ApplySubstitution(s1, type1->to), ApplySubstitution(s1, type2->to));
        return Compose(s2, s1);
    }

    throw std::runtime_error("Cannot unify types");
}

TypePtr
Arrows(const std::vector<TypePtr> &param_types, TypePtr return_type)
{
    TypePtr result = return_type;
    if (param_types.empty()) {
        return result;
    }
    for (auto it = param_types.rbegin(); it != param_types.rend(); ++it) {
        result = Type::make_arrow(*it, result);
    }
    return result;
}
