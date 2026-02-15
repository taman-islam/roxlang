# ROX Language Reference

ROX is a minimal, clarity-first programming language built on a simple belief:

> Programming logic should not have to fight the language.

ROX removes implicit behavior, hidden conversions, and syntactic tricks so that expressing logic feels direct and mechanical rather than negotiated.

## Core Principles (v0)

- No implicit type conversions
- No bracket indexing (`[]` only for list literals)
- No exceptions — errors are explicit values (`rox_result[T]`)
- A single loop construct (`repeat`)
- Explicit control flow only
- Strict compile-time type checking
- **Reserved Prefix**: `roxv26_` is reserved for internal namespacing. User variables must not start with this prefix.

## Namespacing (v0)

ROX automatically namespaces all user-defined identifiers to prevent collisions with C++ keywords and standard library symbols.

- User variable `x` becomes `roxv26_x` in generated C++.
- **Reserved Components**: The following are NOT namespaced:
  - `main` function (entry point)
  - Built-in functions (`print`, `isOk`, etc.)
  - Math library functions (`num32_abs`, `pi`, etc.)
  - Standard library constants (`true`, `false`, `none`)

**Rule**: User identifiers **must not** begin with `roxv26_`. This prefix is reserved for the compiler.

## Operators

### Arithmetic

- `+`, `-`, `*`: Standard arithmetic.
- `/`: Division. Returns `rox_result[T]`.
- `%`: Modulo. Returns `rox_result[T]`.

### Comparison

- `==`: Equal
- `<`, `<=`, `>`, `>=`
- Note: `!=` is **not** supported. Use `not (a == b)`.

### Logical

- `and`
- `or`
- `not`: Replaces `!`.

## Types

ROX is a statically typed language. All types must be explicit.

### Primitive Types

- `num`: 64-bit Signed Integer. The default integer type.
  ```rox
  num x = 42;
  ```
- `num32`: 32-bit Signed Integer.

  ```rox
  num32 i = 10n;
  ```

- `float`: 64-bit Floating Point.
  ```rox
  float f = 3.14;
  ```
- `bool`: Boolean (`true` or `false`).
- `char`: Single ASCII character.
  ```rox
  char c = 'A';
  ```
- `string`: Immutable sequence of UTF-8 bytes.
  ```rox
  string s = "Hello";
  ```
- `none`: Unit type (similar to `void`).

### Composite Types

- `list[T]`: Dynamic Array. Ordered collection of elements of type `T`.
  ```rox
  list[num] numbers = [1, 2, 3];
  ```
- `dictionary[K, V]`: Hash Map. Key-value pairs.
  ```rox
  dictionary[string, num] scores;
  ```

### Result Type

- `rox_result[T]`: Explicit error handling type.

  ```rox
  rox_result[num] result = list.at(5);
  if (isOk(result)) {
      // Safe to call getValue(result) here
      num val = getValue(result);
  } else {
      print("Error: ", getError(result), "\n");
  }
  ```

  **Safety Note**: `getValue(result)` can _only_ be called within an `if (isOk(result))` block. The compiler currently enforces this via strict flow-sensitive analysis. Reassigning `result` invalidates this check. Future versions may include smarter checking to support more complex flow patterns.

## Control Flow

### Conditionals

Standard `if` / `else if` / `else`.

```rox
if (x > 10) {
    print("Big\n");
} else {
    print("Small\n");
}
```

### Loops

ROX has a single loop construct: `repeat`.

```rox
// Range(start, end, step)
repeat i in range(0, 5, 1) { ... }

// Range(start, end) - step defaults to 1
repeat i in range(0, 5) { ... }

// Backwards
repeat i in range(10, 0, -1) { ... }
```

**Note**: `range(start, end, step)` is a special built-in iterable construct that can only be used with `repeat`. It does not return a list.

### Loop Control

- `break`: Terminates the loop.
- `continue`: Skips to next iteration.

````

## Collections

### Lists

Access is strictly checked and returns `rox_result[T]`.

**Methods:**

- `.size() -> num`
- `.append(item) -> none`
- `.pop() -> none`
- `.at(index) -> rox_result[T]`

### Dictionaries

Key-value maps. Access returns `rox_result[V]`.

**Methods:**

- `.set(key, value) -> none`
- `.remove(key) -> none`
- `.has(key) -> bool`
- `.size() -> num`
- `.get(key) -> rox_result[V]`
- `.getKeys() -> list[K]`

### Strings

Immutable sequence of bytes.

**Methods:**

- `.size() -> num`
- `.at(index) -> rox_result[char]`

## Functions

Functions are first-class citizens in ROX. They must specify parameter types and return types explicitly.

```rox
function add(num a, num b) -> num {
    return a + b;
}
```

### Syntax

```rox
function name(type name, ...) -> return_type {
    // body
    return value;
}
```

- **Parameters**: Must have explicit types.
- **Return Type**: Must be explicit. Use `none` if no value is returned.
- **Recursion**: Supported.
- **Type Matching**: Function types must match exactly. No implicit conversions or variance are allowed.

### Return Values

Functions returning `none` can omit the return statement or use `return;` or `return none;`.

```rox
function log(string msg) -> none {
    print(msg);
    // Implicit return none
}
```

**Rule**: If a function has return type `none`, reaching the end of the function body implicitly returns `none`.

### Functions as Values

Functions can be assigned to variables, passed as arguments, and returned from other functions.

**Syntax for Function Types:** `function(paramType1, paramType2,...) -> returnType`

```rox
function add(num a, num b) -> num {
    return a + b;
}

// Assignment
function(num, num) -> num op = add;

// Passing as argument
function apply(function(num, num) -> num f, num a, num b) -> num {
    return f(a, b);
}

// Returning functions
function log(string msg) -> none {
    print(msg);
}

function get_logger() -> function(string) -> none {
    return log;
}

function(string)->none logger = get_logger();
logger("Log this message\n");
```

## Built-in Functions

- `print(val...) -> none`: Variadic. Accepts one or more arguments.

- `isOk(rox_result[T]) -> bool`
- `getValue(rox_result[T]) -> T`
- `getError(rox_result[T]) -> string`

## Math Library

### `num` (64-bit)

- `num_abs(n)`
- `num_min(a, b)`
- `num_max(a, b)`
- `num_pow(base, exp) -> rox_result[num]`

### `num32` (32-bit)

- `num32_abs(n)`
- `num32_min(a, b)`
- `num32_max(a, b)`
- `num32_pow(base, exp) -> rox_result[num32]`

### `float` (double)

- `float_abs(n)`
- `float_min(a, b)`
- `float_max(a, b)`
- `float_pow(base, exp)`
- `float_sqrt(n) -> rox_result[float]`
- `float_sin, float_cos, float_tan`
- `float_log, float_exp`
- `float_floor, float_ceil`

### Constants

- `pi`
- `e`

## Comments

Single-line comments starting with `//`.

```rox
// This is a comment
num x = 10;
````
