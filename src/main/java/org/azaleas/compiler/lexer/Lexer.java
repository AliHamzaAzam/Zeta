package org.azaleas.compiler.lexer;
import org.azaleas.compiler.errors.ErrorHandler;

import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import java.util.ArrayList;

public class Lexer {
    private static final Map<TokenType, String> PATTERNS = new HashMap<>();
    private static final Map<TokenType, Integer> TOKEN_PRIORITIES = new HashMap<>();
    private int position = 0;
    private String input;
    private List<Token> tokens;
    private ErrorHandler errorHandler;

    static {
        PATTERNS.put(TokenType.WHITESPACE, "[ \\t\\n\\r\\f]+");
        PATTERNS.put(TokenType.MULTI_LINE_COMMENT, "(?s)<<.*?>>");
        PATTERNS.put(TokenType.SINGLE_LINE_COMMENT, "<[^>]*>");
        PATTERNS.put(TokenType.STRING_OR_CHAR, "\\{([^{}]*)\\}");
        PATTERNS.put(TokenType.KEYWORD, "(global|local|tell|ask|is|now|true|false)(?![a-z0-9])");
        PATTERNS.put(TokenType.EXPONENT, "\\^");
        PATTERNS.put(TokenType.OPERATOR, "[+\\-*/%]");
        PATTERNS.put(TokenType.DECIMAL, "[+-]?(\\d+\\.\\d{1,5}|\\.\\d{1,5})([eE][+-]?\\d+)?");
        PATTERNS.put(TokenType.INTEGER, "[+-]?\\d+");
        PATTERNS.put(TokenType.IDENTIFIER, "[a-z][a-z0-9]*");
        PATTERNS.put(TokenType.LEFT_PAREN, "\\(");
        PATTERNS.put(TokenType.RIGHT_PAREN, "\\)");

        TOKEN_PRIORITIES.put(TokenType.WHITESPACE, 0);
        // Multi-line must outrank single-line: both can match at '<<', and the
        // single-line pattern would otherwise stop at the first '>' inside a block.
        TOKEN_PRIORITIES.put(TokenType.MULTI_LINE_COMMENT, 2);
        TOKEN_PRIORITIES.put(TokenType.SINGLE_LINE_COMMENT, 1);
        TOKEN_PRIORITIES.put(TokenType.STRING_OR_CHAR, 2);
        TOKEN_PRIORITIES.put(TokenType.KEYWORD, 3);
        TOKEN_PRIORITIES.put(TokenType.EXPONENT, 2);
        TOKEN_PRIORITIES.put(TokenType.OPERATOR, 2);
        TOKEN_PRIORITIES.put(TokenType.DECIMAL, 3);
        TOKEN_PRIORITIES.put(TokenType.INTEGER, 2);
        TOKEN_PRIORITIES.put(TokenType.IDENTIFIER, 1);
        TOKEN_PRIORITIES.put(TokenType.LEFT_PAREN, 2);
        TOKEN_PRIORITIES.put(TokenType.RIGHT_PAREN, 2);
    }

    public Lexer(ErrorHandler errorHandler) {
        this.tokens = new ArrayList<>();
        this.errorHandler = errorHandler;
    }

    public Lexer() {
        this.tokens = new ArrayList<>();
    }

    public List<Token> tokenize(String input) {
        this.input = input;
        this.position = 0;
        this.tokens.clear();
        int line = 1;
        int column = 1;

        while (position < input.length()) {
            boolean matched = false;
            Token bestMatch = null;
            int bestPriority = -1;

            for (Map.Entry<TokenType, String> entry : PATTERNS.entrySet()) {
                Pattern pattern = Pattern.compile("^" + entry.getValue());
                Matcher matcher = pattern.matcher(input.substring(position));

                if (matcher.find()) {
                    String match = matcher.group();
                    int priority = TOKEN_PRIORITIES.get(entry.getKey());

                    if (bestMatch == null || priority > bestPriority) {
                        bestMatch = new Token(entry.getKey(), match, line, column);
                        bestPriority = priority;
                        matched = true;
                    }
                }
            }

            if (!matched) {
                char invalidChar = input.charAt(position);
                errorHandler.addError(line, "Unexpected token '" + invalidChar + "' at position " + position);
                position++;
                column++;
                continue;
            }

            TokenType matchedType = bestMatch.type();
            boolean isTrivia = matchedType == TokenType.WHITESPACE
                    || matchedType == TokenType.SINGLE_LINE_COMMENT
                    || matchedType == TokenType.MULTI_LINE_COMMENT;

            if (isTrivia) {
                // Whitespace and comments are discarded and never enter the token
                // stream. Walk their characters so line/column stay accurate even
                // when a block comment spans multiple lines.
                for (char c : bestMatch.value().toCharArray()) {
                    if (c == '\n') {
                        line++;
                        column = 1;
                    } else {
                        column++;
                    }
                }
            } else {
                tokens.add(bestMatch);
                column += bestMatch.value().length();
            }

            position += bestMatch.value().length();
        }

        tokens.add(new Token(TokenType.EOF, "", line, column));
        return tokens;
    }
}
