# ROX Language Compiler

ROX is a minimal, clarity-first programming language built on a simple belief:

> Programming logic should not have to fight the language.

ROX removes implicit behavior, hidden conversions, and syntactic tricks so that expressing logic feels direct and mechanical rather than negotiated.

ROX compiles `.rox` source files into C++20 (`.cc`), which are then compiled into native executables using `clang++`.

This repository contains the ROX compiler implementation written in C++20.

---

## First Principles

- We no longer write code that must fit on a punch card — readability wins over brevity.
- Code is read far more than it is written. Optimize for the reader, not the writer.
- Code is written in calm and read in crisis.
- Cleverness is often the enemy of clarity.

## Language Guarantees (v0)

- No implicit type conversions
- No bracket indexing (`[]` only for list literals)
- No exceptions — errors are explicit values (`rox_result[T]`)
- A single loop construct (`for`)
- Explicit control flow only
- Strict compile-time type checking

The surface area is intentionally small and opinionated.

---

## Example

Two Sum implemented in ROX:

```rox
function two_sum(list[int64] int64s, int64 target) -> list[int64] {
    int64 n = int64s.size();

    for i in range(0, n, 1) {
        for j in range(i + 1, n, 1) {

            rox_result[int64] r1 = int64s.at(i);
            if (isOk(r1)) {
                int64 v1 = getValue(r1);

                rox_result[int64] r2 = int64s.at(j);
                if (isOk(r2)) {g
                    int64 v2 = getValue(r2);

                    if (v1 + v2 == target) {
                        return [i, j];
                    }
                }
            }
        }
    }

    return [-1, -1];
}
```

### Notable properties

- `.at()` returns `rox_result[T]`
- Errors must be handled explicitly
- No implicit casts
- `range` is the only loop construct
- Lists are accessed only via `.at()`

ROX prioritizes clarity over convenience. Explicitness may cost more keystrokes, but it eliminates hidden behavior.

## Language Features (v0)

### Types

- `int64` (64 bit signed integer)
- `bool`
- `float64`
- `char`
- `string`
- `none`
- `list[T]`
- `dictionary[K, V]`
- `rox_result[T]`
- User-defined record types

### Control Flow

- `if` / `else`
- `for i in range(start, end, step)`
- `for item in collection`
- `break`
- `continue`

### Built-in Functions

- `print(val) -> none` (supports string, int64, float64, bool, char, list)
- `read_line() -> rox_result[string]` (reads one line from stdin)
- `isOk(rox_result[T]) -> bool`
- `getValue(rox_result[T]) -> T`
- `getError(rox_result[T]) -> string`
- `default(T) -> T` (zero/empty value for any type)

### Built-in Constants

- `pi`, `e` (float64)
- `EOF` (string) — error returned by `read_line()` on end of input

### Math Library

#### `int64`

- `int64_abs(n)`
- `int64_min(a, b)`
- `int64_max(a, b)`
- `int64_pow(base, exp) -> rox_result[int64]`

#### `float64`

- `float64_abs(n)`
- `float64_min(a, b)`
- `float64_max(a, b)`
- `float64_pow(base, exp)`
- `float64_sqrt(n) -> rox_result[float64]`
- `float64_sin(n)`
- `float64_cos(n)`
- `float64_tan(n)`
- `float64_log(n) -> rox_result[float64]`
- `float64_exp(n)`
- `float64_floor(n)`
- `float64_ceil(n)`

#### Constants

- `pi` (float64)
- `e` (float64)

## Error Model

ROX does not use exceptions. Errors are explicit values:

```rox
rox_result[int64] r = int64s.at(i);
if (isOk(r)) {
    int64 value = getValue(r);
} else {
    return [-1, -1];
}
```

To get the error message:

```rox
if (not isOk(r)) {
    print("Error: ", getError(r), "\n");
}
```

Nothing throws. Nothing hides.

## Strings

Strings are immutable sequences of UTF-8 bytes.

```rox
string s = "Hello, World!";
print(s);
print("\n");
```

## Dictionaries

Hash maps for key-value storage.

```rox
dictionary[string, int64] scores;
scores.set("Alice", 100);
if (scores.has("Alice")) {
    print(scores.get("Alice"));
}
```

## Comments

Comments are single-line and start with `//`.

```rox
// This is a comment
int64 x = 10; // This is also a comment
```

## User-Defined Types (Records)

ROX supports user-defined record types with named, typed fields.

```rox
type User {
    id: int64
    name: string
}

function main() -> none {
    User u = User{ id: 7, name: "Taman" };
    print(u.name, "\n");

    u.name = "Apon";
    print(u.name, "\n");

    // Zero constructor
    User empty = default(User);
    print(empty.id, "\n"); // 0
}
```

Records have copy semantics, support nested composition, and enforce all fields at construction time. Functions operate on records as parameters and return values — no methods.

## Compilation Model

ROX is compiled, not interpreted.

`.rox` → `.cc` → native binary

1. ROX source is parsed and type-checked.
2. C++20 code is generated.
3. `clang++` compiles the emitted C++ into an executable.

The generated C++ is intentionally straightforward and readable.

## Requirements

- C++20-compatible compiler (e.g., `clang++`)
- Make

## Build

```bash
make
```

## Usage

### Compile and Run

```bash
./rox run test/two_sum.rox
```

### Format Code

```bash
./rox format test/two_sum.rox
```

### Generate C++ Only

```bash
./rox generate test/two_sum.rox
```

### Compile Without Running

```bash
./rox compile test/two_sum.rox
```

## Test Programs

You can run all verified test programs with the provided script:

```bash
./test.sh
```

Alternatively, run them individually:

```bash
./rox run test/two_sum.rox
```

The `test/` directory contains verified implementations of:

- Two Sum
- Valid Parentheses
- Binary Search
- Maximum Subarray (Kadane’s Algorithm)
- Longest Substring Without Repeating Characters

These serve as correctness and regression tests for the compiler pipeline.

## Project Status

ROX v0 focuses on:

- Core type system
- Explicit error handling
- Deterministic control flow
- Clean C++ code generation
- Minimal language surface

Future directions (ROX++) may include:

- Module system
- Expanded standard library
- Static analysis improvements

## Philosophy Summary

ROX is not trying to compete with mainstream languages. It is an exploration of this question:

> What does programming look like when the language refuses to be clever?

## Web Playground

A local web-based playground is available to try ROX code interactively.

### Setup

```bash
cd web
npm install
```

### Run

```bash
node web/server.js
```

Then open `http://localhost:3000` in your browser.
