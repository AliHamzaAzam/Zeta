package org.azaleas.compiler.integration;

import org.junit.jupiter.api.Test;
import org.azaleas.compiler.ast.Program;
import org.azaleas.compiler.errors.ErrorHandler;
import org.azaleas.compiler.interpreter.Interpreter;
import org.azaleas.compiler.lexer.Lexer;
import org.azaleas.compiler.lexer.Preprocessor;
import org.azaleas.compiler.lexer.Token;
import org.azaleas.compiler.parser.Parser;
import org.azaleas.compiler.semantic.SemanticAnalyzer;

import java.io.ByteArrayOutputStream;
import java.io.PrintStream;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.List;

import static org.junit.jupiter.api.Assertions.*;

public class EndToEndTest {
    private String runProgram(String source) {
        Preprocessor preprocessor = new Preprocessor();
        String preprocessed = preprocessor.process(source);
        
        ErrorHandler eh = new ErrorHandler();
        Lexer lexer = new Lexer(eh);
        List<Token> tokens = lexer.tokenize(preprocessed);
        
        Parser parser = new Parser(tokens, eh);
        Program program = parser.parseProgram();
        
        SemanticAnalyzer analyzer = new SemanticAnalyzer(eh);
        analyzer.analyze(program);
        
        assertFalse(eh.hasErrors(), "Semantic errors: " + eh.getErrors());
        
        ByteArrayOutputStream baos = new ByteArrayOutputStream();
        PrintStream originalOut = System.out;
        System.setOut(new PrintStream(baos));
        
        Interpreter interpreter = new Interpreter();
        interpreter.interpret(program);
        
        System.setOut(originalOut);
        return baos.toString();
    }
    
    @Test
    public void testHelloWorld() throws Exception {
        String source = Files.readString(Path.of("examples/01_hello.zeta"));
        String output = runProgram(source);
        assertEquals("Hello, World!\n", output);
    }
    
    @Test
    public void testArithmetic() throws Exception {
        String source = Files.readString(Path.of("examples/02_arithmetic.zeta"));
        String output = runProgram(source);
        assertTrue(output.contains("78.5"), "Expected 78.5 in output, got: " + output);
    }
    
    @Test
    public void testPrecedence() throws Exception {
        String source = Files.readString(Path.of("examples/05_precedence.zeta"));
        String output = runProgram(source);
        assertTrue(output.contains("14"), "Expected 14 in output");
        assertTrue(output.contains("20"), "Expected 20 in output");
        assertTrue(output.contains("512"), "Expected 512 in output");
    }
    
    @Test
    public void testScoping() throws Exception {
        String source = Files.readString(Path.of("examples/04_scoping.zeta"));
        String output = runProgram(source);
        assertTrue(output.contains("Max is 100"), "Expected 'Max is 100'");
        assertTrue(output.contains("Count is 0"), "Expected 'Count is 0'");
        assertTrue(output.contains("Count is now 1"), "Expected 'Count is now 1'");
    }

    @Test
    public void testSingleLineComment() {
        String output = runProgram("< this is a comment >\ntell {hi}");
        assertEquals("hi\n", output);
    }

    @Test
    public void testBlockCommentOnOneLine() {
        String output = runProgram("<< inline block comment >>\ntell {ok}");
        assertEquals("ok\n", output);
    }

    @Test
    public void testMultiLineComment() {
        String source = "<< this comment\n   spans several\n   lines >>\ntell {after}";
        String output = runProgram(source);
        assertEquals("after\n", output);
    }

    @Test
    public void testBlockCommentBetweenTokens() {
        String output = runProgram("local x is 5 << note >>\ntell {} x");
        assertEquals("5\n", output);
    }
}
