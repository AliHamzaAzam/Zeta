package org.azaleas.compiler.semantic;

import java.util.HashMap;
import java.util.Map;

public class Scope {
    private final Scope parent;
    private final Map<String, SymbolInfo> symbols = new HashMap<>();

    public Scope() {
        this.parent = null;
    }

    public Scope(Scope parent) {
        this.parent = parent;
    }

    public boolean declare(String name, ZetaType type, boolean isConstant) {
        if (symbols.containsKey(name)) {
            return false;
        }
        symbols.put(name, new SymbolInfo(type, isConstant));
        return true;
    }

    public SymbolInfo lookup(String name) {
        if (symbols.containsKey(name)) return symbols.get(name);
        if (parent != null) return parent.lookup(name);
        return null;
    }

    public boolean isDeclaredLocally(String name) {
        return symbols.containsKey(name);
    }

    public static class SymbolInfo {
        public final ZetaType type;
        public final boolean isConstant;

        public SymbolInfo(ZetaType type, boolean isConstant) {
            this.type = type;
            this.isConstant = isConstant;
        }
    }
}
