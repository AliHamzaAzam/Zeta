package org.azaleas.compiler.ast;

public record Assign(String name, Expression value) implements Statement {}
