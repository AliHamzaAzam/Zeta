package org.azaleas.compiler.lexer;

public class Preprocessor {
    public String process(String input) {
        StringBuilder result = new StringBuilder();
        String[] lines = input.split("\n", -1);
        
        for (String line : lines) {
            line = line.replaceAll("<[^>]*>", "");
            line = line.trim();
            
            if (!line.isEmpty()) {
                result.append(line).append("\n");
            }
        }
        
        return result.toString();
    }
}
