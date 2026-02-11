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

std::vector<num> two_sum(std::vector<num32> nums, num32 target) {
  num n = nums.size();
  for (auto i = 0LL; i < n; i += 1LL)   {
    for (auto j = (i + 1LL); j < n; j += 1LL)     {
      rox_result<num32> r1 = rox_at(nums, i);
      if ((isOk(r1) == false))       {
        return std::vector{(-1LL), (-1LL)};
      }
      num32 v1 = getValue(r1);
      rox_result<num32> r2 = rox_at(nums, j);
      if ((isOk(r2) == false))       {
        return std::vector{(-1LL), (-1LL)};
      }
      num32 v2 = getValue(r2);
      if (((v1 + v2) == target))       {
        return std::vector{i, j};
      }
    }
  }
  return std::vector{(-1LL), (-1LL)};
}
int main() {
  std::vector<num32> nums = std::vector{2, 7, 11, 15};
  num32 target = 9;
  std::vector<num> result = two_sum(nums, target);
  if ((result.size() == 2LL))   {
    rox_result<num> r0 = rox_at(result, 0LL);
    rox_result<num> r1 = rox_at(result, 1LL);
    if ((isOk(r0) && isOk(r1)))     {
      if (((getValue(r0) == 0LL) && (getValue(r1) == 1LL)))       {
        print(std::vector{'T', 'w', 'o', ' ', 'S', 'u', 'm', ':', ' ', 'P', 'a', 's', 's', 'e', 'd', '\n'});
        return 0;
      }
    }
  }
  print(std::vector{'F', 'a', 'i', 'l', 'e', 'd', '\n'});
  return 0;
}
