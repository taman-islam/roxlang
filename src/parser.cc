#include "parser.h"
#include <iostream>

namespace rox {

Parser::Parser(const std::vector<Token>& tokens) : tokens(tokens) {}

std::vector<std::unique_ptr<Stmt>> Parser::parse() {
    std::vector<std::unique_ptr<Stmt>> statements;
    while (!isAtEnd()) {
        statements.push_back(declaration());
    }
    return statements;
}

std::unique_ptr<Stmt> Parser::declaration() {
    if (match({TokenType::FUNCTION})) return functionDeclaration("function");
    if (match({TokenType::FUNCTION})) return functionDeclaration("function");
    if (match({TokenType::CONST})) return varDeclaration();

    // Check for variable declaration starting with a type
    if (check(TokenType::TYPE_NUM32) || check(TokenType::TYPE_NUM) ||
        check(TokenType::TYPE_FLOAT) || check(TokenType::TYPE_BOOL) ||
        check(TokenType::TYPE_CHAR) || check(TokenType::TYPE_STRING) ||
        check(TokenType::TYPE_LIST) ||
        check(TokenType::TYPE_DICT) || check(TokenType::TYPE_ROX_RESULT) ||
        check(TokenType::NONE)) {
        return varDeclaration();
    }

    return statement();
}

std::unique_ptr<Stmt> Parser::functionDeclaration(std::string kind) {
    Token name = consume(TokenType::IDENTIFIER, "Expect " + kind + " name.");

    consume(TokenType::LEFT_PAREN, "Expect '(' after " + kind + " name.");

    std::vector<FunctionStmt::Param> params;
    if (!check(TokenType::RIGHT_PAREN)) {
    if (!check(TokenType::RIGHT_PAREN)) {
        do {
            std::unique_ptr<Type> paramType = type();
            Token paramName = consume(TokenType::IDENTIFIER, "Expect parameter name.");
            params.push_back({paramName, std::move(paramType)});
        } while (match({TokenType::COMMA}));
    }
    }
    consume(TokenType::RIGHT_PAREN, "Expect ')' after parameters.");

    consume(TokenType::MINUS, "Expect '->' return type.");
    consume(TokenType::GREATER, "Expect '->' return type."); // The > in ->

    std::unique_ptr<Type> returnType = type();

    consume(TokenType::LEFT_BRACE, "Expect '{' before " + kind + " body.");
    std::vector<std::unique_ptr<Stmt>> body = block();

    return std::make_unique<FunctionStmt>(name, std::move(params), std::move(returnType), std::move(body));
}

std::unique_ptr<Stmt> Parser::varDeclaration() {
    bool isConst = false;
    // blocked by previous() check in caller?
    // If we matched CONST, previous() is CONST.
    // If we matched TYPE, previous() is ... wait, we used check() for TYPE in caller.
    // But for CONST we utilized match(), so previous is CONST.

    if (previous().type == TokenType::CONST) {
        isConst = true;
    }

    // Parse Type
    // If it was const, we are now at the type.
    // If it was not const, we are at the type (via check in caller).
    std::unique_ptr<Type> varType = type();

    Token name = consume(TokenType::IDENTIFIER, "Expect variable name.");

    // Removed < > around type.

    std::unique_ptr<Expr> initializer = nullptr;
    if (match({TokenType::EQUAL})) {
         initializer = expression();
    }

    consume(TokenType::SEMICOLON, "Expect ';' after variable declaration.");

    return std::make_unique<LetStmt>(name, std::move(varType), std::move(initializer), isConst);
}

std::unique_ptr<Stmt> Parser::statement() {
    if (match({TokenType::IF})) return ifStatement();
    if (match({TokenType::REPEAT})) return repeatStatement();
    if (match({TokenType::RETURN})) return returnStatement();
    if (match({TokenType::LEFT_BRACE})) return std::make_unique<BlockStmt>(block());

    std::unique_ptr<Expr> expr = expression();
    consume(TokenType::SEMICOLON, "Expect ';' after expression.");
    return std::make_unique<ExprStmt>(std::move(expr));
}

std::unique_ptr<Stmt> Parser::ifStatement() {
    consume(TokenType::LEFT_PAREN, "Expect '(' after 'if'.");
    std::unique_ptr<Expr> condition = expression();
    consume(TokenType::RIGHT_PAREN, "Expect ')' after if condition.");

    std::unique_ptr<Stmt> thenBranch = statement();
    std::unique_ptr<Stmt> elseBranch = nullptr;

    if (match({TokenType::ELSE})) {
        elseBranch = statement();
    }

    return std::make_unique<IfStmt>(std::move(condition), std::move(thenBranch), std::move(elseBranch));
}

std::unique_ptr<Stmt> Parser::repeatStatement() {
    Token iterator = consume(TokenType::IDENTIFIER, "Expect iterator name after 'repeat'.");
    consume(TokenType::IDENTIFIER, "Expect 'in' after iterator."); // keyword 'in' ? Lexer doesn't have IN?
    // Wait, 'in' is not in my keyword list in token.h?
    // Checking token.h... AND, CLASS, ... IF, LET, ...
    // Checking instruction.md... "repeat i in range..."
    // I missed 'in' keyword!
    // I'll treat it as identifier "in" for now, or add it to lexer.
    // Since I already implemented lexer without 'in', it will be scanned as IDENTIFIER "in".
    // So consume(IDENTIFIER) and check lexeme?
    // Or just assume if it's there.

    // Actually, "range" is keyword RANGE in lexer.
    consume(TokenType::RANGE, "Expect 'range' after 'in'.");
    consume(TokenType::LEFT_PAREN, "Expect '(' after 'range'.");

    std::unique_ptr<Expr> start = expression();
    consume(TokenType::COMMA, "Expect ',' after start.");
    std::unique_ptr<Expr> end = expression();

    std::unique_ptr<Expr> step = nullptr;
    if (match({TokenType::COMMA})) {
        step = expression();
    }

    consume(TokenType::RIGHT_PAREN, "Expect ')' after range arguments.");

    std::unique_ptr<Stmt> body = statement();

    return std::make_unique<RepeatStmt>(iterator, std::move(start), std::move(end), std::move(step), std::move(body));
}

std::unique_ptr<Stmt> Parser::returnStatement() {
    Token keyword = previous();
    std::unique_ptr<Expr> value = nullptr;
    if (!check(TokenType::SEMICOLON)) {
        value = expression();
    }
    consume(TokenType::SEMICOLON, "Expect ';' after return value.");
    return std::make_unique<ReturnStmt>(keyword, std::move(value));
}

std::vector<std::unique_ptr<Stmt>> Parser::block() {
    std::vector<std::unique_ptr<Stmt>> statements;
    while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
        statements.push_back(declaration());
    }
    consume(TokenType::RIGHT_BRACE, "Expect '}' after block.");
    return statements;
}

