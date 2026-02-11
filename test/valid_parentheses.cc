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
using num = int64_t;
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
bool isOk(rox_result<T> r) {
    return r.err == 0;
}
template<typename T>
T getValue(rox_result<T> r) {
    if (r.err != 0) {
        std::cerr << "Called getValue on runtime error result!" << std::endl;
        exit(r.err);
    }
    return r.value;
}
void print_loop(num32 n) {
    for (int i = 0; i < n; ++i) {
        std::cout << "Hello, World!" << std::endl;
    }
}
// Result constructors
template<typename T>
rox_result<T> ok(T value) { return {value, 0}; }
template<typename T>
rox_result<T> error(num32 code) { return {T{}, code}; }
const double pi = 3.141592653589793;
const double e  = 2.718281828459045;
None print(const std::vector<char>& s) {
    for (char c : s) std::cout << c;
    return none;
}
std::vector<char> rox_str(const char* s) {
    std::vector<char> v;
    while (*s) v.push_back(*s++);
    return v;
}

template<typename T>
rox_result<T> rox_at(const std::vector<T>& xs, num i) {
    if (i < 0 || i >= (num)xs.size()) return error<T>(1); // index_out_of_range
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

num32 num32_abs(num32 x) { return std::abs(x); }

// End Runtime

bool is_valid(std::vector<char> s) {
  std::vector<char> stack = std::vector{'a'};
  stack.pop_back();
  num n = s.size();
  for (auto i = ((num)0); i < n; i += ((num)1))   {
    rox_result<char> r = rox_at(s, i);
    if ((isOk(r) == false))     {
      return false;
    }
    char c = getValue(r);
    if ((c == '('))     {
      stack.push_back(c);
    }
    else     if ((c == '{'))     {
      stack.push_back(c);
    }
    else     if ((c == '['))     {
      stack.push_back(c);
    }
    else     {
      if ((stack.size() == ((num)0)))       {
        return false;
      }
      num last_idx = (stack.size() - ((num)1));
      rox_result<char> rt = rox_at(stack, last_idx);
      if ((isOk(rt) == false))       {
        return false;
      }
      char top = getValue(rt);
      stack.pop_back();
      if ((c == ')'))       {
        if ((top != '('))         {
          return false;
        }
      }
      else       if ((c == '}'))       {
        if ((top != '{'))         {
          return false;
        }
      }
      else       if ((c == ']'))       {
        if ((top != '['))         {
          return false;
        }
      }
    }
  }
  return (stack.size() == ((num)0));
}
int main() {
  if ((is_valid(std::vector{'(', ')'}) == true))   {
    if ((is_valid(std::vector{'(', ')', '[', ']', '{', '}'}) == true))     {
      if ((is_valid(std::vector{'(', ']'}) == false))       {
        if ((is_valid(std::vector{'(', '[', ')', ']'}) == false))         {
          if ((is_valid(std::vector{'{', '[', ']', '}'}) == true))           {
            print(std::vector{'V', 'a', 'l', 'i', 'd', ' ', 'P', 'a', 'r', 'e', 'n', 't', 'h', 'e', 's', 'e', 's', ':', ' ', 'P', 'a', 's', 's', 'e', 'd', '\n'});
            return 0;
          }
        }
      }
    }
  }
  print(std::vector{'V', 'a', 'l', 'i', 'd', ' ', 'P', 'a', 'r', 'e', 'n', 't', 'h', 'e', 's', 'e', 's', ':', ' ', 'F', 'a', 'i', 'l', 'e', 'd', '\n'});
  return 0;
}
