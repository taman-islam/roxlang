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

    // Helper for string literals
    out << "class RoxString {\n";
    out << "public:\n";
    out << "    std::string val;\n";
    out << "    RoxString(const char* s) : val(s) {}\n";
    out << "    RoxString(std::string s) : val(std::move(s)) {}\n";
    out << "    RoxString() = default;\n";
    out << "\n";
    out << "    num size() const { return (num)val.size(); }\n";
    out << "    bool operator==(const RoxString& other) const { return val == other.val; }\n";
    out << "    bool operator!=(const RoxString& other) const { return val != other.val; }\n";
    out << "};\n";
    out << "\n";
    out << "std::ostream& operator<<(std::ostream& os, const RoxString& s) {\n";
    out << "    return os << s.val;\n";
    out << "}\n";
    out << "\n";
    out << "RoxString rox_str(const char* s) {\n";
    out << "    return RoxString(s);\n";
    out << "}\n";
    out << "\n";
    out << "// I/O\n";
    out << "std::ostream& operator<<(std::ostream& os, const std::vector<char>& s) {\n";
    out << "    for (char c : s) os << c;\n";
    out << "    return os;\n";
    out << "}\n";
    out << "\n";
    out << "template<typename... Args>\n";
    out << "None print(const Args&... args) {\n";
    out << "    ((std::cout << args), ...);\n";
    out << "    return none;\n";
    out << "}\n";
    out << "\n";
    out << "\n";
    out << "// List access\n";
    out << "template<typename T>\n";
    out << "rox_result<T> rox_at(const std::vector<T>& xs, num i) {\n";
    out << "    if (i < 0 || i >= (num)xs.size()) return error<T>(1); // index_out_of_range\n";
    out << "    return ok(xs[i]);\n";
    out << "}\n";
    out << "\n";
    out << "// String access\n";
    out << "rox_result<char> rox_at(const RoxString& s, num i) {\n";
    out << "    if (i < 0 || i >= s.size()) return error<char>(1);\n";
    out << "    return ok(s.val[i]);\n";
    out << "}\n";
    out << "\n";
    out << "// String to List\n";
    out << "std::vector<char> rox_to_list(const RoxString& s) {\n";
    out << "    return std::vector<char>(s.val.begin(), s.val.end());\n";
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
    out << "// Dictionary Hash for RoxString\n";
    out << "namespace std {\n";
    out << "    template <> struct hash<RoxString> {\n";
    out << "        size_t operator()(const RoxString& s) const {\n";
    out << "            return hash<string>()(s.val);\n";
    out << "        }\n";
    out << "    };\n";
    out << "}\n";
    out << "\n";
    out << "// Dictionary Access\n";
    out << "template<typename K, typename V>\n";
    out << "rox_result<V> rox_at(const std::unordered_map<K, V>& dict, K key) {\n";
    out << "    auto it = dict.find(key);\n";
    out << "    if (it == dict.end()) return error<V>(2); // key_not_found\n";
    out << "    return ok(it->second);\n";
    out << "}\n";
    out << "\n";
    out << "// Dictionary Set\n";
    out << "template<typename K, typename V>\n";
    out << "void rox_set(std::unordered_map<K, V>& dict, K key, V val) {\n";
    out << "    dict.insert_or_assign(key, val);\n";
    out << "}\n";
    out << "\n";
    out << "// Dictionary Remove\n";
    out << "template<typename K, typename V>\n";
    out << "void rox_remove(std::unordered_map<K, V>& dict, K key) {\n";
    out << "    dict.erase(key);\n";
    out << "}\n";
    out << "\n";
    out << "// Dictionary Has\n";
    out << "template<typename K, typename V>\n";
    out << "bool rox_has(const std::unordered_map<K, V>& dict, K key) {\n";
    out << "    return dict.find(key) != dict.end();\n";
    out << "}\n";
    out << "\n";
    out << "\n";

    out << "num32 num32_abs(num32 x) { return std::abs(x); }\n";
    out << "num32 num32_min(num32 x, num32 y) { return std::min(x, y); }\n";
    out << "num32 num32_max(num32 x, num32 y) { return std::max(x, y); }\n";
    out << "rox_result<num32> num32_pow(num32 base, num32 exp) {\n";
    out << "    if (exp < 0) return error<num32>(10); // invalid_argument\n";
    out << "    num32 res = 1;\n";
    out << "    for (int i = 0; i < exp; ++i) res *= base;\n";
    out << "    return ok(res);\n";
    out << "}\n";
    out << "\n";
    out << "num num_abs(num x) { return std::abs(x); }\n";
    out << "num num_min(num x, num y) { return std::min(x, y); }\n";
    out << "num num_max(num x, num y) { return std::max(x, y); }\n";
    out << "rox_result<num> num_pow(num base, num exp) {\n";
    out << "    if (exp < 0) return error<num>(10); // invalid_argument\n";
    out << "    num res = 1;\n";
    out << "    for (int i = 0; i < exp; ++i) res *= base;\n";
    out << "    return ok(res);\n";
    out << "}\n";
    out << "\n";
    out << "double float_abs(double x) { return std::abs(x); }\n";
    out << "double float_min(double x, double y) { return std::min(x, y); }\n";
    out << "double float_max(double x, double y) { return std::max(x, y); }\n";
    out << "double float_pow(double x, double y) { return std::pow(x, y); }\n";
    out << "rox_result<double> float_sqrt(double x) {\n";
    out << "    if (x < 0) return error<double>(10); // invalid_argument\n";
    out << "    return ok(std::sqrt(x));\n";
    out << "}\n";
    out << "double float_sin(double x) { return std::sin(x); }\n";
    out << "double float_cos(double x) { return std::cos(x); }\n";
    out << "double float_tan(double x) { return std::tan(x); }\n";
    out << "rox_result<double> float_log(double x) {\n";
    out << "    if (x <= 0) return error<double>(10); // invalid_argument\n";
    out << "    return ok(std::log(x));\n";
    out << "}\n";
    out << "double float_exp(double x) { return std::exp(x); }\n";
    out << "double float_floor(double x) { return std::floor(x); }\n";
    out << "double float_ceil(double x) { return std::ceil(x); }\n";

    out << "std::vector<char> numToString(num n) {\n";
    out << "    std::string s = std::to_string(n);\n";
    out << "    return std::vector<char>(s.begin(), s.end());\n";
    out << "}\n";
    out << "\n";
    out << "std::vector<char> charToString(char c) {\n";
    out << "    return {c};\n";
    out << "}\n";
    out << "\n";
    out << "std::string any_to_string(num n) { return std::to_string(n); }\n";
    out << "std::string any_to_string(int n) { return std::to_string(n); }\n";
    out << "std::string any_to_string(double d) { return std::to_string(d); }\n";
    out << "std::string any_to_string(char c) { return std::string(1, c); }\n";
    out << "std::string any_to_string(bool b) { return b ? \"true\" : \"false\"; }\n";
    out << "std::string any_to_string(const RoxString& s) { return s.val; }\n";
    out << "\n";
    out << "template <typename T>\n";
    out << "std::vector<char> listToString(const std::vector<T>& list) {\n";
    out << "    std::string s = \"[\";\n";
    out << "    for (size_t i = 0; i < list.size(); ++i) {\n";
    out << "        if (i > 0) s += \", \";\n";
    out << "        s += any_to_string(list[i]);\n";
    out << "    }\n";
    out << "    s += \"]\";\n";
    out << "    return std::vector<char>(s.begin(), s.end());\n";
    out << "}\n";
    out << "\n";

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
        else if (s == "string") out << "RoxString";
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
    out << " " << stmt->name.lexeme;

    if (!stmt->initializer) {
        // No initializer -> default initialization
        out << "{};";
        out << "\n";
        return;
    }

    out << " = ";

    // Optimized handling for ListLiteralExpr to ensure std::vector<T> is explicitly constructed
    // This fixes issues with empty lists [] where std::vector{} (CTAD) fails.
    if (auto* listLit = dynamic_cast<ListLiteralExpr*>(stmt->initializer.get())) {
        if (auto* listType = dynamic_cast<ListType*>(stmt->type.get())) {
             out << "std::vector<";
             genType(listType->elementType.get());
             out << ">{";
             for (size_t i = 0; i < listLit->elements.size(); ++i) {
                 if (i > 0) out << ", ";
                 genExpr(listLit->elements[i].get());
             }
             out << "}";
             out << ";\n";
             return;
        }
    }

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
    if (expr->op.type == TokenType::NOT) {
        out << "(!";
    } else {
        out << "(" << expr->op.lexeme;
    }
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
    } else if (method == "toList") {
        out << "rox_to_list(";
        genExpr(expr->object.get());
        out << ")";
    } else if (method == "append") {
        genExpr(expr->object.get());
        out << ".push_back(";
        if (!expr->arguments.empty()) genExpr(expr->arguments[0].get());
        out << ")";
    } else if (method == "pop") {
        genExpr(expr->object.get());
        out << ".pop_back()";
    } else if (method == "set") {
        out << "rox_set(";
        genExpr(expr->object.get());
        out << ", ";
        genExpr(expr->arguments[0].get());
        out << ", ";
        genExpr(expr->arguments[1].get());
        out << ")";
    } else if (method == "remove") {
        out << "rox_remove(";
        genExpr(expr->object.get());
        out << ", ";
        genExpr(expr->arguments[0].get());
        out << ")";
    } else if (method == "has") {
        out << "rox_has(";
        genExpr(expr->object.get());
        out << ", ";
        genExpr(expr->arguments[0].get());
        out << ")";
    } else if (method == "size") {
        // cast to num for strict typing
        out << "((num)";
        genExpr(expr->object.get());
        out << ".size())";
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
