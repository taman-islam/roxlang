#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cstdlib>
#include "lexer.h"
#include "parser.h"
#include "codegen.h"

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

void writeFile(const std::string& path, const std::string& content) {
    std::ofstream file(path);
    if (!file.is_open()) {
        std::cerr << "Could not write to file " << path << std::endl;
        exit(1);
    }
    file << content;
}

std::string generate_cc(const std::string& source) {
    rox::Lexer lexer(source);
    std::vector<rox::Token> tokens = lexer.scanTokens();

    rox::Parser parser(tokens);
    std::vector<std::unique_ptr<rox::Stmt>> statements = parser.parse();

    rox::Codegen codegen(statements);
    std::string result = codegen.generate();
    return result;
}

void cmd_generate(const std::string& inputPath) {
    std::string source = readFile(inputPath);
    std::string cc = generate_cc(source);

    // Extract filename from input path (handle directories)
    std::string filename = inputPath;
    size_t lastSlash = inputPath.find_last_of('/');
    if (lastSlash != std::string::npos) {
        filename = inputPath.substr(lastSlash + 1);
    }

    // Remove extension
    if (filename.size() > 4 && filename.substr(filename.size() - 4) == ".rox") {
        filename = filename.substr(0, filename.size() - 4);
    }

    // Ensure generated directory exists
    system("mkdir -p generated");

    std::string outputPath = "generated/" + filename + ".cc";
    writeFile(outputPath, cc);
    std::cout << "Generated " << outputPath << std::endl;
}

void cmd_compile(const std::string& inputPath) {
    cmd_generate(inputPath);

    // Reconstruct output path logic to parse the filename
    std::string filename = inputPath;
    size_t lastSlash = inputPath.find_last_of('/');
    if (lastSlash != std::string::npos) {
        filename = inputPath.substr(lastSlash + 1);
    }
     if (filename.size() > 4 && filename.substr(filename.size() - 4) == ".rox") {
        filename = filename.substr(0, filename.size() - 4);
    }

    std::string ccPath = "generated/" + filename + ".cc";
    std::string binaryPath = "generated/" + filename;

    std::string cmd = "clang++ -w -std=c++20 -o " + binaryPath + " " + ccPath;
    int ret = system(cmd.c_str());
    if (ret != 0) {
        std::cerr << "Compilation failed." << std::endl;
        exit(1);
    }
    std::cout << "Compiled " << binaryPath << std::endl;
}

void cmd_run(const std::string& inputPath) {
    cmd_compile(inputPath);

    // Logic to get binary path
    std::string filename = inputPath;
     size_t lastSlash = inputPath.find_last_of('/');
    if (lastSlash != std::string::npos) {
        filename = inputPath.substr(lastSlash + 1);
    }
     if (filename.size() > 4 && filename.substr(filename.size() - 4) == ".rox") {
        filename = filename.substr(0, filename.size() - 4);
    }

    std::string binaryPath = "generated/" + filename;

    // Execute
    std::string cmd = "./" + binaryPath;
    int ret = system(cmd.c_str());
    if (ret != 0) {
         // Return exit code
         exit(WEXITSTATUS(ret));
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: rox <command> [args]" << std::endl;
        std::cout << "Commands:" << std::endl;
        std::cout << "  generate <file.rox>" << std::endl;
        std::cout << "  compile <file.rox>" << std::endl;
        std::cout << "  run <file.rox>" << std::endl;
        return 1;
    }

    std::string command = argv[1];

    if (command == "generate") {
        if (argc < 3) {
            std::cout << "Usage: rox generate <file.rox>" << std::endl;
            return 1;
        }
        cmd_generate(argv[2]);
    } else if (command == "compile") {
        if (argc < 3) {
            std::cout << "Usage: rox compile <file.rox>" << std::endl;
            return 1;
        }
        cmd_compile(argv[2]);
    } else if (command == "run") {
        if (argc < 3) {
            std::cout << "Usage: rox run <file.rox>" << std::endl;
            return 1;
        }
        cmd_run(argv[2]);
    } else if (command == "test_lex") {
        // ... kept for debugging
        // ... implementation from before ...
        // Re-implementing simplified version since I overwrite file
         if (argc < 3) return 1;
         rox::Lexer lexer(readFile(argv[2]));
         auto tokens = lexer.scanTokens();
         for (auto& t : tokens) std::cout << t.lexeme << "\n";
    } else if (command == "test_parse") {
        if (argc < 3) return 1;
        rox::Lexer lexer(readFile(argv[2]));
        auto tokens = lexer.scanTokens();
        rox::Parser parser(tokens);
        parser.parse();
    } else {
        std::cout << "Unknown command: " << command << std::endl;
        return 1;
    }

    return 0;
}
