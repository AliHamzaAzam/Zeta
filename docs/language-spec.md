# Zeta Language Specification

## Overview

Zeta is a simple, English-like programming language designed for readability and educational purposes. The syntax reads like natural language, making it accessible to beginners while offering enough expressiveness for basic programming tasks. Zeta is intentionally minimal, it omits common features like loops, conditionals, and functions to keep the language easy to understand and implement.

Design goals:

- **Readability**: Keywords like `tell`, `ask`, `is`, and `now` make code read like sentences
- **Simplicity**: A small feature set means a small mental model
- **Type safety**: Strong static typing with inference catches errors early
- **Educational value**: The language is simple enough to serve as a teaching tool for compiler construction

---

## Lexical Specification

Zeta source code is a sequence of Unicode characters that the lexer groups into tokens. Whitespace and comments are discarded during tokenization.

### Token Types

| Token | Regex Pattern | Description |
|-------|---------------|-------------|
| **Keywords** | `global`, `local`, `is`, `now`, `tell`, `ask`, `true`, `false` | Reserved words with special meaning |
| **Identifier** | `[a-zA-Z_][a-zA-Z0-9_]*` | Variable names of letters, digits and underscores, starting with a letter or underscore |
| **Integer** | `[+-]?\d+` | Whole numbers, optionally signed |
| **Decimal** | `[+-]?(\d+\.\d{1,5}|\.\d{1,5})([eE][+-]?\d+)?` | Floating-point numbers with up to 5 decimal places |
| **String** | `\{[^{}]*\}` | Text enclosed in curly braces |
| **Operator** | `[+\-*/%]` | Arithmetic operators (+, -, *, /, %) |
| **Exponent** | `\^` | Exponentiation operator |
| **Left Paren** | `\(` | Opening parenthesis |
| **Right Paren** | `\)` | Closing parenthesis |
| **Comment (single-line)** | `<[^>]*>` | Single-line comment enclosed in angle brackets |
| **Comment (multi-line)** | `<<.*?>>` | Multi-line comment enclosed in double angle brackets |
| **Whitespace** | `[ \t\n\r\f]+` | Spaces, tabs, newlines (discarded) |

### Notes on Lexing

- Keywords are matched with a negative lookahead `(?![a-zA-Z0-9_])` to prevent partial matches (e.g., `globalx` and `is_now` are identifiers, not the keywords `global` / `is`)
- The lexer uses priority-based matching: DECIMAL (priority 3) beats INTEGER (priority 2), so `3.14` is tokenized as a single decimal rather than integer `3` followed by `.14`
- Signed numbers are handled at the lexer level: `-5` is a single INTEGER token, not an operator `-` followed by `5`
- Strings in curly braces do not support escape sequences; the content between `{` and `}` is taken literally
- Comments are completely discarded and do not appear in the token stream

---

## Syntax (EBNF)

```
program       ::= statement*
statement     ::= varDecl | assignment | tell | ask

varDecl       ::= ("global" | "local") identifier "is" expression
assignment    ::= identifier "is" "now" expression
tell          ::= "tell" expression+
ask           ::= "ask" identifier

expression    ::= additive
additive      ::= multiplicative (("+" | "-") multiplicative)*
multiplicative ::= power (("*" | "/" | "%") power)*
power         ::= unary ("^" power)?
unary         ::= ("+" | "-") unary | primary
primary       ::= NUMBER | STRING | "true" | "false" | identifier | "(" expression ")"

identifier    ::= [a-zA-Z_][a-zA-Z0-9_]*
NUMBER        ::= INTEGER | DECIMAL
INTEGER       ::= [+-]?\d+
DECIMAL       ::= [+-]?(\d+\.\d{1,5}|\.\d{1,5})([eE][+-]?\d+)?
STRING        ::= "{" [^{}]* "}"
```

### Syntactic Notes

- Programs are sequences of statements executed in order from top to bottom
- `tell` accepts one or more expressions, all of which are evaluated and concatenated into a single output line
- `ask` reads a line from standard input and stores it in the named variable
- Parentheses override the default precedence and may nest arbitrarily
- Unary `+` and `-` are desugared into binary operations: `+x` becomes `0 + x`, and `-x` becomes `0 - x`

---

## Type System

Zeta has four built-in types. All types are inferred from literals; there are no explicit type annotations.

| Type | Description | Literal Form | Storage |
|------|-------------|--------------|---------|
| **INT** | 32-bit signed integers | `42`, `-7` | Java `Integer` |
| **DECIMAL** | 64-bit floating-point numbers | `3.14`, `-0.5` | Java `Double` |
| **STRING** | Unicode text | `{Hello}` | Java `String` |
| **BOOL** | Boolean values | `true`, `false` | Java `Boolean` |

### Type Inference Rules

The type of an expression is determined as follows:

