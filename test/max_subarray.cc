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

num32 max_sub_array(std::vector<num32> nums) {
  num n = nums.size();
  rox_result<num32> r0 = rox_at(nums, 0LL);
  if ((isOk(r0) == false))   {
    return 0;
  }
  num32 max_so_far = getValue(r0);
  num32 current_max = max_so_far;
  for (auto i = 1LL; i < n; i += 1LL)   {
    rox_result<num32> r = rox_at(nums, i);
    if ((isOk(r) == false))     {
      return 0;
    }
    num32 x = getValue(r);
    num32 sum = (current_max + x);
    if ((x > sum))     {
      (current_max = x);
    }
    else     {
      (current_max = sum);
    }
    if ((current_max > max_so_far))     {
      (max_so_far = current_max);
    }
  }
  return max_so_far;
}
int main() {
  std::vector<num32> nums = std::vector{(-2), 1, (-3), 4, (-1), 2, 1, (-5), 4};
  num32 res = max_sub_array(nums);
  if ((res == 6))   {
    print(std::vector{'M', 'a', 'x', 'i', 'm', 'u', 'm', ' ', 'S', 'u', 'b', 'a', 'r', 'r', 'a', 'y', ':', ' ', 'P', 'a', 's', 's', 'e', 'd', '\n'});
  }
  else   {
    print(std::vector{'M', 'a', 'x', 'i', 'm', 'u', 'm', ' ', 'S', 'u', 'b', 'a', 'r', 'r', 'a', 'y', ':', ' ', 'F', 'a', 'i', 'l', 'e', 'd', '\n'});
  }
  return 0;
}
