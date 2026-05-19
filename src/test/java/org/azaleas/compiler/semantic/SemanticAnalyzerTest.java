package org.azaleas.compiler.semantic;

import org.junit.jupiter.api.Test;
import org.azaleas.compiler.ast.Program;
import org.azaleas.compiler.errors.ErrorHandler;
import org.azaleas.compiler.lexer.Lexer;
import org.azaleas.compiler.lexer.Token;
import org.azaleas.compiler.parser.Parser;
import java.util.List;
import static org.junit.jupiter.api.Assertions.*;

public class SemanticAnalyzerTest {
    private ErrorHandler analyze(String source) {
        ErrorHandler eh = new ErrorHandler();
        Lexer lexer = new Lexer(eh);
        List<Token> tokens = lexer.tokenize(source);
        Parser parser = new Parser(tokens, eh);
        Program program = parser.parseProgram();
        SemanticAnalyzer analyzer = new SemanticAnalyzer(eh);
        analyzer.analyze(program);
        return eh;
    }
    
    @Test
    public void testValidProgram() {
        ErrorHandler eh = analyze("global x is 5\ntell {Hello}");
        assertFalse(eh.hasErrors());
    }
    
    @Test
    public void testUndefinedVariable() {
        ErrorHandler eh = analyze("tell x");
        assertTrue(eh.hasErrors());
    }
    
    @Test
    public void testConstantReassignment() {
        ErrorHandler eh = analyze("global max is 100\nmax is now 200");
        assertTrue(eh.hasErrors());
    }
    
    @Test
    public void testDuplicateDeclaration() {
        ErrorHandler eh = analyze("global x is 5\nglobal x is 10");
        assertTrue(eh.hasErrors());
    }
    
    @Test
    public void testTypeMismatch() {
        ErrorHandler eh = analyze("global x is 5\nx is now {hello}");
        assertTrue(eh.hasErrors());
    }
}
