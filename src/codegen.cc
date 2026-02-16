#include "codegen.h"
#include "lexer.h"
#include <iostream>
#include <unordered_map>
#include <unordered_set>

namespace rox {

// Runtime Helpers
// is_ok, get_value, and print functions moved to emitPreamble


Codegen::Codegen(const std::vector<std::unique_ptr<Stmt>>& statements) : statements(statements) {
    enterScope();
}

void Codegen::enterScope() {
    scopes.push_back({});
}

void Codegen::exitScope() {
    if (scopes.empty()) {
        std::cerr << "Internal Compiler Error: Unbalanced scope exit." << std::endl;
        exit(1);
    }
    scopes.pop_back();
}

void Codegen::declareVar(const std::string& name, Type* type) {
    if (scopes.empty()) {
        std::cerr << "Internal Compiler Error: declaration outside scope." << std::endl;
        exit(1);
    }
    scopes.back()[name] = {type, false};
}

auto Codegen::resolveVar(const std::string& name) -> VarInfo* {
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
        auto vit = it->find(name);
        if (vit != it->end()) {
            return &vit->second;
        }
    }
    return nullptr;
}

void Codegen::refineVar(const std::string& name) {
    VarInfo* info = resolveVar(name);
    if (info) {
        info->isProvenOk = true;
    }
}

