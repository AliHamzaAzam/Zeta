package org.azaleas.compiler.lexer;


public record Token(TokenType type, String value, int line, int column) {
}
