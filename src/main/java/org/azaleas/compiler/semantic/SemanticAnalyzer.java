package org.azaleas.compiler.semantic;

import org.azaleas.compiler.ast.*;
import org.azaleas.compiler.errors.ErrorHandler;
import org.azaleas.compiler.semantic.Scope.SymbolInfo;

public class SemanticAnalyzer {
    private Scope currentScope;
    private final ErrorHandler errorHandler;

    public SemanticAnalyzer(ErrorHandler errorHandler) {
        this.errorHandler = errorHandler;
        this.currentScope = new Scope();
    }

    public void analyze(Program program) {
        for (Statement stmt : program.statements()) {
            analyzeStatement(stmt);
        }
    }

    private void analyzeStatement(Statement stmt) {
        if (stmt instanceof VarDecl decl) {
            ZetaType type = inferType(decl.initializer());
            boolean isConstant = decl.scope().equals("global");
            boolean success = currentScope.declare(decl.name(), type, isConstant);
            if (!success) {
                errorHandler.addError(0, "Variable '" + decl.name() + "' already declared");
            }
        } else if (stmt instanceof Assign assign) {
            SymbolInfo info = currentScope.lookup(assign.name());
            if (info == null) {
                errorHandler.addError(0, "Undefined variable: '" + assign.name() + "'");
            } else if (info.isConstant) {
                errorHandler.addError(0, "Cannot reassign constant: '" + assign.name() + "'");
            } else {
                ZetaType valueType = inferType(assign.value());
                if (!isCompatible(info.type, valueType)) {
                    errorHandler.addError(0, "Type mismatch: cannot assign " + valueType + " to " + info.type);
                }
            }
        } else if (stmt instanceof Tell tell) {
            for (Expression expr : tell.expressions()) {
                inferType(expr);
            }
        } else if (stmt instanceof Ask ask) {
            SymbolInfo info = currentScope.lookup(ask.variableName());
            if (info == null) {
                errorHandler.addError(0, "Undefined variable: '" + ask.variableName() + "'");
            }
        }
    }

    private ZetaType inferType(Expression expr) {
        if (expr instanceof Literal lit) {
            Object value = lit.value();
            if (value instanceof Integer) return ZetaType.INT;
            if (value instanceof Double) return ZetaType.DECIMAL;
            if (value instanceof String) return ZetaType.STRING;
            if (value instanceof Boolean) return ZetaType.BOOL;
            return ZetaType.UNKNOWN;
        } else if (expr instanceof Variable var) {
            SymbolInfo info = currentScope.lookup(var.name());
            if (info == null) {
                errorHandler.addError(0, "Undefined variable: '" + var.name() + "'");
                return ZetaType.UNKNOWN;
            }
            return info.type;
        } else if (expr instanceof BinaryExpr bin) {
            ZetaType left = inferType(bin.left());
            ZetaType right = inferType(bin.right());
            return resolveBinaryType(left, bin.operator(), right);
        }
        return ZetaType.UNKNOWN;
    }

    private ZetaType resolveBinaryType(ZetaType left, String op, ZetaType right) {
        if (op.equals("+")) {
            if (left == ZetaType.STRING || right == ZetaType.STRING) return ZetaType.STRING;
            if (left == ZetaType.DECIMAL || right == ZetaType.DECIMAL) return ZetaType.DECIMAL;
            if (left == ZetaType.INT && right == ZetaType.INT) return ZetaType.INT;
        }
        if (op.equals("-") || op.equals("*") || op.equals("/") || op.equals("%") || op.equals("^")) {
            if (left == ZetaType.DECIMAL || right == ZetaType.DECIMAL) return ZetaType.DECIMAL;
            if (left == ZetaType.INT && right == ZetaType.INT) return ZetaType.INT;
        }
        errorHandler.addError(0, "Type mismatch in binary expression: " + left + " " + op + " " + right);
        return ZetaType.UNKNOWN;
    }

    private boolean isCompatible(ZetaType target, ZetaType source) {
        if (target == source) return true;
        if (target == ZetaType.DECIMAL && source == ZetaType.INT) return true;
        return false;
    }
}
