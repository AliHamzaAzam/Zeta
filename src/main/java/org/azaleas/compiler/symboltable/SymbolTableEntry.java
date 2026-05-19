package org.azaleas.compiler.symboltable;

import org.azaleas.compiler.semantic.ZetaType;

import java.util.Objects;

public class SymbolTableEntry {
    private String name;
    private ZetaType type;
    private Object value;
    private String scope;
    private boolean isConstant;

    public SymbolTableEntry(String name, ZetaType type, Object value, String scope, boolean isConstant) {
        this.name = name;
        this.type = type;
        this.value = value;
        this.scope = scope;
        this.isConstant = isConstant;
    }

    public String getName() {
        return name;
    }

    public ZetaType getType() {
        return type;
    }

    public Object getValue() {
        return value;
    }

    public String getScope() {
        return scope;
    }

    public boolean isConstant() {
        return isConstant;
    }

    public void setValue(Object value) {
        this.value = value;
    }

    @Override
    public String toString() {
        return "SymbolTableEntry{" +
                "name='" + name + '\'' +
                ", type=" + type +
                ", value=" + value +
                ", scope=" + scope +
                ", isConstant=" + isConstant +
                '}';
    }

    @Override
    public boolean equals(Object obj) {
        if (obj instanceof SymbolTableEntry) {
            SymbolTableEntry entry = (SymbolTableEntry) obj;
            return name.equals(entry.name) && type == entry.type && Objects.equals(scope, entry.scope) && isConstant == entry.isConstant;
        }
        return false;
    }

    @Override
    public int hashCode() {
        return name.hashCode() + type.hashCode() + scope.hashCode() + (isConstant ? 1 : 0);
    }
}