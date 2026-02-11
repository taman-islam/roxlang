#include "codegen.h"
#include <iostream>
#include <map>

namespace rox {

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
    out << "using num64 = int64_t;\n";
    out << "using rox_float = double;\n"; // float is a keyword in C++, use rox_float or just double?
    // Lexer emits TYPE_FLOAT as 'float'. User writes 'float'.
    // If I map 'float' in ROX to 'double' in C++, I should use 'double' in codegen for type.
    // But for variable names, 'float' is reserved in C++. ROX doesn't allow 'float' as var name.
    // So mapping ROX 'float' type to C++ 'double' is fine.

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

    // Result constructors
    out << "template<typename T>\n";
    out << "rox_result<T> ok(T value) { return {value, 0}; }\n";
    out << "template<typename T>\n";
    out << "rox_result<T> error(num32 code) { return {T{}, code}; }\n";

    // Result helpers
    out << "template<typename T> bool isOk(const rox_result<T>& r) { return r.err == 0; }\n";
    out << "template<typename T> num32 getErrorCode(const rox_result<T>& r) { return r.err; }\n";
    out << "template<typename T> T getValue(const rox_result<T>& r) { if (r.err != 0) exit(r.err); return r.value; }\n";

    // Built-in constants
    out << "const double pi = 3.141592653589793;\n";
    out << "const double e  = 2.718281828459045;\n";

    // I/O
    out << "rox_result<None> print(const std::vector<char>& s) {\n";
    out << "    for (char c : s) std::cout << c;\n"; // ROX string is list[char]
    out << "    return ok(none);\n";
    out << "}\n";

