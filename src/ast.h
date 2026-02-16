#ifndef ROX_AST_H
#define ROX_AST_H

#include <string>
#include <vector>
#include <memory>
#include <variant>
#include "token.h"

namespace rox {

// --- Types ---

struct Type {
    virtual ~Type() = default;
    virtual std::string toString() const = 0;
    virtual std::unique_ptr<Type> clone() const = 0;
};

struct PrimitiveType : Type {
    Token token; // e.g. num32, float, etc.
    PrimitiveType(Token token) : token(token) {}
    std::string toString() const override { return token.lexeme; }
    std::unique_ptr<Type> clone() const override { return std::make_unique<PrimitiveType>(token); }
};

struct ListType : Type {
    std::unique_ptr<Type> elementType;
    ListType(std::unique_ptr<Type> elementType) : elementType(std::move(elementType)) {}
    std::string toString() const override { return "list[" + elementType->toString() + "]"; }
    std::unique_ptr<Type> clone() const override { return std::make_unique<ListType>(elementType->clone()); }
};

struct DictionaryType : Type {
    std::unique_ptr<Type> keyType;
    std::unique_ptr<Type> valueType;
    DictionaryType(std::unique_ptr<Type> keyType, std::unique_ptr<Type> valueType)
        : keyType(std::move(keyType)), valueType(std::move(valueType)) {}
    std::string toString() const override {
        return "dictionary[" + keyType->toString() + ", " + valueType->toString() + "]";
    }
    std::unique_ptr<Type> clone() const override {
        return std::make_unique<DictionaryType>(keyType->clone(), valueType->clone());
    }
};

class RoxResultType : public Type {
public:
    std::unique_ptr<Type> valueType;
    RoxResultType(std::unique_ptr<Type> valueType) : valueType(std::move(valueType)) {}
    std::string toString() const override { return "result[" + valueType->toString() + "]"; }
    std::unique_ptr<Type> clone() const override { return std::make_unique<RoxResultType>(valueType->clone()); }
};

struct FunctionType : Type {
    std::vector<std::unique_ptr<Type>> paramTypes;
    std::unique_ptr<Type> returnType;
    FunctionType(std::vector<std::unique_ptr<Type>> paramTypes, std::unique_ptr<Type> returnType)
        : paramTypes(std::move(paramTypes)), returnType(std::move(returnType)) {}
    std::string toString() const override {
        std::string s = "function(";
        for (size_t i = 0; i < paramTypes.size(); ++i) {
            if (i > 0) s += ", ";
            s += paramTypes[i]->toString();
        }
        s += ") -> " + returnType->toString();
        return s;
    }
    std::unique_ptr<Type> clone() const override {
        std::vector<std::unique_ptr<Type>> paramTypesCopy;
        for (const auto& t : paramTypes) paramTypesCopy.push_back(t->clone());
        return std::make_unique<FunctionType>(std::move(paramTypesCopy), returnType->clone());
    }
};

// --- Expressions ---

struct Expr {
    virtual ~Expr() = default;
};

struct LogicalExpr : Expr {
    std::unique_ptr<Expr> left;
    Token op;
    std::unique_ptr<Expr> right;
    LogicalExpr(std::unique_ptr<Expr> left, Token op, std::unique_ptr<Expr> right)
        : left(std::move(left)), op(op), right(std::move(right)) {}
};

struct BinaryExpr : Expr {
    std::unique_ptr<Expr> left;
    Token op;
    std::unique_ptr<Expr> right;
    BinaryExpr(std::unique_ptr<Expr> left, Token op, std::unique_ptr<Expr> right)
        : left(std::move(left)), op(op), right(std::move(right)) {}
};

struct UnaryExpr : Expr {
    Token op;
    std::unique_ptr<Expr> right;
    UnaryExpr(Token op, std::unique_ptr<Expr> right)
        : op(op), right(std::move(right)) {}
};

struct LiteralExpr : Expr {
    Token value; // Holds the token with the literal value
    LiteralExpr(Token value) : value(value) {}
};

struct VariableExpr : Expr {
    Token name;
    VariableExpr(Token name) : name(name) {}
};

struct AssignmentExpr : Expr {
    Token name;
    std::unique_ptr<Expr> value; // The newly assigned value
    AssignmentExpr(Token name, std::unique_ptr<Expr> value)
        : name(name), value(std::move(value)) {}
};

struct ListLiteralExpr : Expr {
    std::vector<std::unique_ptr<Expr>> elements;
    ListLiteralExpr(std::vector<std::unique_ptr<Expr>> elements)
        : elements(std::move(elements)) {}
};

struct CallExpr : Expr {
    std::unique_ptr<Expr> callee; // Usually a VariableExpr
    Token paren; // Closing paren for location
    std::vector<std::unique_ptr<Expr>> arguments;
    CallExpr(std::unique_ptr<Expr> callee, Token paren, std::vector<std::unique_ptr<Expr>> arguments)
        : callee(std::move(callee)), paren(paren), arguments(std::move(arguments)) {}
};

// A method call like xs.at(i) is a call where key is "at" and object is "xs".
struct MethodCallExpr : Expr {
    std::unique_ptr<Expr> object;
    Token name; // Method name
    std::vector<std::unique_ptr<Expr>> arguments;
    MethodCallExpr(std::unique_ptr<Expr> object, Token name, std::vector<std::unique_ptr<Expr>> arguments)
        : object(std::move(object)), name(name), arguments(std::move(arguments)) {}
};

// --- Statements ---

struct Stmt {
    virtual ~Stmt() = default;
};

struct ExprStmt : Stmt {
    std::unique_ptr<Expr> expression;
    ExprStmt(std::unique_ptr<Expr> expression) : expression(std::move(expression)) {}
};

struct BreakStmt : Stmt {
    Token keyword;
    BreakStmt(Token keyword) : keyword(keyword) {}
};

struct ContinueStmt : Stmt {
    Token keyword;
    ContinueStmt(Token keyword) : keyword(keyword) {}
};

struct ReturnStmt : Stmt {
    Token keyword;
    std::unique_ptr<Expr> value; // Can be null for 'return;'
    ReturnStmt(Token keyword, std::unique_ptr<Expr> value)
        : keyword(keyword), value(std::move(value)) {}
};

struct LetStmt : Stmt {
    Token name;
    std::unique_ptr<Type> type; // Explicit type required
    std::unique_ptr<Expr> initializer;
    bool isConst;
    LetStmt(Token name, std::unique_ptr<Type> type, std::unique_ptr<Expr> initializer, bool isConst)
        : name(name), type(std::move(type)), initializer(std::move(initializer)), isConst(isConst) {}
};

struct BlockStmt : Stmt {
    std::vector<std::unique_ptr<Stmt>> statements;
    BlockStmt(std::vector<std::unique_ptr<Stmt>> statements)
        : statements(std::move(statements)) {}
};

struct IfStmt : Stmt {
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> thenBranch;
    std::unique_ptr<Stmt> elseBranch; // Can be null
    IfStmt(std::unique_ptr<Expr> condition, std::unique_ptr<Stmt> thenBranch, std::unique_ptr<Stmt> elseBranch)
        : condition(std::move(condition)), thenBranch(std::move(thenBranch)), elseBranch(std::move(elseBranch)) {}
};

struct RepeatStmt : Stmt {
    Token iterator;
    std::unique_ptr<Expr> start;
    std::unique_ptr<Expr> end;
    std::unique_ptr<Expr> step; // Can be null if default, but parser should probably fill it? Or Codegen.
                                // Instruction says default 1.
    std::unique_ptr<Stmt> body;
    RepeatStmt(Token iterator, std::unique_ptr<Expr> start, std::unique_ptr<Expr> end, std::unique_ptr<Expr> step, std::unique_ptr<Stmt> body)
        : iterator(iterator), start(std::move(start)), end(std::move(end)), step(std::move(step)), body(std::move(body)) {}
};

struct FunctionStmt : Stmt {
    Token name;
    struct Param {
        Token name;
        std::unique_ptr<Type> type;
    };
    std::vector<Param> params;
    std::unique_ptr<Type> returnType;
    std::vector<std::unique_ptr<Stmt>> body;
    FunctionStmt(Token name, std::vector<Param> params, std::unique_ptr<Type> returnType, std::vector<std::unique_ptr<Stmt>> body)
        : name(name), params(std::move(params)), returnType(std::move(returnType)), body(std::move(body)) {}
};

} // namespace rox

#endif // ROX_AST_H
