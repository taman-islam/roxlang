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

num binary_search_recursive(std::vector<num32> nums, num32 target, num low, num high) {
  if ((low > high))   {
    return (-1LL);
  }
  num diff = (high - low);
  rox_result<num> r_div = rox_div(diff, 2LL);
  if ((isOk(r_div) == false))   {
    return (-1LL);
  }
  num half = getValue(r_div);
  num mid = (low + half);
  rox_result<num32> r = rox_at(nums, mid);
  if ((isOk(r) == false))   {
    return (-1LL);
  }
  num32 midVal = getValue(r);
  if ((midVal == target))   {
    return mid;
  }
  else   if ((midVal < target))   {
    return binary_search_recursive(nums, target, (mid + 1LL), high);
  }
  else   {
    return binary_search_recursive(nums, target, low, (mid - 1LL));
  }
}
num search(std::vector<num32> nums, num32 target) {
  num n = nums.size();
  if ((n == 0LL))   {
    return (-1LL);
  }
  return binary_search_recursive(nums, target, 0LL, (n - 1LL));
}
int main() {
  std::vector<num32> nums = std::vector{(-1), 0, 3, 5, 9, 12};
  if ((search(nums, 9) == 4LL))   {
    if ((search(nums, 2) == (-1LL)))     {
      print(std::vector{'B', 'i', 'n', 'a', 'r', 'y', ' ', 'S', 'e', 'a', 'r', 'c', 'h', ':', ' ', 'P', 'a', 's', 's', 'e', 'd', '\n'});
      return 0;
    }
  }
  print(std::vector{'B', 'i', 'n', 'a', 'r', 'y', ' ', 'S', 'e', 'a', 'r', 'c', 'h', ':', ' ', 'F', 'a', 'i', 'l', 'e', 'd', '\n'});
  return 0;
}
