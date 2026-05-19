package org.azaleas.compiler.ast;

import java.util.List;

public record Tell(List<Expression> expressions) implements Statement {}
