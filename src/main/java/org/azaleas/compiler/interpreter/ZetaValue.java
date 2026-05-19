package org.azaleas.compiler.interpreter;
import org.azaleas.compiler.semantic.ZetaType;

public record ZetaValue(ZetaType type, Object value) {
    public int asInt() { return ((Number) value).intValue(); }
    public double asDouble() { return ((Number) value).doubleValue(); }
    public String asString() { return (String) value; }
    public boolean asBool() { return (Boolean) value; }
    
    @Override
    public String toString() {
        return value.toString();
    }
}
