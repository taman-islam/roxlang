# ROX Language Specification

## Version 0 — FROZEN

> Status: Locked for implementation.

---

## 1 Core Principles

1. One way to express an idea
2. Everything explicit
3. Plain English naming
4. Strong typing (no implicit conversions)
5. Compiled, emits C++
6. Errors are values (no exceptions)

---

## 2 Type System

### 2.1 Primitive Types (passed by value)

| Type    | C++ Mapping      | Notes            |
| ------- | ---------------- | ---------------- |
| `num32` | `int32_t`        | 32-bit signed    |
| `num64` | `int64_t`        | 64-bit signed    |
| `float` | `double`         | IEEE 754         |
| `bool`  | `bool`           | `true` / `false` |
| `char`  | `char`           | ASCII            |
| `none`  | `struct None {}` | Unit type        |

`none` has exactly one value: `none`.

### 2.2 Container Types (passed by read-only reference)

| Type               | C++ Mapping                |
| ------------------ | -------------------------- |
| `list[T]`          | `std::vector<T>`           |
| `dictionary[K, V]` | `std::unordered_map<K, V>` |

There is no `string` type. Text is `list[char]`.

### 2.3 Global Parameter Rule (LOCKED)

Primitive → pass by value
Container → pass by read-only reference
No other modes exist.

---

## 3 Literals

| Literal          | Type    |
| ---------------- | ------- |
| `10`             | `num64` |
| `10n32`          | `num32` |
| `20.0`           | `float` |
| `true` / `false` | `bool`  |
| `'a'`            | `char`  |
| `none`           | `none`  |

No implicit conversions.

---

## 4 Bindings

```
let   name <type> = expression;
const name <type> = expression;
```

Rules:

- Statements end with `;`
- No type inference
- `const` allowed anywhere
- `const` holds primitives only
- `const` immutable everywhere
- `let` mutable only in creation scope

---

## 5 Functions

```
function name(param <type>) -> return_type {
  statements;
}
```

Rules:

- No overloading
- No default args
- Explicit return type
- `return;` allowed only in `-> none`
- `return none;` equivalent in `-> none`

### 5.1 Entry Point

```
function main() -> none {
  ...
}
```

Exactly one `main`. No parameters.

---

## 6 Mutability (LOCKED)

A value is mutable only in the scope where it was created.
Outside that scope it is read-only.

---

## 7 Operators

### 7.1 Arithmetic

Applies to numeric primitives only:

```
+  -  *  /  %
```

Integer `/` and `%` return `rox_result[T]`.
Division by zero → error code `3n32`.
Float `/` follows IEEE behavior.

### 7.2 Boolean Operators (LOCKED)

English keywords only. No `&&`, `||`, `!`.

```
and
or
not
```

Example:

```
if (a and not b) {
  ...
}
```

### 7.3 Equality

| Type              | Rule               |
| ----------------- | ------------------ |
| Primitives        | Value equality     |
| `list[T]`         | Element-by-element |
| `dictionary[K,V]` | Key/value equality |
| Cross-type        | Compile error      |

### 7.4 Comparison Operators (LOCKED)

Available operators: `<`, `<=`, `>`, `>=`, `==`, `!=`

Rules:

- Both operands must be the same type — no implicit conversions
- Result type is always `bool`

| Type              | `==` `!=` | `<` `<=` `>` `>=` | Notes                           |
| ----------------- | --------- | ----------------- | ------------------------------- |
| `num32`           | ✅        | ✅                |                                 |
| `num64`           | ✅        | ✅                |                                 |
| `float`           | ✅        | ✅                |                                 |
| `char`            | ✅        | ✅                | Maps to ASCII numeric ordering  |
| `bool`            | ✅        | ❌                | No ordering on booleans         |
| `list[T]`         | ✅        | ❌                | No lexicographic ordering in v0 |
| `dictionary[K,V]` | ❌        | ❌                | No comparison operations        |
| `none`            | ✅        | ❌                | `none == none` is always `true` |

Valid examples:

```
1n32 < 2n32;
3 < 5;
2.0 <= 3.5;
'a' < 'z';
```

Invalid examples:

```
1n32 < 2;       // num32 vs num64 — compile error
3 < 5.0;        // num64 vs float — compile error
true < false;   // no ordering on bool — compile error
```

---

## 8 Containers

### 8.1 List Literals (LOCKED)

```
[expr1, expr2, ...]
```

Rules:

- All elements must have identical type
- Type inferred as `list[T]`
- No implicit conversions

Empty list:

```
let xs <list[num32]> = [];
```

Allowed only if element type is known from context.

### 8.2 Access (LOCKED)

Bracket indexing is forbidden.

```
xs[0];      // illegal
dict[key];  // illegal
```

All reads use `.at()`. Every `.at()` returns `rox_result[T]`.

```
xs.at(i);
dict.at(key);
```

### 8.3 Mutation (creation scope only)

**`list[T]`:** `append`, `insert`, `remove_at`, `resize`, `clear`, `set`

**`dictionary[K, V]`:** `set`, `remove`

---

## 9 Control Flow

### 9.1 `if`

```
if (condition) {
  ...
} else if (condition) {
  ...
} else {
  ...
}
```

Condition must be `bool`.

### 9.2 `repeat` / `range`

```
repeat i in range(start, end, inc) {
  ...
}
```

Rules:

