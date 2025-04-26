//
// Created by Ali Hamza Azam on 25/04/2025.
//
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>

#define MAX_STACK_SIZE 100
#define MAX_INPUT_LEN 1000
#define MAX_PROD_LEN 100
#define MAX_SYMBOL_LEN 20
#define MAX_TABLE_ENTRIES 100

// Data structure for parsing table entries
typedef struct {
    char non_terminal[MAX_SYMBOL_LEN];
    char terminal[MAX_SYMBOL_LEN];
    char production[MAX_PROD_LEN];
} ParsingTableEntry;

ParsingTableEntry parsing_table[MAX_TABLE_ENTRIES];
int table_size = 0;
std::vector<std::string> terminals; // Store terminals from header

// Stack structure
typedef struct {
    char items[MAX_STACK_SIZE][MAX_SYMBOL_LEN];
    int top;
} Stack;

// Initialize stack with start symbol and $
void stack_init(Stack *s, const char *start_symbol) {
    s->top = -1;
    strcpy(s->items[++s->top], "$");
    strcpy(s->items[++s->top], start_symbol);
}

// Push a symbol onto the stack
void stack_push(Stack *s, const char *symbol) {
    if (s->top >= MAX_STACK_SIZE - 1) {
        fprintf(stderr, "Stack overflow!\n");
        exit(EXIT_FAILURE);
    }
    strcpy(s->items[++s->top], symbol);
}

// Pop a symbol from the stack
char* stack_pop(Stack *s) {
    if (s->top < 0) {
        fprintf(stderr, "Stack underflow!\n");
        exit(EXIT_FAILURE);
    }
    return s->items[s->top--];
}

// Peek at the top of the stack
char* stack_peek(Stack *s) {
    return (s->top >= 0) ? s->items[s->top] : nullptr;
}

// Load parsing table from a CSV file
void load_parsing_table(const char *filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        perror("Error opening parsing table file");
        exit(EXIT_FAILURE);
    }

    std::string line;
    bool header_read = false;

    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string segment;
        std::vector<std::string> segments;

        while (std::getline(ss, segment, ',')) {
            // Trim leading/trailing whitespace if necessary (basic trim)
            segment.erase(0, segment.find_first_not_of(" \t\n\r\f\v"));
            segment.erase(segment.find_last_not_of(" \t\n\r\f\v") + 1);
            segments.push_back(segment);
        }

        if (segments.empty()) continue; // Skip empty lines

        if (!header_read) {
            // Read header: first segment is "Non-Terminal", skip it
            for (size_t i = 1; i < segments.size(); ++i) {
                terminals.push_back(segments[i]);
            }
            header_read = true;
        } else {
            // Read data row
            if (segments.empty()) continue; // Malformed row
            const std::string& non_terminal = segments[0];

            for (size_t i = 1; i < segments.size(); ++i) {
                if (i - 1 < terminals.size() && !segments[i].empty()) {
                    if (table_size >= MAX_TABLE_ENTRIES) {
                        fprintf(stderr, "Parsing table size exceeded MAX_TABLE_ENTRIES\n");
                        exit(EXIT_FAILURE);
                    }

                    const std::string& production_full = segments[i]; // e.g., " E → T E'"
                    std::string production_rhs;

                    // Find the arrow '→' or '->'
                    size_t arrow_pos = production_full.find("→");
                    std::string arrow_str = "→";
                    size_t arrow_len = std::string(arrow_str).length(); // Get length of arrow string

                    if (arrow_pos == std::string::npos) {
                         arrow_pos = production_full.find("->");
                         arrow_str = "->";
                         arrow_len = std::string(arrow_str).length(); // Get length of arrow string
                    }

                    if (arrow_pos != std::string::npos) {
                        // Extract substring AFTER the arrow
                        if (arrow_pos + arrow_len < production_full.length()) {
                            production_rhs = production_full.substr(arrow_pos + arrow_len);
                        } else {
                            // Arrow is at the very end, means epsilon
                            production_rhs = "ε";
                        }
                    } else if (production_full == "ε") {
                        production_rhs = "ε"; // Handle epsilon explicitly if no arrow
                    } else {
                         // No arrow found, and not epsilon. Assume the segment IS the RHS.
                         production_rhs = production_full;
                         // Optional: Add warning if format strictly requires an arrow
                         // fprintf(stderr, "Warning: No arrow found in production '%s'. Assuming it's the RHS.\n", production_full.c_str());
                    }

                    // Trim leading/trailing whitespace from the extracted RHS
                    production_rhs.erase(0, production_rhs.find_first_not_of(" \t\n\r\f\v"));
                    production_rhs.erase(production_rhs.find_last_not_of(" \t\n\r\f\v") + 1);

                    // Ensure RHS is not empty after trimming, default to epsilon if it is
                    // (unless the original segment was already epsilon)
                    if (production_rhs.empty() && production_full != "ε") {
                         production_rhs = "ε";
                    }

                    // Store the cleaned RHS
                    if (production_rhs.length() >= MAX_PROD_LEN) {
                         fprintf(stderr, "Error: Production RHS '%s' too long (max %d)\n", production_rhs.c_str(), MAX_PROD_LEN - 1);
                         exit(EXIT_FAILURE);
                    }
                    if (strlen(non_terminal.c_str()) >= MAX_SYMBOL_LEN) {
                         fprintf(stderr, "Error: Non-terminal '%s' too long (max %d)\n", non_terminal.c_str(), MAX_SYMBOL_LEN - 1);
                         exit(EXIT_FAILURE);
                    }
                     if (terminals[i - 1].length() >= MAX_SYMBOL_LEN) {
                         fprintf(stderr, "Error: Terminal '%s' too long (max %d)\n", terminals[i - 1].c_str(), MAX_SYMBOL_LEN - 1);
                         exit(EXIT_FAILURE);
                    }

                    strcpy(parsing_table[table_size].non_terminal, non_terminal.c_str());
                    strcpy(parsing_table[table_size].terminal, terminals[i - 1].c_str());
                    strcpy(parsing_table[table_size].production, production_rhs.c_str());
                    table_size++;
                }
            }
        }
    }

    if (!header_read) {
         fprintf(stderr, "Error: Could not read header from parsing table file.\n");
         exit(EXIT_FAILURE);
    }
    if (table_size == 0) {
        fprintf(stderr, "Warning: No entries loaded from parsing table.\n");
    }
}