- **Integer literal** -> INT
- **Decimal literal** -> DECIMAL
- **String literal** -> STRING
- **Boolean literal** -> BOOL
- **Variable reference** -> the type recorded in the symbol table at declaration
- **Binary expression** -> determined by the operator and operand types (see below)

### Type Compatibility Matrix

The `+` operator is the most polymorphic:

| Left | Right | Result | Behavior |
|------|-------|--------|----------|
| INT | INT | INT | Integer addition |
| INT | DECIMAL | DECIMAL | Promote INT to DECIMAL, then add |
| DECIMAL | INT | DECIMAL | Promote INT to DECIMAL, then add |
| DECIMAL | DECIMAL | DECIMAL | Floating-point addition |
| STRING | any | STRING | Convert both to string and concatenate |
| any | STRING | STRING | Convert both to string and concatenate |

For `-`, `*`, `/`, `%`, `^`:

| Left | Right | Result | Behavior |
|------|-------|--------|----------|
| INT | INT | INT | Integer operation (except `/` and `^`) |
| INT | DECIMAL | DECIMAL | Promote INT to DECIMAL, then operate |
| DECIMAL | INT | DECIMAL | Promote INT to DECIMAL, then operate |
| DECIMAL | DECIMAL | DECIMAL | Floating-point operation |

Special cases:

- `/` (division) always returns DECIMAL, even with two INT operands
- `%` (modulus) requires both operands to be INT and returns INT
- `^` (exponentiation) always returns DECIMAL
- String concatenation with `+` requires at least one operand to be STRING; the other is converted via `toString()`

### Assignment Compatibility

A value of type `source` may be assigned to a variable of type `target` if:

- `target == source` (exact match), OR
- `target == DECIMAL && source == INT` (INT promotes to DECIMAL)

All other combinations are rejected by the semantic analyzer.

---

## Semantics

### Variable Declaration and Scoping

Variables are declared with `global` or `local`:

```
global pi is 3.14    -- constant, accessible everywhere
local count is 0     -- mutable, accessible in current scope
```

- `global` declares a constant. It cannot be reassigned.
- `local` declares a mutable variable. It may be reassigned with `is now`.
- Scope resolution is lexical. The current scope is checked first, then parent scopes are traversed upward.
- A variable may not be declared twice in the same scope.
- Accessing an undeclared variable is a semantic error.

### Assignment and Mutability

Assignment uses the `is now` syntax:

```
count is now count + 1
```

- Only `local` variables may be reassigned. Reassigning a `global` is a semantic error.
- The new value must be type-compatible with the variable's declared type.
- Assignment mutates the variable in place; there is no shadowing.

### Expression Evaluation

Expressions are evaluated using a tree-walking interpreter with the following rules:

- Literals evaluate to their corresponding typed values
- Variables evaluate to their current bound value in the environment
- Binary expressions evaluate left and right subtrees, then apply the operator
- Evaluation order is left-to-right for left-associative operators (`+`, `-`, `*`, `/`, `%`)
- `^` is right-associative: `a ^ b ^ c` parses as `a ^ (b ^ c)`

### Type Coercion

The only automatic coercion is INT to DECIMAL in mixed arithmetic operations. There is no coercion from DECIMAL to INT, STRING to number, or any other direction.

### I/O Behavior

**tell** evaluates each expression, converts the result to a string via `toString()`, concatenates all results in order, and prints a single line to standard output followed by a newline.

**ask** reads a single line from standard input. The input is parsed based on the variable's current type:

- If the variable is INT, the input is parsed with `Integer.parseInt()`
- If the variable is DECIMAL, the input is parsed with `Double.parseDouble()`
- Otherwise (STRING or BOOL), the input is stored as a raw string

If parsing fails, a Java `NumberFormatException` is thrown. There is no special error handling for invalid input.

---

## Operator Semantics

### Arithmetic Operators

| Operator | Types | Result | Description |
|----------|-------|--------|-------------|
| `+` | INT, INT | INT | Integer addition |
| `+` | DECIMAL, DECIMAL | DECIMAL | Floating-point addition |
| `+` | INT, DECIMAL | DECIMAL | Promote INT, then add |
| `+` | STRING, any | STRING | String concatenation |
| `-` | INT, INT | INT | Integer subtraction |
| `-` | DECIMAL, DECIMAL | DECIMAL | Floating-point subtraction |
| `-` | INT, DECIMAL | DECIMAL | Promote INT, then subtract |
| `*` | INT, INT | INT | Integer multiplication |
| `*` | DECIMAL, DECIMAL | DECIMAL | Floating-point multiplication |
| `*` | INT, DECIMAL | DECIMAL | Promote INT, then multiply |
| `/` | INT, INT | DECIMAL | Floating-point division |
| `/` | DECIMAL, DECIMAL | DECIMAL | Floating-point division |
| `%` | INT, INT | INT | Integer remainder |
| `^` | INT, INT | DECIMAL | `Math.pow(a, b)` |
| `^` | DECIMAL, DECIMAL | DECIMAL | `Math.pow(a, b)` |

