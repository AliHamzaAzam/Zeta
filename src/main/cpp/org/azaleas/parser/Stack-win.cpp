
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
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

typedef struct {
    char non_terminal[MAX_SYMBOL_LEN];
    char terminal[MAX_SYMBOL_LEN];
    char production[MAX_PROD_LEN];
} ParsingTableEntry;

ParsingTableEntry parsing_table[MAX_TABLE_ENTRIES];
int table_size = 0;
std::vector<std::string> terminals;

typedef struct {
    char items[MAX_STACK_SIZE][MAX_SYMBOL_LEN];
    int top;
} Stack;

void stack_init(Stack* s, const char* start_symbol) {
    s->top = -1;
    strcpy_s(s->items[++s->top], MAX_SYMBOL_LEN, "$");
    strcpy_s(s->items[++s->top], MAX_SYMBOL_LEN, start_symbol);
}

void stack_push(Stack* s, const char* symbol) {
    if (s->top >= MAX_STACK_SIZE - 1) {
        fprintf(stderr, "Stack overflow!\n");
        exit(EXIT_FAILURE);
    }
    strcpy_s(s->items[++s->top], MAX_SYMBOL_LEN, symbol);
}

char* stack_pop(Stack* s) {
    if (s->top < 0) {
        fprintf(stderr, "Stack underflow!\n");
        exit(EXIT_FAILURE);
    }
    return s->items[s->top--];
}

char* stack_peek(Stack* s) {
    return (s->top >= 0) ? s->items[s->top] : NULL;
}

void load_parsing_table(const char* filename) {
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
            segment.erase(0, segment.find_first_not_of(" \t\n\r\f\v"));
            segment.erase(segment.find_last_not_of(" \t\n\r\f\v") + 1);
            segments.push_back(segment);
        }

        if (segments.empty()) continue;

        if (!header_read) {
            for (size_t i = 1; i < segments.size(); ++i) {
                terminals.push_back(segments[i]);
            }
            header_read = true;
        }
        else {
            if (segments.size() < 1) continue;
            std::string non_terminal = segments[0];

            for (size_t i = 1; i < segments.size(); ++i) {
                if (i - 1 < terminals.size() && !segments[i].empty()) {
                    if (table_size >= MAX_TABLE_ENTRIES) {
                        fprintf(stderr, "Parsing table size exceeded\n");
                        exit(EXIT_FAILURE);
                    }

                    std::string production_full = segments[i];
                    std::string production_rhs;

                    size_t arrow_pos = production_full.find("->");
                    if (arrow_pos != std::string::npos) {
                        if (arrow_pos + 2 < production_full.length()) {
                            production_rhs = production_full.substr(arrow_pos + 2);
                        }
                        else {
                            production_rhs = "#";
                        }
                    }
                    else if (production_full == "#") {
                        production_rhs = "#";
                    }
                    else {
                        production_rhs = production_full;
                    }

                    production_rhs.erase(0, production_rhs.find_first_not_of(" \t\n\r\f\v"));
                    production_rhs.erase(production_rhs.find_last_not_of(" \t\n\r\f\v") + 1);

                    if (production_rhs == "#") {
                        production_rhs.clear();
                    }

                    if (production_rhs.empty() && production_full != "#") {
                        production_rhs = "#";
                        production_rhs.erase(0, production_rhs.find_first_not_of(" \t\n\r\f\v"));
                        production_rhs.erase(production_rhs.find_last_not_of(" \t\n\r\f\v") + 1);
                        if (production_rhs == "#") {
                            production_rhs.clear();
                        }
                    }

                    if (production_rhs.length() >= MAX_PROD_LEN) {
                        fprintf(stderr, "Production RHS too long\n");
                        exit(EXIT_FAILURE);
                    }

                    strcpy_s(parsing_table[table_size].non_terminal, MAX_SYMBOL_LEN, non_terminal.c_str());
                    strcpy_s(parsing_table[table_size].terminal, MAX_SYMBOL_LEN, terminals[i - 1].c_str());
                    strcpy_s(parsing_table[table_size].production, MAX_PROD_LEN, production_rhs.c_str());
                    table_size++;
                }
            }
        }
    }

    if (!header_read) {
        fprintf(stderr, "Error reading header\n");
        exit(EXIT_FAILURE);
    }
}

