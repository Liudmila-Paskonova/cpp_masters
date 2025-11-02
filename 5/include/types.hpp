#pragma once
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>

struct Type;
using TypePtr = std::shared_ptr<Type>;

enum class TypeTag {
    /// type var
    TVAR,
    /// type constant
    TCONST,
    /// type application
    TAPP,
    /// func type
    TARR
};

struct Type {
    TypeTag tag;
    /// TVAR
    int var_id = -1;
    /// TCONST, TAPP
    std::string name;
    /// TAPP
    std::vector<TypePtr> args;
    /// TARROW
    TypePtr from;
    TypePtr to;

    Type(int id);
    Type(const std::string &c);
    Type(const std::string &c, const std::vector<TypePtr> &a);
    Type(TypePtr f, TypePtr t);

    static TypePtr make_var(int id);
    static TypePtr make_const(const std::string &n);
    static TypePtr make_app(const std::string &n, const std::vector<TypePtr> &a);
    static TypePtr make_arrow(TypePtr a, TypePtr b);
};

struct Scheme {
    /// quantified type vars
    std::vector<int> vars;
    /// the polymorphic type
    TypePtr type;
};

/// variable names -> their schemes
using TypeEnv = std::unordered_map<std::string, Scheme>;
/// type variables -> their types
using Subst = std::unordered_map<int, TypePtr>;

class TVarGenerator
{
    int next = 0;

  public:
    int Fresh();
};

class TypePrinter
{
  public:
    std::string Print(const TypePtr &t);
    std::string PrintScheme(const Scheme &scheme);

  private:
    void PrintImpl(const TypePtr &t, std::ostringstream &os);
};

/// Apply subst to the type
TypePtr ApplySubstitution(const Subst &substitution, const TypePtr &type);

/// Compose two substitutions (substitution2 after substitutions1)
Subst Compose(const Subst &substitution2, const Subst &substitution1);

/// Find all free type variables in a type
void FTVType(const TypePtr &type, std::unordered_set<int> &free_vars);

/// Free type variables in a scheme (excluding quantified ones)
std::unordered_set<int> FTVScheme(const Scheme &scheme);

/// Free type variables in type environment
std::unordered_set<int> FTVEnv(const TypeEnv &environment);

/// Instantiate a polymorphic type scheme with fresh type variables
TypePtr Instantiate(const Scheme &scheme, TVarGenerator &generator);

/// Generalize a type into a scheme by quantifying free type variables
Scheme Generalize(const TypeEnv &environment, const TypePtr &type);

/// Check if type variable appears in the type with subst
bool OccursInType(int var_id, const TypePtr &type, const Subst &substitution);

/// Find substitution that makes two types equal
Subst Unify(TypePtr type1, TypePtr type2);

/// Build func -> types
TypePtr Arrows(const std::vector<TypePtr> &param_types, TypePtr return_type);
