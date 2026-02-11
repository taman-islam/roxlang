#ifndef ROX_TOKEN_H
#define ROX_TOKEN_H

#include <string>
#include <string_view>
#include <variant>

namespace rox {

enum class TokenType {
    // Single-character tokens.
    LEFT_PAREN, RIGHT_PAREN, LEFT_BRACE, RIGHT_BRACE,
    LEFT_BRACKET, RIGHT_BRACKET, COMMA, DOT, MINUS, PLUS,
    SEMICOLON, SLASH, STAR, PERCENT,

    // One or two character tokens.
    BANG, BANG_EQUAL,
    EQUAL, EQUAL_EQUAL,
    GREATER, GREATER_EQUAL,
    LESS, LESS_EQUAL,

    // Literals.
    IDENTIFIER, STRING, NUMBER_INT, NUMBER_FLOAT, CHAR_LITERAL,

    // Keywords.
    AND, CLASS, ELSE, FALSE, FUNCTION, IF, LET, CONST, NONE, OR,
    PRINT, RETURN, TRUE, WHILE, FOR, REPEAT, RANGE, NOT, READ_LINE,

    // Types
    TYPE_NUM32, TYPE_NUM64, TYPE_FLOAT, TYPE_BOOL, TYPE_CHAR,
    TYPE_LIST, TYPE_DICT,

    // End of file.
    END_OF_FILE,

    // Error token
    ERROR
};

struct Token {
    TokenType type;
    std::string lexeme;
    int line;
    // For literals, we might want to store the value, but keeping it simple for now.
    // The parser can parse the value from the lexeme.
};

} // namespace rox

#endif // ROX_TOKEN_H
