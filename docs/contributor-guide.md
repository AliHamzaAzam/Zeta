# Zeta Contributor Guide

## Project Structure
```
src/main/java/org/azaleas/compiler/
  ast/         - AST node classes
  lexer/       - Lexer and preprocessor
  parser/      - Recursive descent parser
  semantic/    - Semantic analyzer
  interpreter/ - Tree-walking interpreter
  cli/         - CLI entrypoint
  errors/      - Error handling
  symboltable/ - Symbol table (legacy)
  automata/    - NFA/DFA framework
```

## Adding a New Statement Type
1. Add AST node in `ast/`
2. Add parsing logic in `parser/Parser.java`
3. Add semantic check in `semantic/SemanticAnalyzer.java`
4. Add execution in `interpreter/Interpreter.java`
5. Add test in `src/test/java/`

## Testing
Run all tests:
```bash
mvn test
```