void Codegen::invalidateVar(const std::string& name) {
    VarInfo* info = resolveVar(name);
    if (info) {
        info->isProvenOk = false;
    }
}

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
    out << "#include <functional>\n"; // For std::function literals

    out << "\n// ROX Runtime\n";
    // out << "using num = int64_t;\n"; // Removed usage of num

    out << "using rox_char = char;\n";
    out << "using rox_bool = bool;\n";

    out << "struct None { bool operator==(const None&) const { return true; } };\n";
    out << "const None none = {};\n";
    out << "\n";
    out << "    // Helper for string literals\n";
    out << "class RoxString {\n";
    out << "public:\n";
    out << "    std::string val;\n";
    out << "    RoxString(const char* s) : val(s) {}\n";
    out << "    RoxString(std::string s) : val(std::move(s)) {}\n";
    out << "    RoxString() = default;\n";
    out << "\n";
    out << "    int64_t size() const { return (int64_t)val.size(); }\n";
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

    // RoxRange iterable
    out << "struct RoxRange {\n";
    out << "    int64_t start_, end_, step_;\n";
    out << "    RoxRange(int64_t s, int64_t e, int64_t st) : start_(s), end_(e), step_(st) {\n";
    out << "        if (st == 0) { std::cerr << \"Runtime Error: range() step cannot be 0.\" << std::endl; exit(1); }\n";
    out << "    }\n";
    out << "    struct Iterator {\n";
    out << "        int64_t current, step, end;\n";
    out << "        int64_t operator*() const { return current; }\n";
    out << "        Iterator& operator++() { current += step; return *this; }\n";
    out << "        bool operator!=(const Iterator& o) const {\n";
    out << "            return step > 0 ? current < o.current : current > o.current;\n";
    out << "        }\n";
    out << "    };\n";
    out << "    Iterator begin() const { return {start_, step_, end_}; }\n";
    out << "    Iterator end() const { return {end_, step_, end_}; }\n";
    out << "};\n\n";

    // Result type
    out << "template<typename T>\n";
    out << "struct rox_result {\n";
    out << "    T value;\n";
    out << "    RoxString err;\n";
    out << "};\n";

    // Runtime Helpers
    out << "template<typename T>\n";
    out << "bool isOk(rox_result<T> r) {\n";
    out << "    return r.err.val.empty();\n";
    out << "}\n";

    out << "template<typename T>\n";
    out << "T getValue(rox_result<T> r) {\n";
    out << "    if (!r.err.val.empty()) {\n";
    out << "        std::cerr << \"Runtime Error: \" << r.err.val << std::endl;\n";
    out << "        exit(1);\n";
    out << "    }\n";
    out << "    return r.value;\n";
    out << "}\n";

    out << "template<typename T>\n";
    out << "RoxString getError(rox_result<T> r) {\n";
    out << "    return r.err;\n";
    out << "}\n";

    out << "void print_loop(int64_t n) {\n";
    out << "    for (int i = 0; i < n; ++i) {\n";
    out << "        std::cout << \"Hello, World!\" << std::endl;\n";
    out << "    }\n";
    out << "}\n";

    out << "// Result constructors\n";
    out << "template<typename T>\n";
    out << "rox_result<T> ok(T value) { return {value, RoxString(\"\")}; }\n";
    out << "template<typename T>\n";
    out << "rox_result<T> error(const char* msg) { return {T{}, RoxString(msg)}; }\n";


    // Built-in constants
    out << "const double pi = 3.141592653589793;\n";
    out << "const double e  = 2.718281828459045;\n";    out << "\n";
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
    out << "rox_result<T> rox_at(const std::vector<T>& xs, int64_t i) {\n";
    out << "    if (i < 0 || i >= (int64_t)xs.size()) return error<T>(\"Index out of bounds\");\n";
    out << "    return ok(xs[i]);\n";
    out << "}\n";
    out << "\n";
    out << "// List Set\n";
    out << "template<typename T>\n";
    out << "void rox_set(std::vector<T>& xs, int64_t i, T val) {\n";
    out << "    if (i < 0 || i >= (int64_t)xs.size()) {\n";
    out << "        std::cerr << \"Error: Index out of bounds in list.set\" << std::endl;\n";
    out << "        exit(1);\n";
    out << "    }\n";
    out << "    xs[i] = val;\n";
    out << "}\n";
    out << "\n";
    out << "// String access\n";
    out << "rox_result<char> rox_at(const RoxString& s, int64_t i) {\n";
    out << "    if (i < 0 || i >= s.size()) return error<char>(\"Index out of bounds\");\n";
    out << "    return ok(s.val[i]);\n";
    out << "}\n";
    out << "\n";

    out << "\n";
    out << "// Division\n";
    out << "template<typename T>\n";
    out << "rox_result<T> rox_div(T a, T b) {\n";
    out << "    if (b == 0) return error<T>(\"Division by zero\");\n";
    out << "    return ok(a / b);\n";
    out << "}\n";
    out << "\n";
    out << "// Modulo\n";
    out << "template<typename T>\n";
    out << "rox_result<T> rox_mod(T a, T b) {\n";
    out << "    if (b == 0) return error<T>(\"Division by zero\");\n";
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
    out << "rox_result<V> rox_get(const std::unordered_map<K, V>& dict, K key) {\n";
    out << "    auto it = dict.find(key);\n";
    out << "    if (it == dict.end()) return error<V>(\"Key not found\");\n";
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
    out << "// Dictionary Keys\n";
    out << "template<typename K, typename V>\n";
    out << "std::vector<K> rox_keys(const std::unordered_map<K, V>& dict) {\n";
    out << "    std::vector<K> keys;\n";
    out << "    keys.reserve(dict.size());\n";
    out << "    for (const auto& kv : dict) {\n";
    out << "        keys.push_back(kv.first);\n";
    out << "    }\n";
    out << "    return keys;\n";
    out << "}\n";
    out << "\n";

    out << "int64_t int64_abs(int64_t x) { return std::abs(x); }\n";
    out << "int64_t int64_min(int64_t x, int64_t y) { return std::min(x, y); }\n";
    out << "int64_t int64_max(int64_t x, int64_t y) { return std::max(x, y); }\n";
    out << "rox_result<int64_t> int64_pow(int64_t base, int64_t exp) {\n";
    out << "    if (exp < 0) return error<int64_t>(\"Negative exponent\");\n";
    out << "    int64_t res = 1;\n";
    out << "    for (int i = 0; i < exp; ++i) res *= base;\n";
    out << "    return ok(res);\n";
    out << "}\n";
    out << "\n";
    out << "double float64_abs(double x) { return std::abs(x); }\n";
    out << "double float64_min(double x, double y) { return std::min(x, y); }\n";
    out << "double float64_max(double x, double y) { return std::max(x, y); }\n";
    out << "double float64_pow(double x, double y) { return std::pow(x, y); }\n";
    out << "rox_result<double> float64_sqrt(double x) {\n";
    out << "    if (x < 0) return error<double>(\"Negative input for sqrt\");\n";
    out << "    return ok(std::sqrt(x));\n";
    out << "}\n";
    out << "double float64_sin(double x) { return std::sin(x); }\n";
    out << "double float64_cos(double x) { return std::cos(x); }\n";
    out << "double float64_tan(double x) { return std::tan(x); }\n";
    out << "rox_result<double> float64_log(double x) {\n";
    out << "    if (x <= 0) return error<double>(\"Non-positive input for log\");\n";
    out << "    return ok(std::log(x));\n";
    out << "}\n";
    out << "double float64_exp(double x) { return std::exp(x); }\n";
    out << "double float64_floor(double x) { return std::floor(x); }\n";
    out << "double float64_ceil(double x) { return std::ceil(x); }\n";
    out << "\n";
    out << "\n";
    out << "\n";

    out << "\n// End Runtime\n\n";
}

