package org.azaleas.compiler.interpreter;

import org.junit.jupiter.api.Test;
import org.azaleas.compiler.ast.Program;
import org.azaleas.compiler.errors.ErrorHandler;
import org.azaleas.compiler.lexer.Lexer;
import org.azaleas.compiler.lexer.Token;
import org.azaleas.compiler.parser.Parser;
import org.azaleas.compiler.semantic.SemanticAnalyzer;
import java.io.ByteArrayOutputStream;
import java.io.PrintStream;
import java.util.List;
import static org.junit.jupiter.api.Assertions.*;

public class InterpreterTest {
    private String run(String source) {
        ErrorHandler eh = new ErrorHandler();
        Lexer lexer = new Lexer(eh);
        List<Token> tokens = lexer.tokenize(source);
        Parser parser = new Parser(tokens, eh);
        Program program = parser.parseProgram();
        SemanticAnalyzer analyzer = new SemanticAnalyzer(eh);
        analyzer.analyze(program);
        
        ByteArrayOutputStream baos = new ByteArrayOutputStream();
        PrintStream originalOut = System.out;
        System.setOut(new PrintStream(baos));
        
        Interpreter interpreter = new Interpreter();
        interpreter.interpret(program);
        
        System.setOut(originalOut);
        return baos.toString().trim();
    }
    
    @Test
    public void testHelloWorld() {
        String output = run("tell {Hello, World!}");
        assertEquals("Hello, World!", output);
    }
    
    @Test
    public void testArithmetic() {
        String output = run("global x is 2 + 3\ntell x");
        assertEquals("5", output);
    }
    
    @Test
    public void testVariableReuse() {
        String output = run("local x is 5\nx is now 10\ntell x");
        assertEquals("10", output);
    }
    
    @Test
    public void testStringConcatenation() {
        String output = run("global msg is {Hello, } + {World!}\ntell msg");
        assertEquals("Hello, World!", output);
    }
}
