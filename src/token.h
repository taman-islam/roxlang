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
    SEMICOLON, SLASH, STAR, PERCENT, COLON,

    // One or two character tokens.

    EQUAL, EQUAL_EQUAL,
    GREATER, GREATER_EQUAL,
    LESS, LESS_EQUAL,

    // Literals.
    IDENTIFIER, STRING, NUMBER_INT, NUMBER_FLOAT, CHAR_LITERAL,

    // Keywords.
    AND, ELSE, FALSE, FUNCTION, IF, CONST, NONE, OR,
    PRINT, RETURN, TRUE, FOR, NOT, READ_LINE,
    BREAK, CONTINUE, TYPE, DEFAULT,

    // Types
    TYPE_INT64, TYPE_FLOAT64, TYPE_BOOL, TYPE_CHAR, TYPE_STRING, TYPE_LIST, TYPE_DICT,
    TYPE_ROX_RESULT, // New

    // End of file.
    END_OF_FILE,

    // Error token
    ERROR,

    // Comment
    COMMENT
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