void Codegen::genStmt(Stmt* stmt) {
    if (!stmt) {
        return;
    }
    if (auto* s = dynamic_cast<BlockStmt*>(stmt)) genBlock(s);
    else if (auto* s = dynamic_cast<IfStmt*>(stmt)) genIf(s);
    else if (auto* s = dynamic_cast<ForStmt*>(stmt)) genFor(s);
    else if (auto* s = dynamic_cast<FunctionStmt*>(stmt)) genFunction(s);
    else if (auto* s = dynamic_cast<ReturnStmt*>(stmt)) genReturn(s);
    else if (auto* s = dynamic_cast<BreakStmt*>(stmt)) genBreak(s);
    else if (auto* s = dynamic_cast<ContinueStmt*>(stmt)) genContinue(s);
    else if (auto* s = dynamic_cast<LetStmt*>(stmt)) genLet(s);
    else if (auto* s = dynamic_cast<ExprStmt*>(stmt)) genExprStmt(s);
    else emitLine("// Unknown statement");
}

void Codegen::genBreak(BreakStmt* _unused_stmt) {
    emitLine("break;");
}

void Codegen::genContinue(ContinueStmt* _unused_stmt) {
    emitLine("continue;");
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
        if (t->token.type == TokenType::TYPE_INT64) {
            out << "int64_t";
        }
        else if (t->token.type == TokenType::TYPE_FLOAT64) out << "double";
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
    } else if (auto* t = dynamic_cast<FunctionType*>(type)) {
        out << "std::function<";
        genType(t->returnType.get());
        out << "(";
        for (size_t i = 0; i < t->paramTypes.size(); ++i) {
            if (i > 0) out << ", ";
            genType(t->paramTypes[i].get());
        }
        out << ")>";
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
    // Check if condition is 'isOk(var)'
    // If so, refine var in then branch
    std::string refinedVarName = "";
    if (auto* call = dynamic_cast<CallExpr*>(stmt->condition.get())) {
        if (auto* var = dynamic_cast<VariableExpr*>(call->callee.get())) {
             if (var->name.lexeme == "isOk" && call->arguments.size() == 1) {
                 if (auto* arg = dynamic_cast<VariableExpr*>(call->arguments[0].get())) {
                     refinedVarName = arg->name.lexeme;
                 }
             }
        }
    }

    emitIndent();
    out << "if (";
    genExpr(stmt->condition.get());
    out << ") ";

    // Enter scope for then branch to refine var
    enterScope();
    if (!refinedVarName.empty()) {
        VarInfo* outer = resolveVar(refinedVarName);
        if (outer) {
            scopes.back()[refinedVarName] = {outer->type, true};
        }
    }

    genStmt(stmt->thenBranch.get());
    exitScope();

    if (stmt->elseBranch) {
        emitIndent();
        out << "else ";
        genStmt(stmt->elseBranch.get());
        // No refinement in else
    }
}

void Codegen::genFor(ForStmt* stmt) {
    // Compile-time validation: check for literal step=0 in range() calls
    if (auto* call = dynamic_cast<CallExpr*>(stmt->iterable.get())) {
        auto* callee = dynamic_cast<VariableExpr*>(call->callee.get());
        if (callee && callee->name.lexeme == "range") {
            if (call->arguments.size() != 3) {
                std::cerr << "Error: range() requires exactly 3 arguments: range(start, end, step)." << std::endl;
                exit(1);
            }
            // Check for literal 0 step
            if (auto* lit = dynamic_cast<LiteralExpr*>(call->arguments[2].get())) {
                if (lit->value.type == TokenType::NUMBER_INT && lit->value.lexeme == "0") {
                    std::cerr << "Error: range() step cannot be 0." << std::endl;
                    exit(1);
                }
            }
        }
    }

    emitIndent();
    out << "for (auto " << sanitize(stmt->iterator.lexeme) << " : ";
    genExpr(stmt->iterable.get());
    out << ") ";
    genStmt(stmt->body.get());
}

