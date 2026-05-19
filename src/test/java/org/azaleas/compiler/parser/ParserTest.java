package org.azaleas.compiler.parser;

import org.junit.jupiter.api.Test;
import org.azaleas.compiler.ast.*;
import org.azaleas.compiler.errors.ErrorHandler;
import org.azaleas.compiler.lexer.Lexer;
import org.azaleas.compiler.lexer.Token;
import java.util.List;
import static org.junit.jupiter.api.Assertions.*;

public class ParserTest {
    private Program parse(String source) {
        ErrorHandler eh = new ErrorHandler();
        Lexer lexer = new Lexer(eh);
        List<Token> tokens = lexer.tokenize(source);
        Parser parser = new Parser(tokens, eh);
        return parser.parseProgram();
    }
    
    @Test
    public void testParseHelloWorld() {
        Program program = parse("tell {Hello, World!}");
        assertEquals(1, program.statements().size());
        assertTrue(program.statements().get(0) instanceof Tell);
    }
    
    @Test
    public void testParseVariableDeclaration() {
        Program program = parse("global x is 5");
        assertEquals(1, program.statements().size());
        VarDecl decl = (VarDecl) program.statements().get(0);
        assertEquals("global", decl.scope());
        assertEquals("x", decl.name());
    }
    
    @Test
    public void testParseAssignment() {
        Program program = parse("x is now 10");
        assertEquals(1, program.statements().size());
        Assign assign = (Assign) program.statements().get(0);
        assertEquals("x", assign.name());
    }
    
    @Test
    public void testParseAsk() {
        Program program = parse("ask radius");
        assertEquals(1, program.statements().size());
        Ask ask = (Ask) program.statements().get(0);
        assertEquals("radius", ask.variableName());
    }
    
    @Test
    public void testParseArithmeticExpression() {
        Program program = parse("global x is 2 + 3 * 4");
        VarDecl decl = (VarDecl) program.statements().get(0);
        assertTrue(decl.initializer() instanceof BinaryExpr);
    }
}