const char* get_production(const char* nt, const char* term) {
    for (int i = 0; i < table_size; i++) {
        if (strcmp(parsing_table[i].non_terminal, nt) == 0 &&
            strcmp(parsing_table[i].terminal, term) == 0) {
            return parsing_table[i].production;
        }
    }
    return NULL;
}

int parse_input(const char* input, const char* start_symbol, int line_number, std::vector<std::string>& errors) {
    Stack s;
    stack_init(&s, start_symbol);
    char input_copy[MAX_INPUT_LEN];
    strcpy_s(input_copy, MAX_INPUT_LEN, input);
    char* next_token = NULL;
    char* token = strtok_s(input_copy, " ", &next_token);
    int step = 1;
    bool error = false;
    int error_count = 0;

    printf("\nParsing Line %d: %s\n", line_number, input);
    printf("-------------------------------\n");

    while (true) {
        char* top = stack_peek(&s);
        if (!top) break;

        printf("Step %d:\n", step++);
        printf("Stack: ");
        for (int i = s.top; i >= 0; i--) {
            printf("%s ", s.items[i]);
        }

        const char* current_input = token ? token : "$";
        printf("\nInput: %s\n", current_input);

        if (strcmp(top, current_input) == 0) {
            if (strcmp(top, "$") == 0) {
                printf("Action: Accept\n");
                break;
            }
            else {
                printf("Action: Match '%s'\n", token);
                stack_pop(&s);
                token = strtok_s(NULL, " ", &next_token);
            }
        }
        else {
            const char* prod = get_production(top, current_input);
            if (!prod) {
                char error_msg[256];
                sprintf_s(error_msg, "Line %d: Syntax Error: Unexpected token '%s'", line_number, current_input);
                errors.push_back(error_msg);
                error_count++;
                error = true;
                break;
            }
            if (strlen(prod) == 0) {
                printf("Action: Expand %s -> #\n", top);
            }
            else {
                printf("Action: Expand %s -> %s\n", top, prod);
            }
            stack_pop(&s);

            if (strlen(prod) > 0) {
                std::string prod_str(prod);
                std::stringstream prod_ss(prod_str);
                std::string part;
                std::vector<std::string> parts;

                while (prod_ss >> part) {
                    parts.push_back(part);
                }

                for (int i = static_cast<int>(parts.size()) - 1; i >= 0; i--) {
                    if (parts[i].length() >= MAX_SYMBOL_LEN) {
                        char error_msg[256];
                        sprintf_s(error_msg, "Line %d: Error: Symbol '%s' too long", line_number, parts[i].c_str());
                        errors.push_back(error_msg);
                        error_count++;
                        error = true;
                        break;
                    }
                    stack_push(&s, parts[i].c_str());
                }
                if (error) break;
            }
        }
        printf("\n");
    }

    if (!error && (token != NULL || strcmp(stack_peek(&s), "$") != 0)) {
        char error_msg[256];
        if (token) {
            sprintf_s(error_msg, "Line %d: Syntax Error: Unexpected token '%s' at end", line_number, token);
        }
        else {
            sprintf_s(error_msg, "Line %d: Syntax Error: Incomplete expression", line_number);
        }
        errors.push_back(error_msg);
        error_count++;
    }

    printf(error_count ? "Parsing failed\n" : "Parsing succeeded\n");
    printf("-------------------------------\n");
    return error_count;
}

int main() {
    load_parsing_table("ll1_parsing_table1.csv");
    FILE* input_file = NULL;
    errno_t err = fopen_s(&input_file, "input_strings1.txt", "r");
    if (err != 0 || !input_file) {
        perror("Error opening input file");
        return EXIT_FAILURE;
    }

    char line[MAX_INPUT_LEN];
    int line_number = 0;
    std::vector<std::string> errors;
    int total_errors = 0;

    while (fgets(line, sizeof(line), input_file)) {
        line_number++;
        line[strcspn(line, "\n")] = '\0';
        if (strlen(line) > 0) {
            total_errors += parse_input(line, "P", line_number, errors);
        }
    }

    fclose(input_file);

    // Print all errors
    if (!errors.empty()) {
        printf("\nParsing Errors:\n");
        for (const auto& error : errors) {
            printf("%s\n", error.c_str());
        }
    }

    // Final summary
    if (total_errors == 0) {
        printf("\nParsing completed with no errors.\n");
    }
    else {
        printf("\nParsing completed with %d error%s.\n", total_errors, (total_errors == 1) ? "" : "s");
    }

    return EXIT_SUCCESS;
}