void Codegen::genFunction(FunctionStmt* stmt) {
    std::string oldFunctionName = currentFunctionName;
    currentFunctionName = sanitize(stmt->name.lexeme);

    emitIndent();
    // Special case for main
    if (stmt->name.lexeme == "main") {
        out << "int main(";
        out << ") {\n";
        indentLevel++;
        emitIndent();
        out << "std::cout << std::boolalpha;\n";
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
    out << " " << sanitize(stmt->name.lexeme) << "(";

    for (size_t i = 0; i < stmt->params.size(); ++i) {
        if (i > 0) out << ", ";
        genType(stmt->params[i].type.get());
        out << " " << sanitize(stmt->params[i].name.lexeme);
    }
    out << ") {\n";
    indentLevel++;
    for (const auto& s : stmt->body) {
        genStmt(s.get());
    }

    // Implicit return for None types
    if (auto* t = dynamic_cast<PrimitiveType*>(stmt->returnType.get())) {
        if (t->token.lexeme == "none") {
            emitLine("return none;");
        }
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
    out << " " << sanitize(stmt->name.lexeme);

    declareVar(stmt->name.lexeme, stmt->type.get());

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
        // No suffix -> num64
        // Cast to (num) to ensure std::vector deduction picks up vector<num>
        // instead of vector<long long> (which might differ from num=int64_t=long on Mac)
        out << "((int64_t)" << s << ")";
    } else {
        out << expr->value.lexeme;
    }
}

void Codegen::genVariable(VariableExpr* expr) {
    out << sanitize(expr->name.lexeme);
}

void Codegen::genAssignment(AssignmentExpr* expr) {
    invalidateVar(expr->name.lexeme);
    out << "(" << sanitize(expr->name.lexeme) << " = ";
    genExpr(expr->value.get());
    out << ")";
}

void Codegen::genCall(CallExpr* expr) {
    // Check for unsafe getValue(var)
    if (auto* var = dynamic_cast<VariableExpr*>(expr->callee.get())) {
        if (var->name.lexeme == "getValue" && expr->arguments.size() == 1) {
            if (auto* arg = dynamic_cast<VariableExpr*>(expr->arguments[0].get())) {
                VarInfo* info = resolveVar(arg->name.lexeme);
                if (info && !info->isProvenOk) {
                    std::cerr << "Compile Error: getValue(" << arg->name.lexeme
                              << ") is unsafe. Variable '" << arg->name.lexeme
                              << "' is not proven to be Ok in this scope. "
                              << "Wrap it in 'if (isOk(" << arg->name.lexeme << ")) { ... }'."
                              << std::endl;
                    exit(1);
                }
            }
        }
    }

    // Intercept range() calls to emit RoxRange constructor
    if (auto* callee = dynamic_cast<VariableExpr*>(expr->callee.get())) {
        if (callee->name.lexeme == "range") {
            out << "RoxRange(";
            for (size_t i = 0; i < expr->arguments.size(); ++i) {
                if (i > 0) out << ", ";
                genExpr(expr->arguments[i].get());
            }
            out << ")";
            return;
        }
    }

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
    } else if (method == "getValue") {
       // Method syntax: x.getValue()
       // Not standard built-in function 'getValue(x)', but maybe supported via method call syntax if added?
       // The example code uses function call 'getValue(x)'.
       // However, if the user tries x.getValue(), we should check too if supported.
       // Current language spec says 'getValue(rox_result)'.
       // But if we support method syntax for it in future:
       if (auto* var = dynamic_cast<VariableExpr*>(expr->object.get())) {
            VarInfo* info = resolveVar(var->name.lexeme);
            if (info && !info->isProvenOk) {
                 std::cerr << "Compile Error: " << var->name.lexeme << ".getValue() is unsafe. "
                           << "Variable '" << var->name.lexeme << "' is not proven to be Ok in this scope."
                           << std::endl;
                 exit(1);
            }
       }
       out << "getValue(";
       genExpr(expr->object.get());
       out << ")";
    } else if (method == "get") {
        out << "rox_get(";
        genExpr(expr->object.get());
        out << ", ";
        if (!expr->arguments.empty()) genExpr(expr->arguments[0].get());
        out << ")";
    } else if (method == "append") {
        auto objType = inferType(expr->object.get());
        if (auto* listType = dynamic_cast<ListType*>(objType.get())) {
             if (expr->arguments.empty()) {
                 std::cerr << "Error: list.append expects 1 argument." << std::endl;
                 exit(1);
             }
             auto argType = inferType(expr->arguments[0].get());
             if (argType && argType->toString() != listType->elementType->toString()) {
                  std::cerr << "Type Error: List append type mismatch. Expected " << listType->elementType->toString()
                            << " but got " << argType->toString() << "." << std::endl;
                  exit(1);
             }
        }
        genExpr(expr->object.get());
        out << ".push_back(";
        if (!expr->arguments.empty()) genExpr(expr->arguments[0].get());
        out << ")";
    } else if (method == "pop") {
        genExpr(expr->object.get());
        out << ".pop_back()";
    } else if (method == "set") {
        // Semantic Analysis: Check for dictionary type mismatch
        auto objType = inferType(expr->object.get());
        if (auto* dictType = dynamic_cast<DictionaryType*>(objType.get())) {
             if (expr->arguments.size() < 2) {
                 std::cerr << "Error: dictionary.set expects 2 arguments." << std::endl;
                 exit(1);
             }
             auto keyType = inferType(expr->arguments[0].get());
             auto valType = inferType(expr->arguments[1].get());

             if (keyType && keyType->toString() != dictType->keyType->toString()) {
                  std::cerr << "Type Error: Dictionary key type mismatch. Expected " << dictType->keyType->toString()
                            << " but got " << keyType->toString() << "." << std::endl;
                  exit(1);
             }
             if (valType && valType->toString() != dictType->valueType->toString()) {
                  std::cerr << "Type Error: Dictionary value type mismatch. Expected " << dictType->valueType->toString()
                            << " but got " << valType->toString() << "." << std::endl;
                  exit(1);
             }
        }

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
        out << "((int64_t)";
        genExpr(expr->object.get());
        out << ".size())";
    } else if (method == "getKeys") {
        out << "rox_keys(";
        genExpr(expr->object.get());
        out << ")";
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

std::string Codegen::sanitize(const std::string& name) {
    if (name.empty()) return name;

    // Check if name is a keyword (like 'print', 'read_line', 'num32')
    if (Lexer::getKeywords().find(name) != Lexer::getKeywords().end()) {
        return name;
    }

    // Check if name is a preserved built-in or special name (like 'main', 'pi', 'isOk')
    if (Lexer::getBuiltins().find(name) != Lexer::getBuiltins().end()) {
        return name;
    }


    // Namespacing for user identifiers
    return "roxv26_" + name;
}

std::unique_ptr<Type> Codegen::inferType(Expr* expr) {
    if (!expr) return nullptr;

    if (auto* lit = dynamic_cast<LiteralExpr*>(expr)) {
        if (lit->value.type == TokenType::NUMBER_INT) return std::make_unique<PrimitiveType>(Token{TokenType::TYPE_INT64, "int64", lit->value.line});
        if (lit->value.type == TokenType::NUMBER_FLOAT) return std::make_unique<PrimitiveType>(Token{TokenType::TYPE_FLOAT64, "float64", lit->value.line});
        if (lit->value.type == TokenType::STRING) return std::make_unique<PrimitiveType>(Token{TokenType::TYPE_STRING, "string", lit->value.line});
        if (lit->value.type == TokenType::CHAR_LITERAL) return std::make_unique<PrimitiveType>(Token{TokenType::TYPE_CHAR, "char", lit->value.line});
        if (lit->value.type == TokenType::TRUE || lit->value.type == TokenType::FALSE) return std::make_unique<PrimitiveType>(Token{TokenType::TYPE_BOOL, "bool", lit->value.line});
        if (lit->value.type == TokenType::NONE) return std::make_unique<PrimitiveType>(Token{TokenType::NONE, "none", lit->value.line});
    }

    if (auto* var = dynamic_cast<VariableExpr*>(expr)) {
        VarInfo* info = resolveVar(var->name.lexeme);
        if (info && info->type) {
             return info->type->clone();
        }
    }

    return nullptr;
}

} // namespace rox