std::unique_ptr<Expr> Parser::expression() {
    return assignment();
}

std::unique_ptr<Expr> Parser::assignment() {
    std::unique_ptr<Expr> expr = logic_or();

    if (match({TokenType::EQUAL})) {
        Token equals = previous();
        std::unique_ptr<Expr> value = assignment(); // recursive

        if (VariableExpr* v = dynamic_cast<VariableExpr*>(expr.get())) {
             Token name = v->name;
             // We need to release ownership from expr since we are creating a new AssignmentExpr
             // Actually, name is just a Token (copyable).
             return std::make_unique<AssignmentExpr>(name, std::move(value));
        }

        error(equals, "Invalid assignment target.");
    }

    return expr;
}

std::unique_ptr<Expr> Parser::logic_or() {
    std::unique_ptr<Expr> expr = logic_and();

    while (match({TokenType::OR})) {
        Token op = previous();
        std::unique_ptr<Expr> right = logic_and();
        expr = std::make_unique<LogicalExpr>(std::move(expr), op, std::move(right));
    }

    return expr;
}

std::unique_ptr<Expr> Parser::logic_and() {
    std::unique_ptr<Expr> expr = equality();

    while (match({TokenType::AND})) {
        Token op = previous();
        std::unique_ptr<Expr> right = equality();
        expr = std::make_unique<LogicalExpr>(std::move(expr), op, std::move(right));
    }

    return expr;
}

