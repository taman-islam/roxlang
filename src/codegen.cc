#include "codegen.h"
#include <iostream>
#include <map>

namespace rox {

// Runtime Helpers
// is_ok, get_value, and print functions moved to emitPreamble


Codegen::Codegen(const std::vector<std::unique_ptr<Stmt>>& statements) : statements(statements) {}

std::string Codegen::generate() {
    emitPreamble();
    for (const auto& stmt : statements) {
        genStmt(stmt.get());
    }
    return out.str();
}

void Codegen::emitIndent() {
    for (int i = 0; i < indentLevel; ++i) out << "  ";
}

void Codegen::emit(const std::string& s) {
    if (out.str().length() > 0 && out.str().back() == '\n') emitIndent();
    out << s;
}

void Codegen::emitLine(const std::string& s) {
    emitIndent();
    out << s << "\n";
}

void Codegen::emitPreamble() {
    out << "#include <iostream>\n";
    out << "#include <vector>\n";
    out << "#include <unordered_map>\n";
    out << "#include <string>\n";
    out << "#include <cmath>\n";
    out << "#include <numeric>\n";
    out << "#include <variant>\n"; // For possible future use or result
    out << "#include <cstdint>\n";

    out << "\n// ROX Runtime\n";
    out << "using num32 = int32_t;\n";
    out << "using num = int64_t;\n";
    out << "using rox_float = double;\n";

    out << "using rox_char = char;\n";
    out << "using rox_bool = bool;\n";

    out << "struct None { bool operator==(const None&) const { return true; } };\n";
    out << "const None none = {};\n";

    // Result type
    out << "template<typename T>\n";
    out << "struct rox_result {\n";
    out << "    T value;\n";
    out << "    num32 err;\n";
    out << "};\n";

    // Runtime Helpers
    out << "template<typename T>\n";
    out << "bool isOk(rox_result<T> r) {\n";
    out << "    return r.err == 0;\n";
    out << "}\n";

    out << "template<typename T>\n";
    out << "T getValue(rox_result<T> r) {\n";
    out << "    if (r.err != 0) {\n";
    out << "        std::cerr << \"Called getValue on runtime error result!\" << std::endl;\n";
    out << "        exit(r.err);\n";
    out << "    }\n";
    out << "    return r.value;\n";
    out << "}\n";


    out << "void print_loop(num32 n) {\n";
    out << "    for (int i = 0; i < n; ++i) {\n";
    out << "        std::cout << \"Hello, World!\" << std::endl;\n";
    out << "    }\n";
    out << "}\n";

    out << "// Result constructors\n";
    out << "template<typename T>\n";
    out << "rox_result<T> ok(T value) { return {value, 0}; }\n";
    out << "template<typename T>\n";
    out << "rox_result<T> error(num32 code) { return {T{}, code}; }\n";


    // Built-in constants
    out << "const double pi = 3.141592653589793;\n";
    out << "const double e  = 2.718281828459045;\n";

    // I/O
    out << "None print(const std::vector<char>& s) {\n";
    out << "    for (char c : s) std::cout << c;\n"; // ROX string is list[char]
    out << "    return none;\n";
    out << "}\n";

    // Helper for string literals in C++ to ROX list[char]
    out << "std::vector<char> rox_str(const char* s) {\n";
    out << "    std::vector<char> v;\n";
    out << "    while (*s) v.push_back(*s++);\n";
    out << "    return v;\n";
    out << "}\n";
    out << "\n";
    // List access
    out << "template<typename T>\n";
    out << "rox_result<T> rox_at(const std::vector<T>& xs, num i) {\n";
    out << "    if (i < 0 || i >= (num)xs.size()) return error<T>(1); // index_out_of_range\n";
    out << "    return ok(xs[i]);\n";
    out << "}\n";
    out << "\n";
    out << "// Division\n";
    out << "template<typename T>\n";
    out << "rox_result<T> rox_div(T a, T b) {\n";
    out << "    if (b == 0) return error<T>(3); // division_by_zero\n";
    out << "    return ok(a / b);\n";
    out << "}\n";
    out << "\n";
    out << "// Modulo\n";
    out << "template<typename T>\n";
    out << "rox_result<T> rox_mod(T a, T b) {\n";
    out << "    if (b == 0) return error<T>(3); // division_by_zero\n";
    out << "    return ok(a % b);\n";
    out << "}\n";
    out << "\n";

    out << "num32 num32_abs(num32 x) { return std::abs(x); }\n";

    out << "\n// End Runtime\n\n";
}

void Codegen::genStmt(Stmt* stmt) {
    if (auto* s = dynamic_cast<BlockStmt*>(stmt)) genBlock(s);
    else if (auto* s = dynamic_cast<IfStmt*>(stmt)) genIf(s);
    else if (auto* s = dynamic_cast<RepeatStmt*>(stmt)) genRepeat(s);
    else if (auto* s = dynamic_cast<FunctionStmt*>(stmt)) genFunction(s);
    else if (auto* s = dynamic_cast<ReturnStmt*>(stmt)) genReturn(s);
    else if (auto* s = dynamic_cast<LetStmt*>(stmt)) genLet(s);
    else if (auto* s = dynamic_cast<ExprStmt*>(stmt)) genExprStmt(s);
    else emitLine("// Unknown statement");
}

void Codegen::genExpr(Expr* expr) {
    if (auto* e = dynamic_cast<BinaryExpr*>(expr)) genBinary(e);
    else if (auto* e = dynamic_cast<LogicalExpr*>(expr)) genLogical(e);
    else if (auto* e = dynamic_cast<UnaryExpr*>(expr)) genUnary(e);
    else if (auto* e = dynamic_cast<LiteralExpr*>(expr)) genLiteral(e);
    else if (auto* e = dynamic_cast<VariableExpr*>(expr)) genVariable(e);
    else if (auto* e = dynamic_cast<AssignmentExpr*>(expr)) genAssignment(e);
    else if (auto* e = dynamic_cast<CallExpr*>(expr)) genCall(e);
    else if (auto* e = dynamic_cast<MethodCallExpr*>(expr)) genMethodCall(e);
    else if (auto* e = dynamic_cast<ListLiteralExpr*>(expr)) genListLiteral(e);
    else emit("/* Unknown expr */");
}

void Codegen::genType(Type* type) {
    if (auto* t = dynamic_cast<PrimitiveType*>(type)) {
        std::string s = t->token.lexeme;
        if (s == "num32") out << "num32";
        else if (t->token.type == TokenType::TYPE_NUM) { // Corrected from `kind == TokenType::TYPE_NUM`
            out << "num";
        }
        else if (s == "float") out << "double";
        else if (s == "bool") out << "bool";
        else if (s == "char") out << "char";
        else if (s == "none") out << "None";
        else out << s; // Fallback
    } else if (auto* t = dynamic_cast<ListType*>(type)) {
        out << "std::vector<";
        genType(t->elementType.get());
        out << ">";
    } else if (auto* t = dynamic_cast<DictionaryType*>(type)) {
        out << "std::unordered_map<";
        genType(t->keyType.get());
        out << ", ";
        genType(t->valueType.get());
        out << ">";
    } else if (auto* t = dynamic_cast<RoxResultType*>(type)) {
        out << "rox_result<";
        genType(t->valueType.get());
        out << ">";
    }
}

void Codegen::genBlock(BlockStmt* stmt) {
    emitLine("{");
    indentLevel++;
    for (const auto& s : stmt->statements) {
        genStmt(s.get());
    }
    indentLevel--;
    emitLine("}");
}

void Codegen::genIf(IfStmt* stmt) {
    emitIndent();
    out << "if (";
    genExpr(stmt->condition.get());
    out << ") ";
    genStmt(stmt->thenBranch.get());
    if (stmt->elseBranch) {
        emitIndent();
        out << "else ";
        genStmt(stmt->elseBranch.get());
    }
}

void Codegen::genRepeat(RepeatStmt* stmt) {
    emitIndent();
    out << "for (auto " << stmt->iterator.lexeme << " = ";
    genExpr(stmt->start.get());
    out << "; " << stmt->iterator.lexeme << " < ";
    genExpr(stmt->end.get());
    out << "; " << stmt->iterator.lexeme << " += ";
    if (stmt->step) genExpr(stmt->step.get());
    else out << "1";
    out << ") ";
    genStmt(stmt->body.get());
}

void Codegen::genFunction(FunctionStmt* stmt) {
    std::string oldFunctionName = currentFunctionName;
    currentFunctionName = stmt->name.lexeme;

    emitIndent();
    // Special case for main
    if (stmt->name.lexeme == "main") {
        out << "int main(";
        out << ") {\n";
        indentLevel++;
        for (const auto& s : stmt->body) {
            genStmt(s.get());
        }
        emitIndent();
        out << "return 0;\n";
        indentLevel--;
        emitLine("}");
        currentFunctionName = oldFunctionName;
        return;
    }

    // Return Type
    genType(stmt->returnType.get());
    out << " " << stmt->name.lexeme << "(";

    for (size_t i = 0; i < stmt->params.size(); ++i) {
        if (i > 0) out << ", ";
        genType(stmt->params[i].type.get());
        out << " " << stmt->params[i].name.lexeme;
    }
    out << ") {\n";
    indentLevel++;
    for (const auto& s : stmt->body) {
        genStmt(s.get());
    }

    indentLevel--;
    emitLine("}");
    currentFunctionName = oldFunctionName;
}

void Codegen::genReturn(ReturnStmt* stmt) {
    // If in main, we must return 0.
    // If there is a return value (like `return none;`), we evaluate it if needed, but discard result for C++ main.
    // Spec says main returns int (0 if success).
    // ROX `main -> none` returns `None`.
    // Valid ROX `return none;` in main.
    // We transform `return none;` to `return 0;`.

    emitIndent();
    out << "return";
    if (currentFunctionName == "main") {
        if (stmt->value) {
            // Check if it's literal none
            bool isNone = false;
            if (auto* lit = dynamic_cast<LiteralExpr*>(stmt->value.get())) {
                 if (lit->value.type == TokenType::NONE) isNone = true;
            }

            if (isNone) {
                 out << " 0";
            } else {
                 out << " (";
                 genExpr(stmt->value.get());
                 out << ", 0)";
            }
        } else {
             out << " 0";
        }
    } else {
        if (stmt->value) {
            out << " ";
            genExpr(stmt->value.get());
        } else {
             out << " none";
        }
    }
    out << ";\n";
}

void Codegen::genLet(LetStmt* stmt) {
    emitIndent();
    if (stmt->isConst) out << "const ";
    genType(stmt->type.get());
    out << " " << stmt->name.lexeme << " = ";
    genExpr(stmt->initializer.get());
    out << ";\n";
}

void Codegen::genExprStmt(ExprStmt* stmt) {
    emitIndent();
    genExpr(stmt->expression.get());
    out << ";\n";
}

void Codegen::genBinary(BinaryExpr* expr) {
    std::string op = expr->op.lexeme;
    if (op == "/") {
        out << "rox_div(";
        genExpr(expr->left.get());
        out << ", ";
        genExpr(expr->right.get());
        out << ")";
        return;
    }
    if (op == "%") {
        out << "rox_mod(";
        genExpr(expr->left.get());
        out << ", ";
        genExpr(expr->right.get());
        out << ")";
        return;
    }

    out << "(";
    genExpr(expr->left.get());
    out << " " << op << " ";
    genExpr(expr->right.get());
    out << ")";
}

void Codegen::genUnary(UnaryExpr* expr) {
    out << "(" << expr->op.lexeme;
    genExpr(expr->right.get());
    out << ")";
}

void Codegen::genLiteral(LiteralExpr* expr) {
    if (expr->value.type == TokenType::STRING) {
        out << "rox_str(" << expr->value.lexeme << ")";
    } else if (expr->value.type == TokenType::NUMBER_INT) {
        std::string s = expr->value.lexeme;
        size_t npos = s.find('n');
        if (npos != std::string::npos) {
            // has 'n' -> likely n32 (since we removed n64 parser support)
            // Just output the number part
            out << s.substr(0, npos);
        } else {
            // No suffix -> num64
            // Cast to (num) to ensure std::vector deduction picks up vector<num>
            // instead of vector<long long> (which might differ from num=int64_t=long on Mac)
            out << "((num)" << s << ")";
        }
    } else {
        out << expr->value.lexeme;
    }
}

void Codegen::genVariable(VariableExpr* expr) {
    out << expr->name.lexeme;
}

void Codegen::genAssignment(AssignmentExpr* expr) {
    out << "(" << expr->name.lexeme << " = ";
    genExpr(expr->value.get());
    out << ")";
}

void Codegen::genCall(CallExpr* expr) {
    genExpr(expr->callee.get());
    out << "(";
    for (size_t i = 0; i < expr->arguments.size(); ++i) {
        if (i > 0) out << ", ";
        genExpr(expr->arguments[i].get());
    }
    out << ")";
}

void Codegen::genMethodCall(MethodCallExpr* expr) {
    std::string method = expr->name.lexeme;
    if (method == "at") {
        out << "rox_at(";
        genExpr(expr->object.get());
        out << ", ";
        if (!expr->arguments.empty()) genExpr(expr->arguments[0].get());
        out << ")";
    } else if (method == "append") {
        genExpr(expr->object.get());
        out << ".push_back(";
        if (!expr->arguments.empty()) genExpr(expr->arguments[0].get());
        out << ")";
    } else if (method == "pop") {
        genExpr(expr->object.get());
        out << ".pop_back()";
    } else {
        genExpr(expr->object.get());
        out << "." << method << "(";
         for (size_t i = 0; i < expr->arguments.size(); ++i) {
            if (i > 0) out << ", ";
            genExpr(expr->arguments[i].get());
        }
        out << ")";
    }
}

void Codegen::genListLiteral(ListLiteralExpr* expr) {
    out << "std::vector{";
    for (size_t i = 0; i < expr->elements.size(); ++i) {
        if (i > 0) out << ", ";
        genExpr(expr->elements[i].get());
    }
    out << "}";
}

void Codegen::genLogical(LogicalExpr* expr) {
    out << "(";
    genExpr(expr->left.get());
    if (expr->op.type == TokenType::OR) out << " || ";
    else out << " && ";
    genExpr(expr->right.get());
    out << ")";
}

} // namespace rox
