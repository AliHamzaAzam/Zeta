package org.azaleas.compiler.cli;

import org.azaleas.compiler.ast.Program;
import org.azaleas.compiler.errors.ErrorHandler;
import org.azaleas.compiler.interpreter.Interpreter;
import org.azaleas.compiler.lexer.Lexer;
import org.azaleas.compiler.lexer.Preprocessor;
import org.azaleas.compiler.lexer.Token;
import org.azaleas.compiler.parser.Parser;
import org.azaleas.compiler.semantic.SemanticAnalyzer;

import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.List;

public class ZetaCompiler {
    public static void main(String[] args) {
        if (args.length == 0) {
            System.err.println("Usage: zetac <file.zeta>");
            System.exit(1);
        }
        
        String filePath = args[0];
        try {
            String source = Files.readString(Path.of(filePath));

            Preprocessor preprocessor = new Preprocessor();
            String preprocessed = preprocessor.process(source);

            ErrorHandler errorHandler = new ErrorHandler();
            Lexer lexer = new Lexer(errorHandler);
            List<Token> tokens = lexer.tokenize(preprocessed);

            if (errorHandler.hasErrors()) {
                errorHandler.printErrors();
                System.exit(1);
            }

            Parser parser = new Parser(tokens, errorHandler);
            Program program = parser.parseProgram();

            if (errorHandler.hasErrors()) {
                errorHandler.printErrors();
                System.exit(1);
            }

            SemanticAnalyzer analyzer = new SemanticAnalyzer(errorHandler);
            analyzer.analyze(program);

            if (errorHandler.hasErrors()) {
                errorHandler.printErrors();
                System.exit(1);
            }

            Interpreter interpreter = new Interpreter();
            interpreter.interpret(program);
            
        } catch (IOException e) {
            System.err.println("Error reading file: " + e.getMessage());
            System.exit(1);
        }
    }
}
