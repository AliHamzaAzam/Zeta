package org.azaleas.compiler.symboltable;

import org.azaleas.compiler.lexer.Token;
import org.azaleas.compiler.lexer.TokenType;
import org.azaleas.compiler.semantic.ZetaType;

import java.util.ArrayList;
import java.util.List;
import java.util.Optional;

public class SymbolTable {
    private List<SymbolTableEntry> symbolTable = new ArrayList<>();

    public void addEntry(String name, ZetaType type, Object value, String scope, boolean isConstant) {
        this.symbolTable.add(new SymbolTableEntry(name, type, value, scope, isConstant));
    }

    public boolean exists(String name) {
        return this.symbolTable.stream().anyMatch(entry -> entry.getName().equals(name));
    }

    public SymbolTableEntry getEntry(String name) {
        return this.symbolTable.stream()
                .filter(entry -> entry.getName().equals(name))
                .findFirst()
                .orElse(null);
    }

    public boolean updateEntry(String name, Object value) {
        Optional<SymbolTableEntry> entry = this.symbolTable.stream()
                .filter(e -> e.getName().equals(name))
                .findFirst();

        if (entry.isPresent()) {
            SymbolTableEntry symbolEntry = entry.get();
            if (!symbolEntry.isConstant()) {
                symbolEntry.setValue(value);
                return true;
            }
        }
        return false;
    }

    public List<SymbolTableEntry> getEntries() {
        return new ArrayList<>(symbolTable);
    }

    private static Object parseValue(Token valueToken) {
        switch (valueToken.type()) {
            case INTEGER:
                return Integer.parseInt(valueToken.value());
            case DECIMAL:
                return Double.parseDouble(valueToken.value());
            case STRING_OR_CHAR:
                String value = valueToken.value();
                return value.substring(1, value.length() - 1); // Remove curly braces
            default:
                throw new IllegalArgumentException("Unexpected value type: " + valueToken.type());
        }
    }

    public List<SymbolTableEntry> populateSymbolTable(List<Token> tokens){
        for(int i=0 ; i<tokens.size() ; i++){
            Token token = tokens.get(i);
            if (token.type() == TokenType.KEYWORD) {
                if((i+3) < tokens.size()){
                    Token varName = tokens.get(i+1);
                    Token dec = tokens.get(i+2);
                    Token varValue = tokens.get(i+3);

                    if(varName.type() == TokenType.IDENTIFIER && dec.type() == TokenType.KEYWORD && dec.value().equals("is") && ( varValue.type() == TokenType.INTEGER || varValue.type() == TokenType.DECIMAL || varValue.type() == TokenType.STRING_OR_CHAR)) {
                        addEntry(token, varName, varValue);
                        i += 3;
                    }
                }
            } else if (token.type() == TokenType.IDENTIFIER){
                if((i+2) < tokens.size()){
                    Token varName = tokens.get(i);
                    Token dec = tokens.get(i+1);
                    Token varValue = tokens.get(i+2);

                    if(varName.type() == TokenType.IDENTIFIER && dec.type() == TokenType.KEYWORD && dec.value().equals("is") && ( varValue.type() == TokenType.INTEGER || varValue.type() == TokenType.DECIMAL || varValue.type() == TokenType.STRING_OR_CHAR)) {
                        addEntry(token, varName, varValue);
                        i += 2;
                    }
                }
            }
        }
        return symbolTable;
    }

    private void addEntry(Token token, Token varName, Token varValue) {
        String name = varName.value();
        ZetaType type = tokenTypeToZetaType(varValue.type());
        Object value = parseValue(varValue);
        String scope = token.value().equals("global") ? "global" : "local";
        boolean isConstant = true;
        addEntry(name, type, value, scope, isConstant);
    }

    private static ZetaType tokenTypeToZetaType(TokenType tokenType) {
        switch (tokenType) {
            case INTEGER:
                return ZetaType.INT;
            case DECIMAL:
                return ZetaType.DECIMAL;
            case STRING_OR_CHAR:
                return ZetaType.STRING;
            default:
                return ZetaType.UNKNOWN;
        }
    }

}