- `start` inclusive
- `end` exclusive
- `inc` non-zero; defaults to `1`
- All args same numeric type
- `i` is read-only
- `range` is lazy

---

## 10 Error Model

### 10.1 `error`

`error` is `num32`.

- `0n32` → no error
- Non-zero → actual error

### 10.2 Global Registry

```
errors <dictionary[num32, list[char]]>;
```

Read-only global. Pre-populated at compile time.
Only compiler-level developers may add new codes.

### 10.3 `rox_result[T]`

```cpp
struct rox_result<T> {
  T       value;
  int32_t err;
};
```

Invariant:

```
err == 0  →  success
err != 0  →  error
```

### 10.4 Result Functions

```
isOk(r <rox_result[T]>) -> bool;
getErrorCode(r <rox_result[T]>) -> num32;
getValue(r <rox_result[T]>) -> T;
```

`getValue` terminates program if `err != 0`.

### 10.5 Constructors (LOCKED)

```
ok(value) -> rox_result[T];
error(code <num32>) -> rox_result[T];
```

No other constructors exist. `fail[T]` does not exist.

### 10.6 Must-Handle Rule

- `rox_result[T]` cannot be ignored
- Cannot be implicitly unwrapped

### 10.7 Canonical Pattern

```
let r <rox_result[num32]> = xs.at(i);

if (not isOk(r)) {
  let code <num32> = getErrorCode(r);
  return error(code);
}

let x <num32> = getValue(r);
```

### 10.8 Error Codes (v0)

| Code        | Meaning            |
| ----------- | ------------------ |
| `0n32`      | ok                 |
| `1n32`      | index_out_of_range |
| `2n32`      | key_not_found      |
| `3n32`      | division_by_zero   |
| `4n32`      | invalid_range      |
| `5n32`      | io_error           |
| `6n32–8n32` | _(reserved)_       |
| `9n32`      | end_of_file        |
| `10n32`     | invalid_argument   |

---

## 11 I/O

No file operations in language. Shell handles redirection.

### 11.1 `print`

```
print(text <list[char]>) -> rox_result[none];
```

Newlines must be explicit (`'\n'`).

### 11.2 `read_line`

```
read_line() -> rox_result[list[char]>;
```

EOF → `9n32`.

---

## 12 Math Library

No overloading. Names prefixed by type.

### 12.1 `num32`

| Function    | Returns             |
| ----------- | ------------------- |
| `num32_abs` | `num32`             |
| `num32_min` | `num32`             |
| `num32_max` | `num32`             |
| `num32_pow` | `rox_result[num32]` |

### 12.2 `num64`

| Function    | Returns             |
| ----------- | ------------------- |
| `num64_abs` | `num64`             |
| `num64_min` | `num64`             |
| `num64_max` | `num64`             |
| `num64_pow` | `rox_result[num64]` |

### 12.3 `float`

| Function      | Returns             |
| ------------- | ------------------- |
| `float_abs`   | `float`             |
| `float_min`   | `float`             |
| `float_max`   | `float`             |
| `float_pow`   | `float`             |
| `float_sqrt`  | `rox_result[float]` |
| `float_sin`   | `float`             |
| `float_cos`   | `float`             |
| `float_tan`   | `float`             |
| `float_log`   | `rox_result[float]` |
| `float_exp`   | `float`             |
| `float_floor` | `float`             |
| `float_ceil`  | `float`             |

### 12.4 Built-in Constants

```
const pi <float> = 3.141592653589793;
const e  <float> = 2.718281828459045;
```

---

## 13 Built-ins

Pre-populated in global scope. User cannot redefine built-ins.

| Category            | Names                                                                                                                                                          |
| ------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Primitive types     | `num32`, `num64`, `float`, `bool`, `char`, `none`                                                                                                              |
| Containers          | `list`, `dictionary`                                                                                                                                           |
| Result type         | `rox_result[T]`                                                                                                                                                |
| Result constructors | `ok`, `error`                                                                                                                                                  |
| Result functions    | `isOk`, `getErrorCode`, `getValue`                                                                                                                             |
| Loop                | `range`                                                                                                                                                        |
| Boolean keywords    | `and`, `or`, `not`                                                                                                                                             |
| Comparison ops      | `<`, `<=`, `>`, `>=`, `==`, `!=`                                                                                                                               |
| I/O                 | `print`, `read_line`                                                                                                                                           |
| Global constant     | `errors`                                                                                                                                                       |
| Math constants      | `pi`, `e`                                                                                                                                                      |
| `num32` math        | `num32_abs`, `num32_min`, `num32_max`, `num32_pow`                                                                                                             |
| `num64` math        | `num64_abs`, `num64_min`, `num64_max`, `num64_pow`                                                                                                             |
| `float` math        | `float_abs`, `float_min`, `float_max`, `float_pow`, `float_sqrt`, `float_sin`, `float_cos`, `float_tan`, `float_log`, `float_exp`, `float_floor`, `float_ceil` |

---

## 14 CLI

| Command                 | Behaviour                           |
| ----------------------- | ----------------------------------- |
| `rox generate file.rox` | Emits `.cc` only                    |
| `rox compile file.rox`  | Emits `.cc` + links → native binary |
| `rox run file.rox`      | Compiles and executes               |

---

## 15 Deferred to v1

- Modules
- User-defined types
- Exceptions
- File APIs
- Pattern matching
- Generics beyond `rox_result[T]`
