#include <format>
#include <iostream>
#include <libgen.h>
#include <string>
#include <unistd.h>

#include "slang/ast/Compilation.h"
#include "slang/ast/symbols/CompilationUnitSymbols.h"
#include "slang/ast/symbols/InstanceSymbols.h"
#include "slang/ast/symbols/PortSymbols.h"
#include "slang/ast/symbols/VariableSymbols.h"
#include "slang/ast/types/Type.h"
#include "slang/syntax/SyntaxTree.h"
#include "slang/syntax/SyntaxVisitor.h"

std::string PROGNAME;

std::string usage() {
    return std::format("Usage: {:s} [-h] [-f FILE]\n"
                       "  -h     \tPrint this help message\n"
                       "  -f FILE\tRead from FILE instead of STDIN\n",
                       PROGNAME);
}

int main(int argc, char** argv) {
    PROGNAME = basename(argv[0]);

    // Get input from STDIN by default
    std::string input_path = "/dev/stdin";

    int opt;
    while ((opt = getopt(argc, argv, "hf:")) != -1) {
        switch (opt) {
            case 'h': {
                std::cout << usage();
                return EXIT_SUCCESS;
            }
            case 'f': {
                // if -f then use FILE
                input_path = optarg;
                break;
            }
            default: {
                std::cerr << usage();
                return EXIT_FAILURE;
            }
        }
    }

    // Update arg
    argc -= optind;
    argv += optind;

    if (argc > 0) {
        std::cerr << std::format("{:s}: Extra arguments given\n", PROGNAME);
        std::cerr << usage();
    }

    slang::syntax::SyntaxTree::TreeOrError tree_or_err = slang::syntax::SyntaxTree::fromFile(
        input_path);

    if (!tree_or_err) {
        std::cerr << std::format("{:s}: Couldn't open {:s}\n", PROGNAME, input_path);
        return tree_or_err.error().first.value();
    }

    std::shared_ptr<slang::syntax::SyntaxTree> tree = *tree_or_err;

    slang::ast::Compilation comp;

    comp.addSyntaxTree(tree);

    // This will elaborate the design and you cannot make changes afterwards
    // ergo const and final
    const slang::ast::RootSymbol& root_symbol = comp.getRoot();

    // $root will always be present and will always have a CompilationUnit
    // Just iterate over Instances
    for (auto& symbol : root_symbol.membersOfType<slang::ast::InstanceSymbol>()) {

        // Get all the Variables
        for (auto& symbol : symbol.body.membersOfType<slang::ast::VariableSymbol>()) {
            std::cout << symbol.name << " " << symbol.getType().toString() << "\n";
        }

        // Get all the Nets
        for (auto& symbol : symbol.body.membersOfType<slang::ast::NetSymbol>()) {
            std::cout << symbol.name << " " << symbol.getType().toString() << "\n";
        }
    }

    return EXIT_SUCCESS;
}