### Division by Zero

Division by zero is not handled specially. Attempting `x / 0` or `x % 0` will throw a Java `ArithmeticException` at runtime. There is no Infinity or NaN representation in Zeta.

### Unary Operators

Unary `+` and `-` are implemented as binary operations against zero:

- `+x` is equivalent to `0 + x`
- `-x` is equivalent to `0 - x`

This means `-3.14` produces DECIMAL (because `0` is INT and `3.14` is DECIMAL, so the result is DECIMAL).

---

## Execution Model

### Tree-Walking Interpreter

Zeta programs are executed by a tree-walking interpreter that traverses the AST depth-first:

1. The lexer converts source text into a token stream
2. The parser builds an AST from the token stream
3. The semantic analyzer performs type checking and scope validation
4. The interpreter walks the AST, evaluating statements and expressions

### Environment and Stack Model

The interpreter maintains an environment chain:

- Each scope has a local variable map and an optional parent environment
- Variable lookup searches the current map, then walks up the parent chain
- Variable assignment searches the same chain and updates the first match found
- The global scope is the root of the chain with no parent

There is no call stack because Zeta has no functions or procedures. Execution is purely sequential.

### Global vs Local Scope Resolution

- `global` declarations go into the root (global) environment
- `local` declarations go into the current (local) environment
- Lookups from any scope can access variables from parent scopes
- The semantic analyzer enforces that `global` variables are constants

### Error Handling

Errors are categorized by phase:

- **Lexical errors**: Unexpected characters that do not match any token pattern
- **Parse errors**: Unexpected tokens that violate the grammar
- **Semantic errors**: Type mismatches, undeclared variables, reassignment of constants
- **Runtime errors**: Division by zero, invalid input parsing in `ask`

All errors are reported through an `ErrorHandler` that collects messages with line numbers. The interpreter throws Java exceptions for runtime errors; there is no Zeta-level exception handling.

---

## Limitations

Zeta is intentionally minimal. The following common language features are NOT supported:

- **No loops**: No `for`, `while`, `repeat`, or any iteration construct
- **No conditionals**: No `if`, `else`, `switch`, or branching
- **No functions or procedures**: No user-defined subroutines, parameters, or return values
- **No arrays or collections**: No lists, sets, maps, or indexed access
- **No modules or imports**: All code must be in a single file
- **No object-oriented features**: No classes, objects, inheritance, or methods
- **No advanced types**: No structs, enums, unions, or generics
- **No exception handling**: No `try`, `catch`, `throw`
- **No bitwise operators**: No `&`, `|`, `~`, `<<`, `>>`
- **No increment/decrement**: No `++`, `--`
- **No compound assignment**: No `+=`, `-=`, `*=`, etc.

These limitations make Zeta unsuitable for production software but ideal for educational exploration of compiler and interpreter construction.

---

## Examples

### Hello World

```
tell {Hello, World!}
```

**AST:**

```
Program
  Tell
    Literal("Hello, World!")
```

### Variables and Arithmetic

```
global pi is 3.14
local radius is 5
local area is pi * radius ^ 2
tell {The area is } area
```

**AST:**

```
Program
  VarDecl(scope="global", name="pi")
    Literal(3.14)
  VarDecl(scope="local", name="radius")
    Literal(5)
  VarDecl(scope="local", name="area")
    BinaryExpr("*")
      Variable("pi")
      BinaryExpr("^")
        Variable("radius")
        Literal(2)
  Tell
    Literal("The area is ")
    Variable("area")
```

### Input/Output

```
local name is {Anonymous}
local age is 0
tell {What is your name?}
ask name
tell {How old are you?}
ask age
tell {Hello, } name {. You are } age { years old.}
```

**AST (first three statements):**

```
Program
  VarDecl(scope="local", name="name")
    Literal("Anonymous")
  VarDecl(scope="local", name="age")
    Literal(0)
  Tell
    Literal("What is your name?")
  Ask(name="name")
  ...
```

### Scoping and Mutation

```
global max is 100
local count is 0
tell {Max is } max
tell {Count is } count
count is now count + 1
tell {Count is now } count
```

### Operator Precedence

```
local a is 2
local b is 3
local c is 4
local expr1 is a + b * c      -- 14 (multiplication before addition)
local expr2 is (a + b) * c    -- 20 (parentheses override)
local expr3 is a ^ b ^ 2      -- 512 (right-associative: 2^(3^2) = 2^9)
local expr4 is 10 / 3 + 10 % 3  -- 6.333... (division and modulus before addition)
```

---

## Summary

Zeta is a minimal, readable, statically typed language with English-like syntax. It supports variable declaration, arithmetic expressions, string concatenation, basic I/O, and lexical scoping. The language is intentionally limited to serve as a foundation for learning about lexers, parsers, semantic analyzers, and tree-walking interpreters.
