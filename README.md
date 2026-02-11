# ROX Language Compiler

ROX is a minimal, clarity-first programming language built on a simple belief:

> Programming logic should not have to fight the language.

ROX removes implicit behavior, hidden conversions, and syntactic tricks so that expressing logic feels direct and mechanical rather than negotiated.

ROX compiles `.rox` source files into C++20 (`.cc`), which are then compiled into native executables using `clang++`.

This repository contains the ROX compiler implementation written in C++20.

---

## Why ROX Exists

In many languages, expressing simple logic often requires navigating:

- Implicit type coercions
- Silent conversions
- Operator overloading
- Hidden control flow
- Exception systems
- Special cases

ROX intentionally removes these.

The goal is not convenience.
The goal is **clarity of expression**.

In ROX:

- Every type is explicit.
- Every error is a value.
- Every access is deliberate.
- Every control structure is visible.
- Nothing implicit happens behind your back.

You write the logic.
The language stays out of the way.

---

## Core Principles

ROX v0 enforces:

- No implicit type conversions
- No bracket indexing (`[]` only for list literals)
- No exceptions — errors are explicit values (`rox_result[T]`)
- A single loop construct (`repeat`)
- Explicit control flow only
- Strict compile-time type checking

The surface area is intentionally small and opinionated.

---

## Example

Two Sum implemented in ROX:

```rox
function two_sum(nums <list[num]>, target <num>) -> list[num] {
    let n <num> = nums.size();

    repeat i in range(0, n, 1) {
        repeat j in range(i + 1, n, 1) {

            let r1 <rox_result[num]> = nums.at(i);
            if (not isOk(r1)) { return [-1, -1]; }
            let v1 <num> = getValue(r1);

            let r2 <rox_result[num]> = nums.at(j);
            if (not isOk(r2)) { return [-1, -1]; }
            let v2 <num> = getValue(r2);

            if (v1 + v2 == target) {
                return [i, j];
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

The language forces clarity — not ceremony.

## Language Features (v0)

### Types

- `num` (int64)
- `num32` (int32)
- `bool`
- `char`
- `none`
- `list[T]`
- `rox_result[T]`

### Control Flow

- `if` / `else`
- `repeat i in range(start, end, step)`

### Built-in Functions

- `print`
- `isOk`
- `getValue`

## Error Model

ROX does not use exceptions. Errors are explicit values:

```rox
let r <rox_result[num]> = nums.at(i);
if (not isOk(r)) {
    return [-1, -1];
}
let value <num> = getValue(r);
```

Nothing throws. Nothing hides.

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

Future directions may include:

- Module system
- Expanded standard library
- Static analysis improvements
- Web-based playground

## Philosophy Summary

ROX is not trying to compete with mainstream languages. It is an exploration of this question:

> What does programming look like when the language refuses to be clever?