// Get production for a non-terminal and terminal
const char* get_production(const char *nt, const char *term) {
    for (int i = 0; i < table_size; i++) {
        if (strcmp(parsing_table[i].non_terminal, nt) == 0 &&
            strcmp(parsing_table[i].terminal, term) == 0) {
            return parsing_table[i].production;
        }
    }
    return nullptr; // No entry found (error)
}

// Parse a single input string and add errors to the vector
void parse_input(const char *input, const char *start_symbol, int line_num, std::vector<std::string>& errors) {
    Stack s;
    stack_init(&s, start_symbol);
    char input_copy[MAX_INPUT_LEN];
    strncpy(input_copy, input, MAX_INPUT_LEN - 1); // Use strncpy for safety
    input_copy[MAX_INPUT_LEN - 1] = '\0'; // Ensure null termination

    char* next_token = nullptr;
    char* token = strtok_r(input_copy, " ", &next_token); // Initial tokenization
    int step = 1;
    bool error_on_line = false;

    printf("\nParsing Line %d: '%s'\n", line_num, input);
    printf("-------------------------------\n");

    while (stack_peek(&s) != nullptr && !error_on_line) { // Stop processing line on first error
        // Print current stack and input
        printf("Step %d:\n", step++);
        printf("Stack: ");
        for (int i = s.top; i >= 0; i--) {
            printf("%s ", s.items[i]);
        }
        // Determine current input symbol (use $ if token is NULL)
        const char *current_input = token ? token : "$";
        printf("\nInput: %s\n", current_input);

        char *top = stack_peek(&s);

        // Check for terminal match or end of input
        if (strcmp(top, current_input) == 0) {
            if (strcmp(top, "$") == 0) { // Both stack top and input are $
                printf("Action: Accept\n");
                break; // Successful parse for this line
            } else { // Matched a terminal
                printf("Action: Match '%s'\n", token);
                stack_pop(&s);
                token = strtok_r(nullptr, " ", &next_token); // Get next token using strtok_r
            }
        } else { // Top is a non-terminal, need to expand
            const char *prod = get_production(top, current_input);
            if (!prod) {
                // *** ERROR HANDLING: No production found ***
                std::string error_msg = "Line " + std::to_string(line_num) +
                                        ": Syntax Error: Unexpected token '" + current_input +
                                        "' when expecting production for " + top;
                // More specific error (requires grammar knowledge):
                errors.push_back(error_msg);
                error_on_line = true;
                printf("Error: %s\n", error_msg.c_str()); // Print immediate error context
                break; // Stop parsing this line
            }
            printf("Action: Expand %s -> %s\n", top, prod);
            stack_pop(&s);

            if (strcmp(prod, "ε") != 0) { // Don't push anything for epsilon
                std::string prod_str(prod);
                std::stringstream prod_ss(prod_str);
                std::string prod_part_str;
                std::vector<std::string> parts; // Use vector to store parts

                // Split the production RHS using stringstream
                while (prod_ss >> prod_part_str) {
                    parts.push_back(prod_part_str);
                }

                // Push parts onto stack in reverse order
                for (int i = parts.size() - 1; i >= 0; i--) {
                    if (parts[i].length() >= MAX_SYMBOL_LEN) {
                        fprintf(stderr, "Error: Symbol '%s' in production '%s' too long (max %d)\n", parts[i].c_str(), prod, MAX_SYMBOL_LEN - 1);
                        // This is a configuration error, maybe exit? For now, report and stop line.
                        std::string error_msg = "Line " + std::to_string(line_num) +
                                                ": Internal Error: Symbol '" + parts[i] + "' too long.";
                        errors.push_back(error_msg);
                        error_on_line = true;
                        break; // Break inner loop
                    }
                    stack_push(&s, parts[i].c_str());
                }
                if (error_on_line) break; // Break outer loop if symbol was too long
            }
        }
        printf("\n"); // Add newline for better formatting
    }

    // Final check after loop (only if no error occurred during parsing steps)
    if (!error_on_line) {
        const char* final_stack_top = stack_peek(&s);
        if (final_stack_top && strcmp(final_stack_top, "$") == 0 && token != nullptr) {
            // If stack is accepted ($) but there's still input left
            std::string error_msg = "Line " + std::to_string(line_num) +
                                    ": Syntax Error: Unexpected token '" + token + "' after end of expression";
            errors.push_back(error_msg);
            error_on_line = true;
            printf("Error: %s\n", error_msg.c_str());
        } else if (!(final_stack_top && strcmp(final_stack_top, "$") == 0) && token == nullptr) {
            // If input is exhausted (token is NULL) but stack isn't accepted ($)
            std::string error_msg = "Line " + std::to_string(line_num) +
                                    ": Syntax Error: Unexpected end of input, expected more tokens" +
                                    (final_stack_top ? " (Stack top: " + std::string(final_stack_top) + ")" : "");
            // More specific: ": Syntax Error: Expected <token(s)> before end of input"
            errors.push_back(error_msg);
            error_on_line = true;
            printf("Error: %s\n", error_msg.c_str());
        } else if (!(final_stack_top && strcmp(final_stack_top, "$") == 0 && token == nullptr)) {
            // Catch unexpected end states (should ideally not happen if logic above is correct)
            std::string error_msg = "Line " + std::to_string(line_num) +
                                    ": Parsing finished in an unexpected state.";
            if (token != nullptr) error_msg += " Remaining input: " + std::string(token);
            if (final_stack_top) error_msg += " Final stack top: " + std::string(final_stack_top);
            errors.push_back(error_msg);
            error_on_line = true;
            printf("Error: %s\n", error_msg.c_str());
        }
    }

    if (error_on_line) {
        printf("\nParsing failed for Line %d.\n", line_num);
    } else if (stack_peek(&s) && strcmp(stack_peek(&s), "$") == 0 && token == nullptr) {
        printf("\nParsing succeeded for Line %d.\n", line_num);
    }
    printf("-------------------------------\n");
}

