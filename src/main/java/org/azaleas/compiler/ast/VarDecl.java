package org.azaleas.compiler.ast;

public record VarDecl(String scope, String name, Expression initializer) implements Statement {}