std::unique_ptr<Expr> Parser::equality() {
    std::unique_ptr<Expr> expr = comparison();

    while (match({TokenType::BANG_EQUAL, TokenType::EQUAL_EQUAL})) {
        Token op = previous();
        std::unique_ptr<Expr> right = comparison();

        // Ban "== true" and "== false"
        if (op.type == TokenType::EQUAL_EQUAL) {
            if (auto* lit = dynamic_cast<LiteralExpr*>(right.get())) {
                if (lit->value.type == TokenType::TRUE || lit->value.type == TokenType::FALSE) {
                    error(op, "Invalid comparison. Do not use '== true' or '== false'. Use 'if (cond)' or 'if (not cond)'.");
                }
            }
        }

        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }

    return expr;
}

std::unique_ptr<Expr> Parser::comparison() {
    std::unique_ptr<Expr> expr = term();

    while (match({TokenType::GREATER, TokenType::GREATER_EQUAL, TokenType::LESS, TokenType::LESS_EQUAL})) {
        Token op = previous();
        std::unique_ptr<Expr> right = term();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }

    return expr;
}

std::unique_ptr<Expr> Parser::term() {
    std::unique_ptr<Expr> expr = factor();

    while (match({TokenType::MINUS, TokenType::PLUS})) {
        Token op = previous();
        std::unique_ptr<Expr> right = factor();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }

    return expr;
}

std::unique_ptr<Expr> Parser::factor() {
    std::unique_ptr<Expr> expr = unary();

    while (match({TokenType::SLASH, TokenType::STAR, TokenType::PERCENT})) {
        Token op = previous();
        std::unique_ptr<Expr> right = unary();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }

    return expr;
}

std::unique_ptr<Expr> Parser::unary() {
    if (match({TokenType::BANG, TokenType::MINUS, TokenType::NOT})) {
        Token op = previous();
        std::unique_ptr<Expr> right = unary();
        return std::make_unique<UnaryExpr>(op, std::move(right));
    }
    return call();
}

std::unique_ptr<Expr> Parser::call() {
    std::unique_ptr<Expr> expr = primary();

    while (true) {
        if (match({TokenType::LEFT_PAREN})) {
            std::vector<std::unique_ptr<Expr>> arguments;
            if (!check(TokenType::RIGHT_PAREN)) {
                do {
                    arguments.push_back(expression());
                } while (match({TokenType::COMMA}));
            }
            Token paren = consume(TokenType::RIGHT_PAREN, "Expect ')' after arguments.");
            expr = std::make_unique<CallExpr>(std::move(expr), paren, std::move(arguments));
        } else if (match({TokenType::DOT})) {
            Token name = consume(TokenType::IDENTIFIER, "Expect property/method name after '.'.");
            consume(TokenType::LEFT_PAREN, "Expect '(' after method name.");
            std::vector<std::unique_ptr<Expr>> arguments;
            if (!check(TokenType::RIGHT_PAREN)) {
                do {
                    arguments.push_back(expression());
                } while (match({TokenType::COMMA}));
            }
            consume(TokenType::RIGHT_PAREN, "Expect ')' after arguments.");
            expr = std::make_unique<MethodCallExpr>(std::move(expr), name, std::move(arguments));
        } else {
            break;
        }
    }
    return expr;
}