int main() {
    // Use the CSV file generated by Parser.cpp (adjust path if needed)
    // Ensure the path is correct relative to where the executable runs
    // Using a direct path relative to the executable location is often safer.
    load_parsing_table("ll1_parsing_table.csv");

    // Same path consideration for input file
    FILE *input_file = fopen("input_strings.txt", "r");
    if (!input_file) {
        perror("Error opening input file (input_strings.txt)");
        input_file = fopen("../input_strings.txt", "r");
        if (!input_file) {
            perror("Error opening input file (../input_strings.txt)");
            return EXIT_FAILURE;
        }
    }

    std::vector<std::string> all_errors; // Vector to store all errors
    char line[MAX_INPUT_LEN];
    int line_num = 1; // Track line numbers

    while (fgets(line, sizeof(line), input_file)) {
        line[strcspn(line, "\n\r")] = '\0'; // Remove newline/carriage return
        if (strlen(line) > 0) {
            // Pass the vector by reference to collect errors
            parse_input(line, "E", line_num, all_errors); // Assuming start symbol is 'E'
        }
        line_num++; // Increment line number for the next line
    }

    fclose(input_file);

    // Print all collected errors at the end
    printf("\n--- Error Summary ---\n");
    if (all_errors.empty()) {
        printf("Parsing completed with 0 errors.\n");
    } else {
        for (const auto& err : all_errors) {
            printf("%s\n", err.c_str());
        }
        // printf("Parsing continued after error recovery.\n");
        printf("Parsing completed with %zu errors.\n", all_errors.size());
    }
    printf("---------------------\n");


    return all_errors.empty() ? EXIT_SUCCESS : EXIT_FAILURE;
}