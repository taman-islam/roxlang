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
      num val = getValue(result);
  }
  ```

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
- `.toList() -> list[char]`

## Built-in Functions

- `print(val) -> none`: Supports `string`, `num`, `num32`, `float`, `bool`, `char`, `list`.
- `listToString(list[T]) -> list[char]`
- `isOk(rox_result[T]) -> bool`
- `getValue(rox_result[T]) -> T`

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
```
