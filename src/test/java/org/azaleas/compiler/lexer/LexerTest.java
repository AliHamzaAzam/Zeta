package org.azaleas.compiler.lexer;

import org.junit.jupiter.api.Test;
import org.azaleas.compiler.errors.ErrorHandler;
import java.util.List;
import static org.junit.jupiter.api.Assertions.*;

public class LexerTest {
    @Test
    public void testTokenizeHelloWorld() {
        ErrorHandler eh = new ErrorHandler();
        Lexer lexer = new Lexer(eh);
        List<Token> tokens = lexer.tokenize("tell {Hello, World!}");
        
        assertEquals(TokenType.KEYWORD, tokens.get(0).type());
        assertEquals("tell", tokens.get(0).value());
        assertEquals(TokenType.STRING_OR_CHAR, tokens.get(1).type());
        assertEquals(TokenType.EOF, tokens.get(tokens.size() - 1).type());
    }
    
    @Test
    public void testIdentifierWithDigits() {
        ErrorHandler eh = new ErrorHandler();
        Lexer lexer = new Lexer(eh);
        List<Token> tokens = lexer.tokenize("x1 is 5");
        
        assertEquals(TokenType.IDENTIFIER, tokens.get(0).type());
        assertEquals("x1", tokens.get(0).value());
    }
    
    @Test
    public void testEOFToken() {
        ErrorHandler eh = new ErrorHandler();
        Lexer lexer = new Lexer(eh);
        List<Token> tokens = lexer.tokenize("");
        
        assertEquals(TokenType.EOF, tokens.get(0).type());
    }
    
    @Test
    public void testArithmeticOperators() {
        ErrorHandler eh = new ErrorHandler();
        Lexer lexer = new Lexer(eh);
        List<Token> tokens = lexer.tokenize("2 + 3 * 4");
        
        assertEquals(TokenType.INTEGER, tokens.get(0).type());
        assertEquals(TokenType.OPERATOR, tokens.get(1).type());
        assertEquals("+", tokens.get(1).value());
        assertEquals(TokenType.INTEGER, tokens.get(2).type());
        assertEquals(TokenType.OPERATOR, tokens.get(3).type());
        assertEquals("*", tokens.get(3).value());
    }
    
    @Test
    public void testExponentOperator() {
        ErrorHandler eh = new ErrorHandler();
        Lexer lexer = new Lexer(eh);
        List<Token> tokens = lexer.tokenize("2 ^ 3");

        assertEquals(TokenType.EXPONENT, tokens.get(1).type());
        assertEquals("^", tokens.get(1).value());
    }

    @Test
    public void testCommentsAreSkippedAndProduceNoTokens() {
        ErrorHandler eh = new ErrorHandler();
        Lexer lexer = new Lexer(eh);
        List<Token> tokens = lexer.tokenize("<< multi\nline >> tell {x} < single >");

        assertFalse(eh.hasErrors(), "Comments should not produce lexical errors");
        assertEquals(TokenType.KEYWORD, tokens.get(0).type());
        assertEquals("tell", tokens.get(0).value());
        assertEquals(TokenType.STRING_OR_CHAR, tokens.get(1).type());
        assertEquals(TokenType.EOF, tokens.get(2).type());
    }
}
