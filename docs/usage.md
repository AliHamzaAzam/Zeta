# Zeta Usage Guide

## Table of Contents

1. [Prerequisites](#prerequisites)
2. [Installation](#installation)
3. [CLI Reference](#cli-reference)
4. [Writing Zeta Programs](#writing-zeta-programs)
5. [Running Programs](#running-programs)
6. [Error Messages](#error-messages)
7. [Troubleshooting](#troubleshooting)
8. [Advanced Usage](#advanced-usage)

---

## Prerequisites

Before using Zeta, make sure you have the following installed:

- **Java 22 or higher** - Zeta is built on modern Java features and requires JDK 22+
- **Maven 3.8+** - Used to build the project from source

Verify your installations:

```bash
java -version
mvn -version
```

Both commands should print version information. If either is missing or too old, see the [Troubleshooting](#troubleshooting) section.

---

## Installation

### Step 1: Clone the Repository

```bash
git clone <repository-url>
cd Zeta
```

### Step 2: Build the Project

```bash
mvn clean package
```

This command compiles the project and creates an executable JAR file at:

```
target/Zeta-1.0-SNAPSHOT.jar
```

### Step 3: Make the Wrapper Script Executable (Optional)

The project includes a convenience wrapper script:

```bash
chmod +x zetac
```

After this step, you can run programs with `./zetac file.zeta`.

---

## CLI Reference

There are three ways to run a Zeta program:

### Option 1: Using the Wrapper Script (Recommended)

```bash
./zetac <file.zeta>
```

This is the most convenient method after building. The `zetac` script automatically invokes the JAR with the correct classpath.

### Option 2: Using the JAR Directly

```bash
java -jar target/Zeta-1.0-SNAPSHOT.jar <file.zeta>
```

Use this when you do not have the wrapper script or need to specify a different JAR path.

### Option 3: Using Maven

```bash
mvn exec:java -Dexec.args="<file.zeta>"
```

This is useful during development when you are making frequent changes and do not want to rebuild the JAR each time.

### Exit Codes

- **0** - Program executed successfully
- **1** - Compilation or runtime error (syntax error, semantic error, file not found, etc.)

---

## Writing Zeta Programs

### File Extension

Zeta source files use the `.zeta` extension.

### Program Structure

A Zeta program is a sequence of statements. Each statement appears on its own line. The language does not use semicolons or braces to delimit statements.

### Comments

Use `--` for line comments. Everything after `--` on a line is ignored.

```zeta
-- This is a comment
global pi is 3.14  -- Comments can also follow code
```

### Variables

Zeta has two kinds of variables:

#### Global Variables (Constants)

Declared with `global`. These are constants and cannot be changed after declaration.

```zeta
global pi is 3.14
global greeting is {Hello, World!}
```

#### Local Variables (Mutable)

Declared with `local`. These can be reassigned using `is now`.

```zeta
local count is 0
count is now count + 1  -- Reassign the local variable
```

#### Variable Declaration Syntax

```
<scope> <name> is <expression>
```

- `<scope>` - Either `global` or `local`
- `<name>` - Any identifier (letters, digits, underscores; must not start with a digit)
- `is` - Required keyword
- `<expression>` - The initial value

### Types

Zeta has four built-in types:

| Type | Description | Examples |
|------|-------------|----------|
| **INT** | Whole numbers | `5`, `-3`, `42`, `0` |
| **DECIMAL** | Floating-point numbers | `3.14`, `2.5`, `-0.5` |
| **STRING** | Text in curly braces | `{Hello}`, `{Zeta}` |
| **BOOL** | Boolean values | `true`, `false` |

Type inference is automatic. You do not declare types explicitly; the compiler infers them from the initial value.

### Operators

Zeta supports arithmetic operators with standard precedence:

| Precedence | Operator | Description | Associativity |
|------------|----------|-------------|---------------|
| 1 (highest) | `^` | Exponentiation | Right |
| 2 | `*`, `/`, `%` | Multiplication, division, modulus | Left |
| 3 (lowest) | `+`, `-` | Addition, subtraction | Left |

Use parentheses to override precedence:

```zeta
local a is 2
local b is 3
local c is 4
local result1 is a + b * c    -- 14 (multiplication first)
local result2 is (a + b) * c  -- 20 (parentheses first)
```

### Input and Output

#### Output with `tell`

The `tell` statement prints values to standard output. You can print multiple values separated by spaces.

```zeta
tell {Hello, World!}           -- Output: Hello, World!
tell {The answer is } 42       -- Output: The answer is 42
tell {Value: } x { and } y     -- Output multiple values
```

#### Input with `ask`

The `ask` statement reads a line from standard input and stores it in a variable.

```zeta
local name is {Anonymous}
tell {What is your name?}
ask name
tell {Hello, } name
```

The variable must be declared before using `ask`. The input type is inferred from the variable's initial type.

### Complete Syntax Summary

```
program        ::= statement*
statement      ::= var-decl | assignment | tell | ask
var-decl       ::= ("global" | "local") identifier "is" expression
assignment     ::= identifier "is" "now" expression
tell           ::= "tell" expression+
ask            ::= "ask" identifier
expression     ::= term (("+" | "-") term)*
term           ::= factor (("*" | "/" | "%") factor)*
factor         ::= power ("^" factor)?
power          ::= primary
primary        ::= number | string | boolean | identifier | "(" expression ")"
number         ::= integer | decimal
string         ::= "{" text "}"
boolean        ::= "true" | "false"
comment        ::= "--" text "\n"
```

---

## Running Programs

### Example 1: Hello World

Create `hello.zeta`:

```zeta
tell {Hello, World!}
```

Run it:

```bash
./zetac hello.zeta
```

Output:

```
Hello, World!
```

### Example 2: Arithmetic

Create `math.zeta`:

```zeta
global pi is 3.14
local radius is 5
local area is pi * radius ^ 2
tell {The area is } area
```

Run it:

```bash
./zetac math.zeta
```

Output:

```
The area is 78.5
```

This example shows:
- Global constant declaration
- Local variable declaration
- Exponentiation operator (`^`)
- Mixed operations with proper precedence

### Example 3: User Input

Create `input.zeta`:

```zeta
local name is {Anonymous}
local age is 0
tell {What is your name?}
ask name
tell {How old are you?}
ask age
tell {Hello, } name {. You are } age { years old.}
```

Run it:

```bash
./zetac input.zeta
```

Sample interaction:

```
What is your name?
Alice
How old are you?
30
Hello, Alice. You are 30 years old.
```

This example shows:
- Default values for variables
- Reading string and integer input
- Concatenating multiple values in a single `tell`

### Example 4: Variable Scoping

Create `scope.zeta`:

```zeta
global max is 100
local count is 0
tell {Max is } max
tell {Count is } count
count is now count + 1
tell {Count is now } count
```

Run it:

```bash
./zetac scope.zeta
```

Output:

```
Max is 100
Count is 0
Count is now 1
```

This example shows:
- Global constants are immutable
- Local variables can be reassigned with `is now`

### Example 5: Operator Precedence

Create `precedence.zeta`:

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

Run it:

```bash
./zetac precedence.zeta
```

Output:

```
a + b * c = 14
(a + b) * c = 20
a ^ b ^ 2 = 512
10 / 3 + 10 % 3 = 4.333...
```

This example shows:
- Multiplication has higher precedence than addition
- Parentheses override precedence
- Exponentiation is right-associative (`2 ^ 3 ^ 2` = `2 ^ 9` = `512`)
- Division and modulus have the same precedence and are evaluated left to right

---

## Error Messages

Zeta reports errors with line numbers to help you locate and fix issues quickly.

### Syntax Errors

#### "Expected 'is' after identifier"

**Cause:** You wrote a variable declaration or assignment but forgot the `is` keyword.

**Incorrect:**

```zeta
local x 5
```

**Correct:**

```zeta
local x is 5
```

**Also incorrect:**

```zeta
x 5
```

**Correct:**

```zeta
x is now 5
```

---

#### "Expected expression after 'tell'"

**Cause:** The `tell` statement has nothing to print.

**Incorrect:**

```zeta
tell
```

**Correct:**

```zeta
tell {Hello}
```

---

#### "Unexpected token: ..."

**Cause:** The parser encountered a token it did not expect in the current context.

**Incorrect:**

```zeta
local x is 5 +
```

**Correct:**

```zeta
local x is 5 + 3
```

---

#### "Unexpected token '...' at position ..."

**Cause:** The lexer found an invalid character.

**Incorrect:**

```zeta
local x is @ 5
```

**Correct:**

```zeta
local x is 5
```

---

### Semantic Errors

#### "Undefined variable: '...'"

**Cause:** You are using a variable that has not been declared.

**Incorrect:**

```zeta
tell x
```

**Correct:**

```zeta
local x is 10
tell x
```

**Also incorrect:**

```zeta
ask name
```

**Correct:**

```zeta
local name is {}
ask name
```

---

#### "Cannot reassign constant: '...'"

**Cause:** You are trying to change a global variable with `is now`.

**Incorrect:**

```zeta
global max is 100
max is now 200
```

**Correct:**

```zeta
global max is 100
local current is 0
current is now 200
tell {Max is } max
tell {Current is } current
```

Use `global` for values that should never change. Use `local` for values that need to be updated.

---

#### "Type mismatch: cannot assign ... to ..."

**Cause:** You are assigning a value of one type to a variable of a different type.

**Incorrect:**

```zeta
local count is 0
count is now {hello}
```

**Correct:**

```zeta
local count is 0
count is now 5
```

**Also incorrect:**

```zeta
local name is {Alice}
name is now 42
```

**Correct:**

```zeta
local name is {Alice}
name is now {Bob}
```

---

#### "Type mismatch in binary expression: ..."

**Cause:** You are combining incompatible types in an expression.

**Incorrect:**

```zeta
local x is 5 + {hello}
```

**Correct:**

```zeta
local x is 5 + 3
```

---

#### "Variable '...' already declared"

**Cause:** You declared the same variable name twice in the same scope.

**Incorrect:**

```zeta
local x is 5
local x is 10
```

**Correct:**

```zeta
local x is 5
x is now 10
```

---

## Troubleshooting

### Java Not Found

**Symptom:**

```bash
java: command not found
```

**Solution:**

1. Check if Java is installed:

```bash
java -version
```

2. If not installed, download and install JDK 22 or higher from [oracle.com/java](https://www.oracle.com/java/) or use a package manager:

```bash
# macOS with Homebrew
brew install openjdk@22

# Ubuntu/Debian
sudo apt install openjdk-22-jdk
```

3. Set the `JAVA_HOME` environment variable:

```bash
# macOS (add to ~/.zshrc or ~/.bash_profile)
export JAVA_HOME=$(/usr/libexec/java_home -v 22)

# Linux (add to ~/.bashrc)
export JAVA_HOME=/usr/lib/jvm/java-22-openjdk
```

4. Verify:

```bash
echo $JAVA_HOME
java -version
```

---

### Maven Not Found

**Symptom:**

```bash
mvn: command not found
```

**Solution:**

1. Install Maven:

```bash
# macOS with Homebrew
brew install maven

# Ubuntu/Debian
sudo apt install maven
```

2. If you are using IntelliJ IDEA, you can use the bundled Maven:

```bash
# The bundled Maven is usually located at:
/Applications/IntelliJ\ IDEA.app/Contents/plugins/maven/lib/maven3/bin/mvn
```

3. Add it to your PATH or create an alias:

```bash
alias mvn='/Applications/IntelliJ\ IDEA.app/Contents/plugins/maven/lib/maven3/bin/mvn'
```

4. Verify:

```bash
mvn -version
```

---

### "Unable to Access Jarfile"

**Symptom:**

```bash
Error: Unable to access jarfile target/Zeta-1.0-SNAPSHOT.jar
```

**Cause:** The JAR file has not been built yet.

**Solution:**

Build the project first:

```bash
mvn clean package
```

Then verify the JAR exists:

```bash
ls target/Zeta-1.0-SNAPSHOT.jar
```

If you are using the `zetac` wrapper script, make sure you are in the project root directory where both `zetac` and `target/` exist.

---

### Permission Denied When Running `zetac`

**Symptom:**

```bash
./zetac: Permission denied
```

**Solution:**

Make the script executable:

```bash
chmod +x zetac
```

---

### Build Failures

**Symptom:** Maven build fails with compilation errors.

**Solution:**

1. Make sure you are using Java 22 or higher:

```bash
java -version
```

2. Clean and rebuild:

```bash
mvn clean package
```

3. If tests fail, you can skip them (not recommended for production):

```bash
mvn clean package -DskipTests
```

---

## Advanced Usage

### Running with a Specific Java Version

If you have multiple Java versions installed, specify which one to use:

```bash
JAVA_HOME=/path/to/jdk22 java -jar target/Zeta-1.0-SNAPSHOT.jar program.zeta
```

### Building from Source with Custom Settings

#### Skip Tests

```bash
mvn clean package -DskipTests
```

#### Build with Verbose Output

```bash
mvn clean package -X
```

#### Build Only (No Tests)

```bash
mvn clean compile
```

This compiles the source but does not create the JAR. Use `mvn package` to create the JAR.

### Running Tests

```bash
mvn test
```

### Running a Specific Test Class

```bash
mvn test -Dtest=LexerTest
mvn test -Dtest=ParserTest
mvn test -Dtest=SemanticAnalyzerTest
mvn test -Dtest=InterpreterTest
mvn test -Dtest=EndToEndTest
```

### Cleaning Build Artifacts

```bash
mvn clean
```

This removes the `target/` directory. You will need to run `mvn package` again to rebuild.

### Creating a Standalone Distribution

If you want to distribute Zeta without requiring Maven on the target machine:

1. Build the JAR:

```bash
mvn clean package
```

2. Copy the JAR and the wrapper script to your distribution directory:

```bash
mkdir zeta-dist
cp target/Zeta-1.0-SNAPSHOT.jar zeta-dist/
cp zetac zeta-dist/
```

3. The wrapper script assumes the JAR is in `target/`. For a standalone distribution, modify the script:

```bash
#!/bin/bash
SCRIPT_DIR=$(dirname "$0")
java -jar "$SCRIPT_DIR/Zeta-1.0-SNAPSHOT.jar" "$@"
```

4. Users can then run:

```bash
./zeta-dist/zetac program.zeta
```

### Environment Variables

| Variable | Description |
|----------|-------------|
| `JAVA_HOME` | Path to the JDK installation. Must be JDK 22 or higher. |
| `PATH` | Should include `$JAVA_HOME/bin` and the directory containing `mvn`. |

---

## Summary

Zeta is a simple, English-like programming language designed for educational purposes. This guide covered:

- Installing Java 22+ and Maven 3.8+
- Building the project with `mvn clean package`
- Running programs with `./zetac`, `java -jar`, or `mvn exec:java`
- Writing programs with variables, types, operators, I/O, and comments
- Understanding and fixing common error messages
- Troubleshooting installation and build issues
- Advanced topics like running with specific Java versions and creating standalone distributions

For more examples, see the `examples/` directory in the project root.
