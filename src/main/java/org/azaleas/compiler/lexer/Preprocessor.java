package org.azaleas.compiler.lexer;

public class Preprocessor {
    public String process(String input) {
        // Strip multi-line block comments first. These may span several lines,
        // so (?s) lets '.' match newlines. Replace with a space to keep the
        // surrounding tokens separated. Done before single-line stripping so the
        // single-line pattern doesn't chop a block comment at its first '>'.
        input = input.replaceAll("(?s)<<.*?>>", " ");

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
