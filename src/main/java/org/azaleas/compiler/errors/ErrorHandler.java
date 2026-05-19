package org.azaleas.compiler.errors;
import java.util.*;

public class ErrorHandler {
    private List<String> errors = new ArrayList<>();

    public void addError(int line, String message) {
        errors.add("Line " + line + ": " + message);
    }

    public void printErrors() {
        if (errors.isEmpty()) {
            System.out.println("No errors detected.");
        } else {
            errors.forEach(System.out::println);
        }
    }

    public boolean hasErrors() {
        return !errors.isEmpty();
    }

    public List<String> getErrors() {
        return new ArrayList<>(errors);
    }
}