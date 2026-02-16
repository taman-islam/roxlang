#ifndef ROX_CODEGEN_H
#define ROX_CODEGEN_H

#include <vector>
#include <memory>
#include <string>
#include <sstream>
#include "ast.h"
#include <unordered_map>
#include <unordered_set>

namespace rox {

class Codegen {
public:
    Codegen(const std::vector<std::unique_ptr<Stmt>>& statements);
    std::string generate();

private:
    const std::vector<std::unique_ptr<Stmt>>& statements;
    std::stringstream out;
    int indentLevel = 0;
    std::string currentFunctionName = "";

    struct VarInfo {
        Type* type;
        bool isProvenOk;
    };

    using Scope = std::unordered_map<std::string, VarInfo>;
    std::vector<Scope> scopes;
    std::unordered_set<std::string> iteratedVars; // collections currently being iterated
    std::unordered_map<std::string, TypeDefStmt*> typeRegistry; // user-defined types

    void enterScope();
    void exitScope();
    void declareVar(const std::string& name, Type* type);
    VarInfo* resolveVar(const std::string& name);
    void refineVar(const std::string& name);
    void invalidateVar(const std::string& name);

    void emitIndent();
    void emit(const std::string& s);
    void emitLine(const std::string& s);
    void emitPreamble();
    std::string sanitize(const std::string& name);

    void genStmt(Stmt* stmt);
    void genExpr(Expr* expr);
    void genType(Type* type);
    std::unique_ptr<Type> inferType(Expr* expr);

    // Helpers for dispatch
    void genBlock(BlockStmt* stmt);
    void genIf(IfStmt* stmt);
    void genFor(ForStmt* stmt);
    void genFunction(FunctionStmt* stmt);
    void genReturn(ReturnStmt* stmt);
    void genBreak(BreakStmt* stmt);
    void genContinue(ContinueStmt* stmt);
    void genLet(LetStmt* stmt);
    void genExprStmt(ExprStmt* stmt);

    void genBinary(BinaryExpr* expr);
    void genLogical(LogicalExpr* expr);
    void genUnary(UnaryExpr* expr);
    void genLiteral(LiteralExpr* expr);
    void genVariable(VariableExpr* expr);
    void genAssignment(AssignmentExpr* expr);
    void genCall(CallExpr* expr);
    void genMethodCall(MethodCallExpr* expr);
    void genListLiteral(ListLiteralExpr* expr);
    void genTypeDef(TypeDefStmt* stmt);
    void genRecordInit(RecordInitExpr* expr);
    void genFieldAccess(FieldAccessExpr* expr);
    void genFieldAssign(FieldAssignExpr* expr);
    void genDefault(DefaultExpr* expr);
};

} // namespace rox

#endif // ROX_CODEGEN_H
