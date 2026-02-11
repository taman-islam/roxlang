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

bool contains(std::vector<char> items, char c) {
  num n = items.size();
  for (auto i = ((num)0); i < n; i += ((num)1))   {
    rox_result<char> r = rox_at(items, i);
    if (isOk(r))     {
      char val = getValue(r);
      if ((val == c))       {
        return true;
      }
    }
  }
  return false;
}
num length_of_longest_substring(std::vector<char> items) {
  num n = items.size();
  num max_len = ((num)0);
  num left = ((num)0);
  for (auto k = ((num)0); k < n; k += ((num)1))   {
    rox_result<char> r_k = rox_at(items, k);
    if ((isOk(r_k) == false))     {
      return ((num)0);
    }
    char c = getValue(r_k);
    bool found = false;
    num found_index = ((num)0);
    for (auto j = left; j < k; j += ((num)1))     {
      rox_result<char> r_j = rox_at(items, j);
      if (isOk(r_j))       {
        char char_j = getValue(r_j);
        if ((char_j == c))         {
          (found = true);
          (found_index = j);
        }
      }
    }
    if ((found == true))     {
      if ((found_index >= left))       {
        (left = (found_index + ((num)1)));
      }
    }
    num current_len = ((k - left) + ((num)1));
    if ((current_len > max_len))     {
      (max_len = current_len);
    }
  }
  return max_len;
}
int main() {
  std::vector<char> s1 = std::vector{'a', 'b', 'c', 'a', 'b', 'c', 'b', 'b'};
  if ((length_of_longest_substring(s1) == ((num)3)))   {
    std::vector<char> s2 = std::vector{'b', 'b', 'b', 'b', 'b'};
    if ((length_of_longest_substring(s2) == ((num)1)))     {
      std::vector<char> s3 = std::vector{'p', 'w', 'w', 'k', 'e', 'w'};
      if ((length_of_longest_substring(s3) == ((num)3)))       {
        print(std::vector{'L', 'o', 'n', 'g', 'e', 's', 't', ' ', 'S', 'u', 'b', 's', 't', 'r', 'i', 'n', 'g', ':', ' ', 'P', 'a', 's', 's', 'e', 'd', '\n'});
        return 0;
      }
    }
  }
  print(std::vector{'L', 'o', 'n', 'g', 'e', 's', 't', ' ', 'S', 'u', 'b', 's', 't', 'r', 'i', 'n', 'g', ':', ' ', 'F', 'a', 'i', 'l', 'e', 'd', '\n'});
  return 0;
}
