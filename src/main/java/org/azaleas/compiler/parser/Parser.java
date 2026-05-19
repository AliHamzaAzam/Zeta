package org.azaleas.compiler.parser;

import org.azaleas.compiler.ast.*;
import org.azaleas.compiler.errors.ErrorHandler;
import org.azaleas.compiler.lexer.Token;
import org.azaleas.compiler.lexer.TokenType;

import java.util.ArrayList;
import java.util.List;

public class Parser {
    private final List<Token> tokens;
    private final ErrorHandler errorHandler;
    private int position = 0;

    public Parser(List<Token> tokens, ErrorHandler errorHandler) {
        this.tokens = tokens;
        this.errorHandler = errorHandler;
    }

    public Program parseProgram() {
        List<Statement> statements = new ArrayList<>();
        while (!check(TokenType.EOF)) {
            statements.add(parseStatement());
        }
        return new Program(statements);
    }

    private Statement parseStatement() {
        if (match(TokenType.KEYWORD, "global") || match(TokenType.KEYWORD, "local")) {
            return parseVarDecl(previous().value());
        } else if (match(TokenType.IDENTIFIER)) {
            return parseAssignOrExpr();
        } else if (match(TokenType.KEYWORD, "tell")) {
            return parseTell();
        } else if (match(TokenType.KEYWORD, "ask")) {
            return parseAsk();
        }
        errorHandler.addError(peek().line(), "Unexpected token: " + peek().value());
        advance();
        return null;
    }

    private VarDecl parseVarDecl(String scope) {
        String name = consume(TokenType.IDENTIFIER, "Expected variable name").value();
        consumeKeyword("is", "Expected 'is' after variable name");
        Expression initializer = parseExpression();
        return new VarDecl(scope, name, initializer);
    }

    private Statement parseAssignOrExpr() {
        String name = previous().value();
        if (matchKeyword("is")) {
            if (matchKeyword("now")) {
                Expression value = parseExpression();
                return new Assign(name, value);
            }
            Expression initializer = parseExpression();
            return new VarDecl("local", name, initializer);
        }
        errorHandler.addError(peek().line(), "Expected 'is' after identifier");
        return null;
    }

    private Tell parseTell() {
        List<Expression> expressions = new ArrayList<>();
        while (!check(TokenType.EOF) && !isStatementStart(peek())) {
            expressions.add(parseExpression());
        }
        if (expressions.isEmpty()) {
            errorHandler.addError(peek().line(), "Expected expression after 'tell'");
        }
        return new Tell(expressions);
    }

    private boolean isStatementStart(Token token) {
        if (token.type() == TokenType.KEYWORD) {
            String value = token.value();
            return value.equals("global") || value.equals("local") ||
                   value.equals("tell") || value.equals("ask");
        }
        if (token.type() == TokenType.IDENTIFIER) {
            Token next = peek(1);
            return next.type() == TokenType.KEYWORD && next.value().equals("is");
        }
        return false;
    }

    private Ask parseAsk() {
        String name = consume(TokenType.IDENTIFIER, "Expected variable name after 'ask'").value();
        return new Ask(name);
    }

    private Expression parseExpression() {
        return parseAdditive();
    }

    private Expression parseAdditive() {
        Expression left = parseMultiplicative();
        while (match(TokenType.OPERATOR, "+") || match(TokenType.OPERATOR, "-")) {
            String op = previous().value();
            Expression right = parseMultiplicative();
            left = new BinaryExpr(left, op, right);
        }
        return left;
    }

    private Expression parseMultiplicative() {
        Expression left = parsePower();
        while (match(TokenType.OPERATOR, "*") || match(TokenType.OPERATOR, "/") || match(TokenType.OPERATOR, "%")) {
            String op = previous().value();
            Expression right = parsePower();
            left = new BinaryExpr(left, op, right);
        }
        return left;
    }

    private Expression parsePower() {
        Expression left = parseUnary();
        if (match(TokenType.EXPONENT, "^")) {
            String op = previous().value();
            Expression right = parsePower();
            return new BinaryExpr(left, op, right);
        }
        return left;
    }

    private Expression parseUnary() {
        if (match(TokenType.OPERATOR, "+") || match(TokenType.OPERATOR, "-")) {
            String op = previous().value();
            Expression operand = parseUnary();
            return new BinaryExpr(new Literal(0), op, operand);
        }
        return parsePrimary();
    }

    private Expression parsePrimary() {
        if (match(TokenType.INTEGER)) {
            return new Literal(Integer.parseInt(previous().value()));
        }
        if (match(TokenType.DECIMAL)) {
            return new Literal(Double.parseDouble(previous().value()));
        }
        if (match(TokenType.STRING_OR_CHAR)) {
            String value = previous().value();
            return new Literal(value.substring(1, value.length() - 1));
        }
        if (match(TokenType.KEYWORD, "true") || match(TokenType.KEYWORD, "false")) {
            return new Literal(Boolean.parseBoolean(previous().value()));
        }
        if (match(TokenType.IDENTIFIER)) {
            return new Variable(previous().value());
        }
        if (match(TokenType.LEFT_PAREN)) {
            Expression expr = parseExpression();
            consume(TokenType.RIGHT_PAREN, "Expected ')' after expression");
            return expr;
        }
        errorHandler.addError(peek().line(), "Unexpected token in expression: " + peek().value());
        advance();
        return new Literal(null);
    }

    private Token consume(TokenType type, String message) {
        if (check(type)) return advance();
        errorHandler.addError(peek().line(), message);
        return peek();
    }

    private void consumeKeyword(String keyword, String message) {
        if (check(TokenType.KEYWORD) && peek().value().equals(keyword)) {
            advance();
            return;
        }
        errorHandler.addError(peek().line(), message);
    }

    private boolean match(TokenType type) {
        if (check(type)) {
            advance();
            return true;
        }
        return false;
    }

    private boolean match(TokenType type, String value) {
        if (check(type) && peek().value().equals(value)) {
            advance();
            return true;
        }
        return false;
    }

    private boolean matchKeyword(String keyword) {
        return match(TokenType.KEYWORD, keyword);
    }

    private boolean check(TokenType type) {
        return peek().type() == type;
    }

    private boolean check(TokenType type, String value) {
        return peek().type() == type && peek().value().equals(value);
    }

    private Token advance() {
        if (!isAtEnd()) position++;
        return previous();
    }

    private boolean isAtEnd() {
        return peek().type() == TokenType.EOF;
    }

    private Token peek() {
        return tokens.get(position);
    }

    private Token peek(int offset) {
        int index = position + offset;
        if (index >= tokens.size()) {
            return tokens.get(tokens.size() - 1);
        }
        return tokens.get(index);
    }

    private Token previous() {
        return tokens.get(position - 1);
    }
}
