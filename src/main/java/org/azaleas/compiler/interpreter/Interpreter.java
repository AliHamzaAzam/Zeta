package org.azaleas.compiler.interpreter;
import org.azaleas.compiler.ast.*;
import org.azaleas.compiler.semantic.ZetaType;
import java.util.Scanner;

public class Interpreter {
    private Environment environment = new Environment();
    private final Scanner scanner = new Scanner(System.in);
    
    public void interpret(Program program) {
        for (Statement stmt : program.statements()) {
            execute(stmt);
        }
    }
    
    private void execute(Statement stmt) {
        if (stmt instanceof VarDecl decl) {
            ZetaValue value = evaluate(decl.initializer());
            environment.define(decl.name(), value);
        } else if (stmt instanceof Assign assign) {
            ZetaValue value = evaluate(assign.value());
            environment.set(assign.name(), value);
        } else if (stmt instanceof Tell tell) {
            StringBuilder output = new StringBuilder();
            for (Expression expr : tell.expressions()) {
                ZetaValue value = evaluate(expr);
                output.append(value.toString());
            }
            System.out.println(output.toString());
        } else if (stmt instanceof Ask ask) {
            String input = scanner.nextLine();
            ZetaValue current = environment.get(ask.variableName());
            ZetaValue value;
            if (current.type() == ZetaType.INT) {
                value = new ZetaValue(ZetaType.INT, Integer.parseInt(input));
            } else if (current.type() == ZetaType.DECIMAL) {
                value = new ZetaValue(ZetaType.DECIMAL, Double.parseDouble(input));
            } else {
                value = new ZetaValue(ZetaType.STRING, input);
            }
            environment.set(ask.variableName(), value);
        }
    }
    
    private ZetaValue evaluate(Expression expr) {
        if (expr instanceof Literal lit) {
            Object value = lit.value();
            if (value instanceof Integer) return new ZetaValue(ZetaType.INT, value);
            if (value instanceof Double) return new ZetaValue(ZetaType.DECIMAL, value);
            if (value instanceof String) return new ZetaValue(ZetaType.STRING, value);
            if (value instanceof Boolean) return new ZetaValue(ZetaType.BOOL, value);
        } else if (expr instanceof Variable var) {
            return environment.get(var.name());
        } else if (expr instanceof BinaryExpr bin) {
            ZetaValue left = evaluate(bin.left());
            ZetaValue right = evaluate(bin.right());
            return evaluateBinary(left, bin.operator(), right);
        }
        throw new RuntimeException("Unknown expression type");
    }
    
    private ZetaValue evaluateBinary(ZetaValue left, String op, ZetaValue right) {
        switch (op) {
            case "+":
                if (left.type() == ZetaType.STRING || right.type() == ZetaType.STRING) {
                    return new ZetaValue(ZetaType.STRING, left.toString() + right.toString());
                }
                if (left.type() == ZetaType.DECIMAL || right.type() == ZetaType.DECIMAL) {
                    return new ZetaValue(ZetaType.DECIMAL, left.asDouble() + right.asDouble());
                }
                return new ZetaValue(ZetaType.INT, left.asInt() + right.asInt());
            case "-":
                if (left.type() == ZetaType.DECIMAL || right.type() == ZetaType.DECIMAL) {
                    return new ZetaValue(ZetaType.DECIMAL, left.asDouble() - right.asDouble());
                }
                return new ZetaValue(ZetaType.INT, left.asInt() - right.asInt());
            case "*":
                if (left.type() == ZetaType.DECIMAL || right.type() == ZetaType.DECIMAL) {
                    return new ZetaValue(ZetaType.DECIMAL, left.asDouble() * right.asDouble());
                }
                return new ZetaValue(ZetaType.INT, left.asInt() * right.asInt());
            case "/":
                return new ZetaValue(ZetaType.DECIMAL, left.asDouble() / right.asDouble());
            case "%":
                return new ZetaValue(ZetaType.INT, left.asInt() % right.asInt());
            case "^":
                return new ZetaValue(ZetaType.DECIMAL, Math.pow(left.asDouble(), right.asDouble()));
            default:
                throw new RuntimeException("Unknown operator: " + op);
        }
    }
}
