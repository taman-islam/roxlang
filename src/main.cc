// ... (in main.cc)
// To test parser:
// rox test_parse file.rox

// Update main.cc
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include "lexer.h"
#include "parser.h"

std::string readFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Could not open file " << path << std::endl;
        exit(1);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void test_lex(const std::string& source) {
    rox::Lexer lexer(source);
    std::vector<rox::Token> tokens = lexer.scanTokens();
    for (const auto& token : tokens) {
        std::cout << "Line " << token.line << ": " << token.lexeme << " (" << (int)token.type << ")" << std::endl;
    }
}

void test_parse(const std::string& source) {
    rox::Lexer lexer(source);
    std::vector<rox::Token> tokens = lexer.scanTokens();
    rox::Parser parser(tokens);
    std::vector<std::unique_ptr<rox::Stmt>> statements = parser.parse();
    std::cout << "Parsed " << statements.size() << " statements." << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: rox <command> [args]" << std::endl;
        return 1;
    }

    std::string command = argv[1];

    if (command == "test_lex") {
        if (argc < 3) {
            std::cout << "Usage: rox test_lex <file>" << std::endl;
            return 1;
        }
        test_lex(readFile(argv[2]));
    } else if (command == "test_parse") {
        if (argc < 3) {
            std::cout << "Usage: rox test_parse <file>" << std::endl;
            return 1;
        }
        test_parse(readFile(argv[2]));
    } else {
        std::cout << "Unknown command: " << command << std::endl;
        return 1;
    }

    return 0;
}
