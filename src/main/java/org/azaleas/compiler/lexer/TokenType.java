package org.azaleas.compiler.lexer;

public enum TokenType {
    WHITESPACE,
    MULTI_LINE_COMMENT,
    SINGLE_LINE_COMMENT,
    STRING_OR_CHAR,
    KEYWORD,
    EXPONENT,
    OPERATOR,
    DECIMAL,
    INTEGER,
    IDENTIFIER,
    LEFT_PAREN,
    RIGHT_PAREN,
    ERROR,
    EOF
}