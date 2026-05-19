package org.azaleas.compiler.ast;

public record BinaryExpr(Expression left, String operator, Expression right) implements Expression {}