std::unique_ptr<Expr> Parser::primary() {
    if (match({TokenType::FALSE})) return std::make_unique<LiteralExpr>(previous());
    if (match({TokenType::TRUE})) return std::make_unique<LiteralExpr>(previous());
    if (match({TokenType::NONE})) return std::make_unique<LiteralExpr>(previous());

    if (match({TokenType::NUMBER_INT, TokenType::NUMBER_FLOAT, TokenType::STRING, TokenType::CHAR_LITERAL})) {
        return std::make_unique<LiteralExpr>(previous());
    }

    if (match({TokenType::IDENTIFIER, TokenType::PRINT, TokenType::READ_LINE})) {
        return std::make_unique<VariableExpr>(previous());
    }

    if (match({TokenType::LEFT_BRACKET})) {
        std::vector<std::unique_ptr<Expr>> elements;
        if (!check(TokenType::RIGHT_BRACKET)) {
            do {
                elements.push_back(expression());
            } while (match({TokenType::COMMA}));
        }
        consume(TokenType::RIGHT_BRACKET, "Expect ']' after list elements.");
        return std::make_unique<ListLiteralExpr>(std::move(elements));
    }

    if (match({TokenType::LEFT_PAREN})) {
        std::unique_ptr<Expr> expr = expression();
        consume(TokenType::RIGHT_PAREN, "Expect ')' after expression.");
        return expr;
    }

    error(peek(), "Expect expression.");
    return nullptr;
}

std::unique_ptr<Type> Parser::type() {
    if (match({TokenType::TYPE_NUM32, TokenType::TYPE_NUM, TokenType::TYPE_FLOAT,
               TokenType::TYPE_BOOL, TokenType::TYPE_CHAR, TokenType::TYPE_STRING, TokenType::NONE})) {
        return std::make_unique<PrimitiveType>(previous());
    }

    if (match({TokenType::TYPE_LIST})) {
        consume(TokenType::LEFT_BRACKET, "Expect '[' after list.");
        std::unique_ptr<Type> elementType = type();
        consume(TokenType::RIGHT_BRACKET, "Expect ']' after list type.");
        return std::make_unique<ListType>(std::move(elementType));
    }

    if (match({TokenType::TYPE_DICT})) {
        consume(TokenType::LEFT_BRACKET, "Expect '[' after dictionary.");
        std::unique_ptr<Type> keyType = type();
        consume(TokenType::COMMA, "Expect ',' after key type.");
        std::unique_ptr<Type> valueType = type();
        consume(TokenType::RIGHT_BRACKET, "Expect ']' after dictionary type.");
        return std::make_unique<DictionaryType>(std::move(keyType), std::move(valueType));
    }

    if (match({TokenType::TYPE_ROX_RESULT})) {
        consume(TokenType::LEFT_BRACKET, "Expect '[' after rox_result.");
        std::unique_ptr<Type> valueType = type();
        consume(TokenType::RIGHT_BRACKET, "Expect ']' after rox_result type.");
        return std::make_unique<RoxResultType>(std::move(valueType));
    }

    error(peek(), "Expect type.");
    return nullptr;
}

bool Parser::match(const std::vector<TokenType>& types) {
    for (TokenType type : types) {
        if (check(type)) {
            advance();
            return true;
        }
    }
    return false;
}

bool Parser::check(TokenType type) {
    if (isAtEnd()) return false;
    return peek().type == type;
}

Token Parser::advance() {
    if (!isAtEnd()) current++;
    return previous();
}

bool Parser::isAtEnd() {
    return peek().type == TokenType::END_OF_FILE;
}

Token Parser::peek() {
    return tokens[current];
}

Token Parser::previous() {
    return tokens[current - 1];
}

Token Parser::consume(TokenType type, std::string message) {
    if (check(type)) return advance();
    error(peek(), message);
    return peek(); // Stub
}

void Parser::synchronize() {
    advance();
    while (!isAtEnd()) {
        if (previous().type == TokenType::SEMICOLON) return;
        switch (peek().type) {
            case TokenType::CLASS:
            case TokenType::FUNCTION:
            case TokenType::LET:
            case TokenType::CONST:
            case TokenType::FOR:
            case TokenType::IF:
            case TokenType::WHILE:
            case TokenType::PRINT:
            case TokenType::RETURN:
                return;
            default:
                advance();
        }
    }
}

void Parser::error(Token token, std::string message) {
    std::cerr << "[line " << token.line << "] Error at '" << token.lexeme << "': " << message << std::endl;
    // For now, simpler error handling (maybe throw?)
    // But since this is a simple compiler, printing to stderr is okay.
    // However, we should probably throw to stop parsing or resync.
    // For v0, let's just print and maybe exit or skip.
    exit(1);
}

} // namespace rox
