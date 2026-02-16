#include "formatter.h"
#include <sstream>

namespace rox {

Formatter::Formatter(const std::vector<Token>& tokens) : tokens(tokens) {}

std::string Formatter::format() {
    std::stringstream out;
    bool startOfLine = true;

    for (size_t i = 0; i < tokens.size(); ++i) {
        Token t = tokens[i];
        if (t.type == TokenType::END_OF_FILE) break;

        // Handle closing brace indentation
        if (t.type == TokenType::RIGHT_BRACE) {
            if (startOfLine) {
                if (indentLevel > 0) indentLevel--;
                // Re-check startOfLine to print indent
            }
        }

        // Preserve newlines (collapse multiple to one)
        if (i > 0) {
            int gap = t.line - tokens[i-1].line;
            if (gap > 1) {
                if (!startOfLine) {
                    out << "\n";
                    startOfLine = true;
                }
                out << "\n";
            }
        }

        // Emit Indent if at start of line
        if (startOfLine) {
            for (int k = 0; k < indentLevel; ++k) out << "    ";
            startOfLine = false;
        }

        // Print token
        out << t.lexeme;

        // Logic for next token
        bool newlineAfter = false;
        bool spaceAfter = false;

        if (t.type == TokenType::LEFT_BRACE) {
            newlineAfter = true;
            indentLevel++;
        } else if (t.type == TokenType::RIGHT_BRACE) {
            newlineAfter = true;
        } else if (t.type == TokenType::SEMICOLON) {
            newlineAfter = true;
        } else if (t.type == TokenType::COMMENT) {
            newlineAfter = true;
        } else {
             // Space logic
             if (i + 1 < tokens.size()) {
                 Token next = tokens[i+1];
                 if (next.type == TokenType::SEMICOLON || next.type == TokenType::COMMA || next.type == TokenType::DOT || next.type == TokenType::RIGHT_PAREN || next.type == TokenType::LEFT_BRACKET || next.type == TokenType::RIGHT_BRACKET) {
                     spaceAfter = false;
                 } else if (t.type == TokenType::LEFT_PAREN || t.type == TokenType::LEFT_BRACKET || t.type == TokenType::DOT) {
                     spaceAfter = false;
                 } else {
                     spaceAfter = true;
                 }

                 // Special cases
                 // func() no space
                 if (t.type == TokenType::IDENTIFIER && next.type == TokenType::LEFT_PAREN) spaceAfter = false;
                 // print(...) no space
                 if (t.type == TokenType::PRINT && next.type == TokenType::LEFT_PAREN) spaceAfter = false;
                 // -> no space
                 if (t.type == TokenType::MINUS && next.type == TokenType::GREATER) spaceAfter = false;

                 // if ( space
                 if ((t.type == TokenType::IF || t.type == TokenType::FOR) && next.type == TokenType::LEFT_PAREN) spaceAfter = true;
             }
        }

        if (newlineAfter) {
            out << "\n";
            startOfLine = true;
        } else if (spaceAfter) {
            out << " ";
        }
    }

    return out.str();
}

} // namespace rox
