package org.azaleas.compiler.interpreter;
import java.util.*;

public class Environment {
    private final Environment parent;
    private final Map<String, ZetaValue> variables = new HashMap<>();
    
    public Environment() { this.parent = null; }
    public Environment(Environment parent) { this.parent = parent; }
    
    public void define(String name, ZetaValue value) {
        variables.put(name, value);
    }
    
    public ZetaValue get(String name) {
        if (variables.containsKey(name)) return variables.get(name);
        if (parent != null) return parent.get(name);
        throw new RuntimeException("Undefined variable: '" + name + "'");
    }
    
    public void set(String name, ZetaValue value) {
        if (variables.containsKey(name)) {
            variables.put(name, value);
            return;
        }
        if (parent != null) {
            parent.set(name, value);
            return;
        }
        throw new RuntimeException("Undefined variable: '" + name + "'");
    }
}
