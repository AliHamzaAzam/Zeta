# Zeta Programming Language

Zeta is a simple, readable programming language with English-like syntax. It is designed to be intuitive for beginners while offering enough expressiveness for educational programming tasks. The language features a clean, keyword-driven syntax that reads like natural language, making it easy to understand and write.

## Features

- **Variables with `global` and `local` scope**: Declare constants with `global` or `local`, and mutate local variables with `is now`
- **Arithmetic expressions with standard precedence**: Full support for `+`, `-`, `*`, `/`, `%`, and `^` with proper operator precedence
- **String literals in curly braces**: Write text naturally with `{Hello, World!}`
- **I/O with `tell` and `ask`**: Output values and read user input with readable keywords
- **Type checking and constant guarding**: Strong type system with INT, DECIMAL, STRING, and BOOL types
- **Comments**: Single-line comments with `<comment>` and multi-line with `<<comment>>`
- **Error reporting**: Clear syntax and semantic error messages with line and column information

## Prerequisites

- Java 22 or higher
- Maven 3.8+

## Installation

Clone the repository and build with Maven:

```bash
git clone git@github.com:AliHamzaAzam/Zeta.git
cd Zeta
mvn clean package
```

This will compile the project and create an executable JAR at `target/Zeta-1.0-SNAPSHOT.jar`.

## Quick Start

Create a file `hello.zeta`:

```zeta
tell {Hello, World!}
```

Run it:

```bash
./zetac hello.zeta
```

Or use the JAR directly:

```bash
java -jar target/Zeta-1.0-SNAPSHOT.jar hello.zeta
```

## Language Syntax

### Variables

```zeta
global pi is 3.14          <Constant, accessible everywhere>
local radius is 5          <Constant in current scope>
local count is 0
count is now count + 1     <Mutate a local variable>
```

### Types

- **INT**: Integer numbers (`5`, `-3`, `42`)
- **DECIMAL**: Floating-point numbers (`3.14`, `2.5`)
- **STRING**: Text in curly braces (`{Hello}`)
- **BOOL**: Boolean values (`true`, `false`)

### Operators

Operator precedence (highest to lowest):

1. `^` - Exponentiation (right-associative)
2. `*`, `/`, `%` - Multiplication, division, modulus
3. `+`, `-` - Addition, subtraction

Parentheses can be used to override precedence.

### I/O

```zeta
tell {The answer is } 42   <Output: The answer is 42>
ask name                   <Read a line from stdin into 'name'>
```

### Comments

```zeta
<This is a line comment>
global x is 10             <Comments can follow code>
```

## Examples

### 1. Hello World

**File:** `examples/01_hello.zeta`

```zeta
tell {Hello, World!}
```

**Expected output:**
```
Hello, World!
```

### 2. Arithmetic

**File:** `examples/02_arithmetic.zeta`

```zeta
global pi is 3.14
local radius is 5
local area is pi * radius ^ 2
tell {The area is } area
```

**Expected output:**
```
The area is 78.5
```

### 3. User Input

**File:** `examples/03_input.zeta`

```zeta
local name is {Anonymous}
local age is 0
tell {What is your name?}
ask name
tell {How old are you?}
ask age
tell {Hello, } name {. You are } age { years old.}
```

**Expected output (with user input "Alice" and "30"):**
```
What is your name?
Alice
How old are you?
30
Hello, Alice. You are 30 years old.
```

### 4. Scoping and Mutation

**File:** `examples/04_scoping.zeta`

```zeta
global max is 100
local count is 0
tell {Max is } max
tell {Count is } count
count is now count + 1
tell {Count is now } count
```

**Expected output:**
```
Max is 100
Count is 0
Count is now 1
```

### 5. Operator Precedence

**File:** `examples/05_precedence.zeta`

```zeta
local a is 2
local b is 3
local c is 4
local expr1 is a + b * c
local expr2 is (a + b) * c
local expr3 is a ^ b ^ 2
local expr4 is 10 / 3 + 10 % 3
tell {a + b * c = } expr1
tell {(a + b) * c = } expr2
tell {a ^ b ^ 2 = } expr3
tell {10 / 3 + 10 % 3 = } expr4
```

**Expected output:**
```
a + b * c = 14
(a + b) * c = 20
a ^ b ^ 2 = 512
10 / 3 + 10 % 3 = 6
```

## Project Structure

```
Zeta/
├── docs/                          # Documentation
│   ├── language-spec.md           # Formal language specification
│   ├── usage.md                   # Usage guide
│   └── contributor-guide.md       # Contribution guidelines
├── examples/                      # Example programs
│   ├── 01_hello.zeta
│   ├── 02_arithmetic.zeta
│   ├── 03_input.zeta
│   ├── 04_scoping.zeta
│   └── 05_precedence.zeta
├── src/
│   ├── main/java/org/azaleas/compiler/
│   │   ├── ast/                   # AST node classes
│   │   ├── automata/              # NFA/DFA framework (educational)
│   │   ├── cli/                   # CLI entry point
│   │   ├── errors/                # Error handling
│   │   ├── interpreter/           # Tree-walking interpreter
│   │   ├── lexer/                 # Lexer and preprocessor
│   │   ├── parser/                # Recursive descent parser
│   │   ├── semantic/              # Semantic analyzer
│   │   └── symboltable/           # Symbol table (legacy)
│   └── test/java/org/azaleas/compiler/
│       ├── integration/           # End-to-end tests
│       ├── interpreter/           # Interpreter tests
│       ├── lexer/                 # Lexer tests
│       ├── parser/                # Parser tests
│       └── semantic/              # Semantic analyzer tests
├── pom.xml                        # Maven build configuration
├── zetac                          # Convenience script to run programs
└── README.md                      # This file
```

## Testing

Run all tests with Maven:

```bash
mvn test
```

The test suite includes:
- **Lexer tests**: Tokenization accuracy
- **Parser tests**: Syntax parsing correctness
- **Semantic analyzer tests**: Type checking and scope validation
- **Interpreter tests**: Execution correctness
- **End-to-end tests**: Full compilation pipeline

## Documentation

- [Language Specification](docs/language-spec.md) - Formal grammar, types, and semantics
- [Usage Guide](docs/usage.md) - Detailed usage instructions and error messages
- [Contributor Guide](docs/contributor-guide.md) - Project structure and how to extend the language

## Contributing

Contributions are welcome! To add a new feature or fix a bug:

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Make your changes
4. Add tests for new functionality
5. Ensure all tests pass (`mvn test`)
6. Commit your changes (`git commit -m 'Add amazing feature'`)
7. Push to the branch (`git push origin feature/amazing-feature`)
8. Open a Pull Request

### Adding a New Statement Type

To extend the language with a new statement:

1. Add an AST node in `src/main/java/org/azaleas/compiler/ast/`
2. Add parsing logic in `src/main/java/org/azaleas/compiler/parser/Parser.java`
3. Add semantic checks in `src/main/java/org/azaleas/compiler/semantic/SemanticAnalyzer.java`
4. Add execution logic in `src/main/java/org/azaleas/compiler/interpreter/Interpreter.java`
5. Add tests in `src/test/java/org/azaleas/compiler/`

## License

This project is licensed under the [MIT License](LICENSE).

## Acknowledgments

Zeta was created as an educational programming language to demonstrate compiler construction concepts including lexical analysis, parsing, semantic analysis, and interpretation.
