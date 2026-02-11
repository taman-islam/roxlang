#include <iostream>
#include <vector>
#include <unordered_map>
#include <string>
#include <cmath>
#include <numeric>
#include <variant>
#include <cstdint>

// ROX Runtime
using num32 = int32_t;
using num64 = int64_t;
using rox_float = double;
using rox_char = char;
using rox_bool = bool;
struct None { bool operator==(const None&) const { return true; } };
const None none = {};
template<typename T>
struct rox_result {
    T value;
    num32 err;
};
template<typename T>
rox_result<T> ok(T value) { return {value, 0}; }
template<typename T>
rox_result<T> error(num32 code) { return {T{}, code}; }
template<typename T> bool isOk(const rox_result<T>& r) { return r.err == 0; }
template<typename T> num32 getErrorCode(const rox_result<T>& r) { return r.err; }
template<typename T> T getValue(const rox_result<T>& r) { if (r.err != 0) exit(r.err); return r.value; }
const double pi = 3.141592653589793;
const double e  = 2.718281828459045;
rox_result<None> print(const std::vector<char>& s) {
    for (char c : s) std::cout << c;
    return ok(none);
}
std::vector<char> rox_str(const char* s) {
    std::vector<char> v;
    while (*s) v.push_back(*s++);
    return v;
}

// List access
template<typename T>
rox_result<T> rox_at(const std::vector<T>& xs, num32 i) {
    if (i < 0 || i >= (num32)xs.size()) return error<T>(1); // index_out_of_range
    return ok(xs[i]);
}

// Division
template<typename T>
rox_result<T> rox_div(T a, T b) {
    if (b == 0) return error<T>(3); // division_by_zero
    return ok(a / b);
}

// Modulo
template<typename T>
rox_result<T> rox_mod(T a, T b) {
    if (b == 0) return error<T>(3); // division_by_zero
    return ok(a % b);
}

// Math library stubs (since we used them in example) - Wait, example didn't use them.
// But spec lists them.
// User might strictly rely on them.
// I should generate them or assume they are available.
// They are just standard C++ functions wrapper?
// rox_num32_abs -> std::abs?
// Since names are unique, user code calls `num32_abs(x)`.
// I should emit `num32_abs` function or variable.
// If I define them in preamble, then `genCall` works.

num32 num32_abs(num32 x) { return std::abs(x); }
// ... others ...

// End Runtime

int main() {
  num32 x = 10;
  print(rox_str("Hello, world!\n"));
  if ((x > 5))   {
    print(rox_str("x is greater than 5\n"));
  }
  print(rox_str("Counting: "));
  for (auto i = 0; i < 5; i += 1)   {
    print(rox_str("."));
  }
  print(rox_str("\n"));
  return 0;
}
