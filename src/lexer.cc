#include "lexer.h"
#include <unordered_map>
#include <iostream>

namespace rox {

const std::unordered_map<std::string, TokenType>& Lexer::getKeywords() {
    static const std::unordered_map<std::string, TokenType> keywords = {
        {"and", TokenType::AND},
        {"else", TokenType::ELSE},
        {"false", TokenType::FALSE},
        {"function", TokenType::FUNCTION},
        {"if", TokenType::IF},
        {"const", TokenType::CONST},
        {"none", TokenType::NONE},
        {"or", TokenType::OR},
        {"print", TokenType::PRINT},
        {"read_line", TokenType::READ_LINE},
        {"return", TokenType::RETURN},
        {"true", TokenType::TRUE},
        {"for", TokenType::FOR},
        {"not", TokenType::NOT},
        {"break", TokenType::BREAK},
        {"continue", TokenType::CONTINUE},
        {"type", TokenType::TYPE},
        {"default", TokenType::DEFAULT},
        {"int64", TokenType::TYPE_INT64},
        {"float64", TokenType::TYPE_FLOAT64},
        {"bool", TokenType::TYPE_BOOL},
        {"char", TokenType::TYPE_CHAR},
        {"list", TokenType::TYPE_LIST},
        {"dictionary", TokenType::TYPE_DICT},
        {"string", TokenType::TYPE_STRING},
        {"rox_result", TokenType::TYPE_ROX_RESULT},
    };
    return keywords;
}

const std::unordered_set<std::string>& Lexer::getBuiltins() {
    static const std::unordered_set<std::string> builtins = {
        // Core Functions
        "isOk", "getValue", "getError", "ok", "error", "range",
        // Constants not in keywords
        "pi", "e", "EOF",
        // Math Functions (int64)
        "int64_abs", "int64_min", "int64_max", "int64_pow",
        // Math Functions (float64)
        "float64_abs", "float64_min", "float64_max", "float64_pow", "float64_sqrt",
        "float64_sin", "float64_cos", "float64_tan", "float64_log", "float64_exp", "float64_floor", "float64_ceil",
        // Collection Helpers
        "rox_at", "rox_set", "rox_remove", "rox_has", "rox_keys", "rox_div", "rox_mod", "rox_get",
        // Special
        "main"
    };
    return builtins;
}

Lexer::Lexer(const std::string& source) : source(source) {}

std::vector<Token> Lexer::scanTokens() {
    while (!isAtEnd()) {
        start = current;
        scanToken();
    }

    tokens.push_back({TokenType::END_OF_FILE, "", line});
    return tokens;
}

void Lexer::scanToken() {
    char c = advance();
    switch (c) {
        case '(': addToken(TokenType::LEFT_PAREN); break;
        case ')': addToken(TokenType::RIGHT_PAREN); break;
        case '{': addToken(TokenType::LEFT_BRACE); break;
        case '}': addToken(TokenType::RIGHT_BRACE); break;
        case '[': addToken(TokenType::LEFT_BRACKET); break;
        case ']': addToken(TokenType::RIGHT_BRACKET); break;
        case ',': addToken(TokenType::COMMA); break;
        case '.': addToken(TokenType::DOT); break;
        case '-': addToken(TokenType::MINUS); break;
        case '+': addToken(TokenType::PLUS); break;
        case ';': addToken(TokenType::SEMICOLON); break;
        case '*': addToken(TokenType::STAR); break;
        case '%': addToken(TokenType::PERCENT); break;
        case ':': addToken(TokenType::COLON); break;

        case '=':
            addToken(match('=') ? TokenType::EQUAL_EQUAL : TokenType::EQUAL);
            break;
        case '<':
            addToken(match('=') ? TokenType::LESS_EQUAL : TokenType::LESS);
            break;
        case '>':
            addToken(match('=') ? TokenType::GREATER_EQUAL : TokenType::GREATER);
            break;
        case '/':
            if (match('/')) {
                // A comment goes until the end of the line.
                while (peek() != '\n' && !isAtEnd()) advance();
                addToken(TokenType::COMMENT);
            } else {
                addToken(TokenType::SLASH);
            }
            break;

        case ' ':
        case '\r':
        case '\t':
            // Ignore whitespace.
            break;

        case '\n':
            line++;
            break;

        case '"': string(); break;
        case '\'': character(); break;

        default:
            if (isdigit(c)) {
                number();
            } else if (isalpha(c)) {
                identifier();
            } else {
                std::cerr << "Unexpected character at line " << line << ": " << c << std::endl;
            }
            break;
    }
}

void Lexer::identifier() {
    while (isalnum(peek()) || peek() == '_') advance();

    std::string text = source.substr(start, current - start);

    if (text.rfind("roxv26_", 0) == 0) { // Check if starts with "roxv26_"
        std::cerr << "Error: Identifier '" << text << "' cannot start with reserved prefix 'roxv26_'." << std::endl;
        exit(1);
    }

    TokenType type = TokenType::IDENTIFIER;
    const auto& keywords = getKeywords();
    auto search = keywords.find(text);
    if (search != keywords.end()) {
        type = search->second;
    }
    addToken(type);
}

void Lexer::number() {
    while (isdigit(peek())) advance();

    // Look for a fractional part.
    if (peek() == '.' && isdigit(peekNext())) {
        // Consume the "."
        advance();

        while (isdigit(peek())) advance();

        addToken(TokenType::NUMBER_FLOAT); // Float literal
        return;
    }

    // Look for n32 suffix - REMOVED
    // if (peek() == 'n') { ... }

    addToken(TokenType::NUMBER_INT); // int64 (default int)
    return;
}

void Lexer::string() {
    while (peek() != '"' && !isAtEnd()) {
        if (peek() == '\n') line++;
        advance();
    }

    if (isAtEnd()) {
        std::cerr << "Unterminated string at line " << line << std::endl;
        return;
    }

    // The closing ".
    advance();

    // Trim the surrounding quotes.
    std::string value = source.substr(start + 1, current - start - 2);
    addToken(TokenType::STRING);
}

void Lexer::character() {
    if (peek() == '\\') {
        advance(); // escape
        advance(); // char
    } else {
        advance();
    }

    if (peek() == '\'') {
        advance();
        addToken(TokenType::CHAR_LITERAL);
    } else {
         std::cerr << "Unterminated char literal at line " << line << std::endl;
    }
}

bool Lexer::isAtEnd() {
    return current >= source.length();
}

char Lexer::advance() {
    return source[current++];
}

char Lexer::peek() {
    if (isAtEnd()) return '\0';
    return source[current];
}

char Lexer::peekNext() {
    if (current + 1 >= source.length()) return '\0';
    return source[current + 1];
}

bool Lexer::match(char expected) {
    if (isAtEnd()) return false;
    if (source[current] != expected) return false;

    current++;
    return true;
}

void Lexer::addToken(TokenType type) {
    addToken(type, "");
}

void Lexer::addToken(TokenType type, std::string /*literal*/) {
    std::string text = source.substr(start, current - start);
    tokens.push_back({type, text, line});
}

} // namespace rox