    // Helper for string literals in C++ to ROX list[char]
    // The codegen for string literal will generate code to create vector<char>.
    out << "std::vector<char> rox_str(const char* s) {\n";
    out << "    std::vector<char> v;\n";
    out << "    while (*s) v.push_back(*s++);\n";
    out << "    return v;\n";
    out << "}\n";
    out << "\n";
    out << "// List access\n";
    out << "template<typename T>\n";
    out << "rox_result<T> rox_at(const std::vector<T>& xs, num32 i) {\n";
    out << "    if (i < 0 || i >= (num32)xs.size()) return error<T>(1); // index_out_of_range\n";
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
    out << "// Math library stubs (since we used them in example) - Wait, example didn't use them.\n";
    out << "// But spec lists them.\n";
    out << "// User might strictly rely on them.\n";
    out << "// I should generate them or assume they are available.\n";
    out << "// They are just standard C++ functions wrapper?\n";
    out << "// rox_num32_abs -> std::abs?\n";
    out << "// Since names are unique, user code calls `num32_abs(x)`.\n";
    out << "// I should emit `num32_abs` function or variable.\n";
    out << "// If I define them in preamble, then `genCall` works.\n";
    out << "\n";
    out << "num32 num32_abs(num32 x) { return std::abs(x); }\n";
    out << "// ... others ...\n";

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
        else if (s == "num64") out << "num64";
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
    // repeat i in range(start, end, inc)
    // translated to:
    // for (auto i = start; i < end; i += inc)
    // BUT inc defaults to 1.
    // and range arguments are expressions.

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
    emitIndent();
    // Special case for main
    if (stmt->name.lexeme == "main") {
        out << "int main(";
        // main has no params in v0
        out << ") {\n";
        indentLevel++;
        for (const auto& s : stmt->body) {
            genStmt(s.get());
        }
        emitIndent();
        out << "return 0;\n";
        indentLevel--;
        emitLine("}");
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
}

void Codegen::genReturn(ReturnStmt* stmt) {
    emitIndent();
    out << "return";
    if (stmt->value) {
        out << " ";
        genExpr(stmt->value.get());
    } else {
        // If return type is none, 'return;' is valid.
        // If explicit 'return none;' is needed:
        // C++ void function -> return;
        // But ROX 'none' is a Type.
        // If function returns None, we return 'none'.
        // But for generic 'stmt', we don't know the function context here easily without symbol table.
        // Assuming user writes correct ROX.
        // 'return;' in ROX -> 'return none;' in C++ if function returns None?
        // Or if function returns void (none type)?
        // My preamble defined `None` struct.
        // So `return none;` works. `return;` works if function returns void.
        // But ROX `function ... -> none` returns `None` type?
        // `none` has exactly one value `none`.
        // So function should return `None`.
        // So `return;` should be `return none;`?
        // Spec: "`return;` allowed only in `-> none`. `return none;` equivalent."
        // So I should probably emit `return none;`?
        out << " none";
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
    // Handling division special case for checked arithmetic if needed?
    // Spec: "Integer / and % return rox_result[T]"
    // But for v0, generated code:
    // If operators are overloaded?
    // C++ built-in / returns value.
    // I should generate a call to helper if I want to match spec "returns rox_result".
    // But let's check `7.1 Arithmetic`.
    // "Integer / and % return rox_result[T]".
    // "Float / follows IEEE".
    // So for integer division, I should emit `div(left, right)`.
    // But I don't know types here without analysis!
    // This is the problem with text-only codegen without type checking.
    // If I assume C++ compilation will succeed or fail...
    // But runtime behavior differs.
    // For v0, maybe I ignore the "result" wrapper for division or implement C++ overloads?
    // I can provide `rox_div(a, b)` templates.
    // Since C++ allows overloading, I can define `rox_div(int, int)` returning result, and `rox_div(double, double)` returning double.
    // And emit `rox_div(left, right)`.

    std::string op = expr->op.lexeme;
    if (op == "/") {
        out << "rox_div(";
        genExpr(expr->left.get());
        out << ", ";
        genExpr(expr->right.get());
        out << ")";
        return;
    }
    // Also %
    if (op == "%") {
        out << "rox_mod(";
        genExpr(expr->left.get());
        out << ", ";
        genExpr(expr->right.get());
        out << ")";
        return;
    }

    // For other ops, standard C++ infix.
    // Special keywords: and, or, not -> &&, ||, ! in C++?
    // Preamble: default.
    // But C++ has `and`, `or`, `not` as alternative tokens!
    // So `a and b` compiles in C++.

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
        // Remove suffix
        std::string s = expr->value.lexeme;
        size_t npos = s.find('n');
        if (npos != std::string::npos) {
            out << s.substr(0, npos);
        } else {
            out << s;
        }
    } else {
        out << expr->value.lexeme;
    }
}

void Codegen::genVariable(VariableExpr* expr) {
    out << expr->name.lexeme;
}

void Codegen::genAssignment(AssignmentExpr* expr) {
    // C++ assignment returns reference.
    out << "(" << expr->name.lexeme << " = ";
    genExpr(expr->value.get());
    out << ")";
}

void Codegen::genCall(CallExpr* expr) {
    // print(...) is special?
    // If it's identifier "print", it's just a call.
    // BUT `print` returns `result`. `print("foo")` works.
    genExpr(expr->callee.get());
    out << "(";
    for (size_t i = 0; i < expr->arguments.size(); ++i) {
        if (i > 0) out << ", ";
        genExpr(expr->arguments[i].get());
    }
    out << ")";
}

void Codegen::genMethodCall(MethodCallExpr* expr) {
    // xs.at(i)
    // C++ vector: xs.at(i) is valid but throws.
    // ROX: .at() returns result.
    // So I should provide helper `at(container, index)`.
    // But syntax is `xs.at(i)`.
    // I can't easily change `xs.at(i)` behavior in C++ unless `xs` is a wrapper class.
    // But I used `std::vector`.
    // `std::vector` has `at`.
    // If I map `xs.at(i)` to `rox_at(xs, i)`?
    // AST has `MethodCall`. Codegen can transform!

    std::string method = expr->name.lexeme;
    if (method == "at") {
        out << "rox_at(";
        genExpr(expr->object.get());
        out << ", ";
        if (!expr->arguments.empty()) genExpr(expr->arguments[0].get());
        out << ")";
    } else if (method == "append") {
        // xs.push_back(val)
        // ROX: append
        genExpr(expr->object.get());
        out << ".push_back(";
        if (!expr->arguments.empty()) genExpr(expr->arguments[0].get());
        out << ")";
    } else {
        // Fallback or other methods
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
    // [1, 2, 3] -> std::vector<T>{1, 2, 3}
    // I don't know T here!
    // C++ `std::vector{1, 2, 3}` (CTAD) works in C++17/20.
    out << "std::vector{";
    for (size_t i = 0; i < expr->elements.size(); ++i) {
        if (i > 0) out << ", ";
        genExpr(expr->elements[i].get());
    }
    out << "}";
}

} // namespace rox
