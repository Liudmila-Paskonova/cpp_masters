#include <exception>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <sstream>
#include "lexer.hpp"
#include "parser.hpp"
#include "type_inference.hpp"

int
main(int argc, char *argv[])
{
    if (argc != 2) {
        std::cerr << "Format: ./build/<MODE>/hindley_milner <input_file>" << std::endl;
        return 1;
    }

    std::ifstream input_file(argv[1]);

    try {
        if (!input_file.is_open()) {
            throw std::runtime_error("Couldn't open file!\n");
        }

        std::stringstream buffer;
        buffer << input_file.rdbuf();
        std::string input = buffer.str();

        Lexer lexer(input);
        auto tokens = lexer.Tokenize();

        Parser parser(tokens);
        auto expression = parser.Parse();

        InferState state;
        state.InitBuiltins();

        auto [substitution, inferred_type] = InferExpr(state, expression);
        TypePtr final_type = ApplySubstitution(substitution, inferred_type);
        TypePrinter printer;

        if (std::holds_alternative<List>(expression)) {
            const List &list_expr = std::get<List>(expression);
            if (!list_expr.elements.empty() && std::holds_alternative<Symbol>(list_expr.elements[0])) {
                const Symbol &head = std::get<Symbol>(list_expr.elements[0]);

                if (head.name == "define" && list_expr.elements.size() >= 2 &&
                    std::holds_alternative<Symbol>(list_expr.elements[1])) {

                    std::string defined_name = std::get<Symbol>(list_expr.elements[1]).name;

                    auto env_it = state.environment.find(defined_name);
                    if (env_it != state.environment.end()) {
                        std::filesystem::path file_path = argv[1];
                        std::ofstream output_file(file_path.parent_path() / "output.txt");
                        output_file << defined_name << " : " << printer.PrintScheme(env_it->second) << "\n";
                        output_file.close();
                    }
                }
            }
        }

    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    input_file.close();

    return 0;
}
