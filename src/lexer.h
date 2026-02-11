#ifndef ROX_LEXER_H
#define ROX_LEXER_H

#include <string>
#include <vector>
#include "token.h"

namespace rox {

class Lexer {
public:
    Lexer(const std::string& source);
    std::vector<Token> scanTokens();

private:
    std::string source;
    std::vector<Token> tokens;
    size_t start = 0;
    size_t current = 0;
    int line = 1;

    bool isAtEnd();
    char advance();
    char peek();
    char peekNext();
    bool match(char expected);
    void addToken(TokenType type);
    void addToken(TokenType type, std::string literal); // Overload if needed

    void scanToken();
    void string();
    void number();
    void identifier();
    void character();
};

} // namespace rox

#endif // ROX_LEXER_H
