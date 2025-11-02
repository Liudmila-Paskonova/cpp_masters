#include "gtest/gtest.h"
#include "type_inference.hpp"
#include "parser.hpp"
#include "lexer.hpp"

class HMTest : public ::testing::Test
{
  protected:
    std::string
    HindleyMilner(const std::string &input)
    {
        Lexer lexer(input);
        auto tokens = lexer.Tokenize();
        Parser parser(tokens);
        auto expression = parser.Parse();

        InferState state;
        state.InitBuiltins();
        auto [substitution, inferred_type] = InferExpr(state, expression);
        TypePtr final_type = ApplySubstitution(substitution, inferred_type);
        TypePrinter printer;

        std::string result;

        if (std::holds_alternative<List>(expression)) {
            const List &list_expr = std::get<List>(expression);
            if (!list_expr.elements.empty() && std::holds_alternative<Symbol>(list_expr.elements[0])) {
                const Symbol &head = std::get<Symbol>(list_expr.elements[0]);
                if (head.name == "define" && list_expr.elements.size() >= 2 &&
                    std::holds_alternative<Symbol>(list_expr.elements[1])) {

                    std::string defined_name = std::get<Symbol>(list_expr.elements[1]).name;

                    auto env_it = state.environment.find(defined_name);
                    if (env_it != state.environment.end()) {
                        result = defined_name + " : " + printer.PrintScheme(env_it->second);
                    }
                }
            }
        }
        return result;
    }
};

TEST_F(HMTest, Identity)
{
    std::string input = "(define func (lambda (x) x))";
    ASSERT_EQ(HindleyMilner(input), "func : (forall (t3) (func t3 t3))");
}

TEST_F(HMTest, Constant)
{
    std::string input = "(define func (lambda (x) (lambda (y) x)))";
    ASSERT_EQ(HindleyMilner(input), "func : (forall (t4 t3) (func t3 (func t4 t3)))");
}

TEST_F(HMTest, Composition)
{
    std::string input = "(define func (lambda (f g) (lambda (x) (f (g x)))))";
    ASSERT_EQ(HindleyMilner(input), "func : (forall (t5 t7 t6) (func (func t6 t7) (func (func t5 t6) (func t5 t7))))");
}

TEST_F(HMTest, Flip)
{
    std::string input = "(define func (lambda (f) (lambda (a b) (f b a))))";
    ASSERT_EQ(HindleyMilner(input), "func : (forall (t6 t4 t5) (func (func t5 (func t4 t6)) (func t4 (func t5 t6))))");
}

TEST_F(HMTest, Map)
{
    std::string input = "(define map (lambda (f xs) (case xs (nil nil) ((cons x xs1) (cons (f x) (map f xs1))))))";
    ASSERT_EQ(HindleyMilner(input), "map : (forall (t8 t5) (func (func t5 t8) (func (list t5) (list t8))))");
}

TEST_F(HMTest, Fold)
{
    std::string input = "(define fold (lambda (f acc xs) (case xs (nil acc) ((cons x xs1) (fold f (f acc x) xs1)))))";
    ASSERT_EQ(HindleyMilner(input),
              "fold : (forall (t6 t8) (func (func t8 (func t6 t8)) (func t8 (func (list t6) t8))))");
}

TEST_F(HMTest, Append)
{
    std::string input = "(define append (lambda (xs ys) (case xs (nil ys) ((cons x xs1) (cons x (append xs1 ys))))))";
    ASSERT_EQ(HindleyMilner(input), "append : (forall (t5) (func (list t5) (func (list t5) (list t5))))");
}

TEST_F(HMTest, Curry)
{
    std::string input = "(define curry (lambda (f) (lambda (x) (lambda (y) (f x y)))))";
    ASSERT_EQ(HindleyMilner(input), "curry : (forall (t6 t5 t4) (func (func t4 (func t5 t6)) (func t4 (func t5 t6))))");
}
