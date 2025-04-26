#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <iomanip>
#include <string>


using namespace std;

// Helper function to tokenize a production string into symbols
vector<string> tokenizeProduction(const string& prod) {
    vector<string> tokens;
    // Trim leading/trailing whitespace before tokenizing
    string trimmedProd = prod;
    size_t first = trimmedProd.find_first_not_of(" \t");
    if (string::npos == first) return tokens; // Empty or whitespace only
    size_t last = trimmedProd.find_last_not_of(" \t");
    trimmedProd = trimmedProd.substr(first, (last - first + 1));

    istringstream iss(trimmedProd);
    string token;
    while (iss >> token) {
        tokens.push_back(token);
    }
    return tokens;
}

// Helper function to join tokens back into a string
string joinTokens(const vector<string>& tokens, size_t start = 0, size_t end = string::npos) {
    string result;
    if (end == string::npos) {
        end = tokens.size();
    }
    for (size_t i = start; i < end; ++i) {
        if (i > start) {
            result += " ";
        }
        result += tokens[i];
    }
    return result;
}


// class to assist in console output redirection to file
class TeeBuf : public streambuf {
public:
    // initializes two streaming buffers
    TeeBuf(streambuf* sb1, streambuf* sb2) : sb1_(sb1), sb2_(sb2) {}

protected:

    // write the character c to both output streams
    int overflow(int c) override {
        if (c == EOF) {
            return !EOF;
        }
        else {
            int r1 = sb1_->sputc(c);
            int r2 = sb2_->sputc(c);
            return (r1 == EOF || r2 == EOF) ? EOF : c;
        }
    }

    // flush both output streams
    int sync() override {
        int r1 = sb1_->pubsync();
        int r2 = sb2_->pubsync();
        return (r1 == 0 && r2 == 0) ? 0 : -1;
    }

private:
    streambuf* sb1_;
    streambuf* sb2_;
};

// Class to store and process Context-Free Grammar (CFG)
class Grammar {
    // Map to store the original cfg
    map<string, vector<string>> cfg;

    // Maps to store first and follow sets
    map<string, set<string>> first;
    map<string, set<string>> follow;

    // Sets of terminals and non-terminals
    set<string> nonTerminals;
    set<string> terminals;

    // map to hold the parsing table
    map<pair<string, string>, string> parsingTable;

    // Start Symbol
    string startSymbol = "S"; // Default start symbol, can be changed


public:
    // Default constructor
    Grammar() = default;

    // Function to read grammar from a file
    int readGrammar(const string& fileName) {
        string line;

        // File opening validation
        ifstream file(fileName);
        if (!file) {
            cerr << "Error: Could not open file." << endl;
            return 0;
        }

        // Read each line of the file
        bool isFirst = true;
        while (getline(file, line)) {
            // Read one line of the cfg (a production rule) into a string stream
            istringstream iss(line);
            string lhs, arrow, rhs;

            // Extract non-terminal on the Left hand side into lhs, and the arrow "->" into arrow
            iss >> lhs >> arrow;

            // Production format validation
            if (arrow != "->") {
                cerr << "Error: Invalid production format." << endl;
                return 0;
            }

            // Extract right-hand side (actual productions) into rhs
            getline(iss, rhs);

            // Read the rhs of the cfg into a string stream
            istringstream rhsStream(rhs);
            string production;

            if (isFirst) {
                startSymbol = lhs;
                isFirst = false;
            }

            // Split productions by '|' and store them in the map
            while (getline(rhsStream, production, '|')) { cfg[lhs].push_back(production); }

        }

        file.close();
        return 1;
    }

    // Function to print the CFG
    void printGrammar() {
        for (const auto& rule : cfg) {
            // Print non-terminal of rule
            cout << rule.first << " -> ";
            for (size_t i = 0; i < rule.second.size(); ++i) {
                // Print right-hand side productions
                cout << rule.second[i];
                // Separate multiple productions
                if (i < rule.second.size() - 1) cout << " | ";
            }
            cout << endl;
        }
    }

    // Function that applies left factoring to the CFG
    int leftFactoring() {
        bool changed = true;
        // counter for new unique non-terminals
        int newSymbolCount = 0;

        // Iterate until no changes are made in an iteration
        while (changed) {
            changed = false;
            map<string, vector<string>> new_cfg;

            // Iterate over the CFG
            for (auto const& [lhs, productions] : cfg) { // Use structured binding
                vector<string> currentProductions = productions; // Work on a copy

                // Continue factoring until no more changes can be made for this non-terminal
                bool localChanged = true;
                while (localChanged && currentProductions.size() > 1) {
                    localChanged = false;

                    // finding longest common prefix among the productions (TOKEN BASED)
                    for (size_t i = 0; i < currentProductions.size(); ++i) {
                        vector<string> tokens_i = tokenizeProduction(currentProductions[i]);
                        if (tokens_i.empty()) continue; // Skip empty productions

                        vector<size_t> commonGroupIndices; // Indices of productions sharing the longest prefix with prod i
                        vector<string> longestPrefixTokens; // The longest common token prefix found so far for prod i

                        // Compare current production (i) with subsequent productions (j)
                        for (size_t j = i + 1; j < currentProductions.size(); ++j) {
                            vector<string> tokens_j = tokenizeProduction(currentProductions[j]);
                            if (tokens_j.empty()) continue;

                            size_t k = 0; // Length of current common token prefix
                            while (k < tokens_i.size() && k < tokens_j.size() && tokens_i[k] == tokens_j[k]) {
                                k++;
                            }

                            // Check if this common prefix is longer than the current longestPrefixTokens
                            if (k > 0 && k >= longestPrefixTokens.size()) {
                                vector<string> currentPrefixTokens(tokens_i.begin(), tokens_i.begin() + k);

                                // If strictly longer, reset the group
                                if (k > longestPrefixTokens.size()) {
                                    longestPrefixTokens = currentPrefixTokens;
                                    commonGroupIndices.clear();
                                    commonGroupIndices.push_back(j); // Add j to the new group
                                }
                                // If equal length, add j to the existing group
                                else if (k == longestPrefixTokens.size()) {
                                     // Only add if the prefix actually matches the current longest
                                    bool prefixMatches = true;
                                    for(size_t p=0; p<k; ++p) {
                                        if (tokens_i[p] != longestPrefixTokens[p]) {
                                            prefixMatches = false;
                                            break;
                                        }
                                    }
                                    if (prefixMatches) {
                                        commonGroupIndices.push_back(j);
                                    }
                                }
                            }
                        } // End comparison loop (j)

                        // If a common prefix was found for production i and at least one other production
                        if (!longestPrefixTokens.empty() && !commonGroupIndices.empty()) {
                            localChanged = changed = true; // Mark that changes were made

                            string newNonTerminal = lhs + "_" + to_string(++newSymbolCount);
                            string prefixStr = joinTokens(longestPrefixTokens);

                            vector<string> newNonTerminalProductions; // Productions for the new non-terminal S'
                            vector<string> remainingProductions; // Productions that didn't share the prefix

                            // Process the original production 'i' which started the group
                            vector<string> suffix_i_tokens(tokens_i.begin() + longestPrefixTokens.size(), tokens_i.end());
                            string suffix_i_str = joinTokens(suffix_i_tokens);
                            if (suffix_i_str.empty()) suffix_i_str = "ε"; // Use epsilon if suffix is empty
                            newNonTerminalProductions.push_back(suffix_i_str);

                            // Keep track of indices processed in this factoring step
                            set<size_t> processedIndices;
                            processedIndices.insert(i);
                            for (size_t idx : commonGroupIndices) {
                                processedIndices.insert(idx);
                            }

                            // Process productions in the common group (found in commonGroupIndices)
                            for (size_t idx : commonGroupIndices) {
                                vector<string> tokens_idx = tokenizeProduction(currentProductions[idx]);
                                vector<string> suffix_idx_tokens(tokens_idx.begin() + longestPrefixTokens.size(), tokens_idx.end());
                                string suffix_idx_str = joinTokens(suffix_idx_tokens);
                                if (suffix_idx_str.empty()) suffix_idx_str = "ε";
                                newNonTerminalProductions.push_back(suffix_idx_str);
                            }

                            // Add the new factored production for the original non-terminal
                            remainingProductions.push_back(prefixStr + " " + newNonTerminal);

                            // Add back any productions that were not part of this factoring group
                            for (size_t k = 0; k < currentProductions.size(); ++k) {
                                if (processedIndices.find(k) == processedIndices.end()) {
                                    remainingProductions.push_back(currentProductions[k]);
                                }
                            }

                            // Update the productions for the current non-terminal
                            currentProductions = remainingProductions;
                            // Add the new rule for the newly created non-terminal
                            new_cfg[newNonTerminal] = newNonTerminalProductions;

                            // Restart the check for the current non-terminal since its productions changed
                            goto next_iteration_for_lhs; // Use goto for clarity in restarting the outer loop check
                        }
                    } // End production loop (i)

                    next_iteration_for_lhs:; // Label for restarting the check for the current lhs
                } // End while(localChanged)

                // Add the final set of productions for this non-terminal to the new grammar
                // Only add if it wasn't added during factoring (e.g., new non-terminals)
                 if (new_cfg.find(lhs) == new_cfg.end()) {
                    new_cfg[lhs] = currentProductions;
                 } else {
                    // If lhs was already added (e.g. as a new non-terminal name), merge productions carefully
                    // This case should ideally not happen with unique naming, but handle defensively
                    vector<string>& existingProds = new_cfg[lhs];
                    existingProds.insert(existingProds.end(), currentProductions.begin(), currentProductions.end());
                 }


            } // End CFG iteration

            // replace old cfg with new left factored updated cfg
            cfg = new_cfg;
        } // End while(changed)

        // success
        return 1;
    }

    // Function to remove left recursion in productions
    int leftRecursion() {
        // map to store updated cfg
        map<string, vector<string>> new_cfg;

        // Iterate over the CFG
        for (const auto& rule : cfg) {
            string lhs = rule.first;
            vector<string> productions = rule.second;

            // vectors to separate valid and left recursive (invalid) productions
            vector<string> leftRecursiveProds;
            vector<string> nonLeftRecursiveProds;

            // iterate over each production in the cfg
            for (const string& prod : productions) {

                // check if the production starts with the same non-terminal
                istringstream iss(prod);
                string firstSymbol;
                iss >> firstSymbol;

                if (firstSymbol == lhs) {
                    // store the part after the recursive symbol
                    string alpha;
                    getline(iss, alpha);
                    // add the invalid production to LR productions vector
                    leftRecursiveProds.push_back(alpha);
                }
                else {
                    // add valid producion to valid set
                    nonLeftRecursiveProds.push_back(prod);
                }
            }

            // If there are no left-recursive productions, keep the original rule
            if (leftRecursiveProds.empty()) {
                new_cfg[lhs] = productions;
                continue;
            }

            // Create a new non-terminal for the LR non-terminal
            string newNonTerminal = lhs + "'";

            // Create new productions
            vector<string> newLhsProds;
            vector<string> newNonTerminalProds;

            // If there are no valid productions, add epsilon to avoid empty rule
            if (nonLeftRecursiveProds.empty()) { nonLeftRecursiveProds.emplace_back("ε"); }

            // Create productions for A -> β A'
            for (const string& beta : nonLeftRecursiveProds) {
                string newProd = beta;

                // Don't append the new non-terminal to epsilon
                if (newProd != "ε") { newProd += " " + newNonTerminal; }
                else { newProd = newNonTerminal; }

                newLhsProds.push_back(newProd);
            }

            // Create productions for A' -> α A' | ε
            for (const string& alpha : leftRecursiveProds) {
                string newProd = alpha + " " + newNonTerminal;
                newNonTerminalProds.push_back(newProd);
            }

            // Add epsilon production for A'
            newNonTerminalProds.emplace_back("ε");

            // Update the grammar
            new_cfg[lhs] = newLhsProds;
            new_cfg[newNonTerminal] = newNonTerminalProds;
        }

        // Replace old grammar with new one
        cfg = new_cfg;
        return 1;
    }

    // identify all terminals and non-terminals in the grammar and store them
    void initializeSymbols() {
        nonTerminals.clear();
        terminals.clear();

        // add all LHS symbols to non-terminal set
        for (const auto& rule : cfg) {
            nonTerminals.insert(rule.first);
        }

        // Find terminals
        for (const auto& rule : cfg) {
            for (const string& prod : rule.second) {
                istringstream iss(prod);
                string token;

                while (iss >> token) {
                    // if it is not in non-terminal set and is not epislon, it is a terminal
                    if (token != "ε" && nonTerminals.find(token) == nonTerminals.end()) { terminals.insert(token); }
                }
            }
        }
    }

    // function to make FIRST set for all non-terminals
    int computeFirst() {
        initializeSymbols();

        // Initialize first sets
        first.clear();
        for (const string& nonTerm : nonTerminals) {
            first[nonTerm] = {};
        }
        // Initialize first sets for terminals as well, needed for computeFirstOfProduction
        for (const string& term : terminals) {
            first[term] = {term};
        }
        // Add epsilon explicitly if needed later
        first["ε"] = {"ε"};


        bool changed = true;
        // iterate until no changes in an iteration
        while (changed) {
            changed = false;

            // iterate over the cfg
            for (const auto& rule : cfg) {
                string lhs = rule.first;

                // iterate over each production of a rule
                for (const string& prodStr : rule.second) {
                    vector<string> prod = tokenizeProduction(prodStr); // Tokenize production

                    // Handle direct epsilon production: X -> ε
                    if (prod.empty() || (prod.size() == 1 && prod[0] == "ε")) {
                         if (first[lhs].insert("ε").second) changed = true;
                         continue;
                    }


                    // compute First for this production X -> Y1 Y2 ... Yk
                    // flag to track if all symbols in production can derive ε
                    bool allDeriveEpsilon = true;

                    for (const auto & symbol : prod) {
                        // Ensure the symbol exists in the first map (could be terminal or non-terminal)
                        if (first.find(symbol) == first.end()) {
                             // This case should ideally not happen if initializeSymbols is correct
                             // If it's a terminal not seen before, its first set is itself
                             if (terminals.count(symbol)) {
                                first[symbol] = {symbol};
                             } else {
                                // Or handle as an error / unknown symbol
                                cerr << "Warning: Symbol '" << symbol << "' in production '" << prodStr << "' not found in FIRST sets." << endl;
                                allDeriveEpsilon = false; // Cannot proceed if symbol is unknown
                                break;
                             }
                        }

                        // flag to track if current symbol derives an epsilon
                        bool currDerivedEpsilon = false;

                        // Add all elements from First(symbol) except epsilon to First(lhs)
                        for (const string& elem : first[symbol]) {
                            if (elem == "ε") currDerivedEpsilon = true;
                            else {
                                // mark change if it's a new addition
                                if (first[lhs].insert(elem).second) changed = true;
                            }
                        }

                        // if this symbol cannot derive epsilon, stop processing more symbols
                        if (!currDerivedEpsilon) {
                            allDeriveEpsilon = false;
                            break;
                        }
                    }

                    // If all symbols of the production can derive epsilon, add epsilon to First(lhs)
                    if (allDeriveEpsilon) {
                        if (first[lhs].insert("ε").second) changed = true;
                    }
                }
            }
        }

        return 1;
    }


    // Helper function to compute First of a sequence of symbols (production RHS)
    // Returns true if the sequence can derive epsilon, false otherwise.
    // Populates firstSet with the First set of the sequence.
    bool computeFirstOfSequence(const vector<string>& symbols, set<string>& firstSet) {
        firstSet.clear();
        bool allDeriveEpsilon = true;

        if (symbols.empty() || (symbols.size() == 1 && symbols[0] == "ε")) {
             firstSet.insert("ε");
             return true; // Epsilon sequence derives epsilon
        }


        for (const auto & symbol : symbols) {
            // Ensure the symbol exists in the first map
            if (first.find(symbol) == first.end()) {
                 if (terminals.count(symbol)) {
                    first[symbol] = {symbol}; // Handle terminals on the fly if missed
                 } else {
                    cerr << "Warning: Symbol '" << symbol << "' not found in FIRST sets during FirstOfSequence calculation." << endl;
                    allDeriveEpsilon = false;
                    break;
                 }
            }

            bool currDerivedEpsilon = false;
            for (const string& elem : first[symbol]) {
                if (elem == "ε") {
                    currDerivedEpsilon = true;
                } else {
                    firstSet.insert(elem);
                }
            }

            if (!currDerivedEpsilon) {
                allDeriveEpsilon = false;
                break; // Stop if a symbol doesn't derive epsilon
            }
        }

        // If all symbols derived epsilon, add epsilon to the result set
        if (allDeriveEpsilon) {
            firstSet.insert("ε");
        }

        return allDeriveEpsilon; // Return whether the whole sequence can derive epsilon
    }


    // function to make FOLLOW set for all non-terminals
    int computeFollow() {

        // Ensure FIRST sets are computed
        if (first.empty()) {
            computeFirst();
        }
         // Ensure symbols are initialized
        if (nonTerminals.empty() || terminals.empty()) {
            initializeSymbols();
        }


        // initialize Follow sets
        follow.clear();
        for (const string& nonTerm : nonTerminals) follow[nonTerm] = {};

        // Rule 1: Add $ to Follow of the designated start symbol
        follow[startSymbol].insert("$");
        cout << "Start Symbol: " << startSymbol << endl;
        bool changed = true;
        // iterate until no changes in an iteration
        while (changed) {
            changed = false;

            // Iterate over rules in the CFG: A -> α
            for (const auto& rule : cfg) {
                string lhs_A = rule.first;


                for (const string& prodStr : rule.second) {
                    vector<string> prod = tokenizeProduction(prodStr); // Tokenize production α

                    // Iterate over each symbol B in the production α
                    for (size_t i = 0; i < prod.size(); ++i) {
                        string symbol_B = prod[i];

                        // We only compute Follow for non-terminals
                        if (nonTerminals.find(symbol_B) == nonTerminals.end()) continue;

                        // Consider the part of the production after B: β
                        // Create a vector for β (symbols from i+1 to end)
                        vector<string> beta_sequence;
                        if (i + 1 < prod.size()) {
                            beta_sequence.assign(prod.begin() + i + 1, prod.end());
                        }

                        // Rule 2: A -> α B β
                        // Add First(β) - {ε} to Follow(B)
                        set<string> firstOfBeta;
                        bool betaDerivesEpsilon = computeFirstOfSequence(beta_sequence, firstOfBeta); // Use helper

                        for (const string& term : firstOfBeta) {
                            if (term != "ε") {
                                if (follow[symbol_B].insert(term).second) changed = true;
                            }
                        }

                        // Rule 3: A -> α B or A -> α B β where First(β) contains ε
                        // Add Follow(A) to Follow(B)
                        if (beta_sequence.empty() || betaDerivesEpsilon) {
                            for (const string& term : follow[lhs_A]) {
                                if (follow[symbol_B].insert(term).second) changed = true;
                            }
                        }
                    }
                }
            }
        }

        return 1;
    }

    // function to print First and Follow sets
    void printFirstAndFollow() {
        cout << "\nFirst Sets:" << endl;
        // iterate over all non-terminals and print items of each non-terminal's first set
        // Sort non-terminals for consistent output
        vector<string> sortedNonTerminals(nonTerminals.begin(), nonTerminals.end());
        sort(sortedNonTerminals.begin(), sortedNonTerminals.end());

        for (const auto& nonTerm : sortedNonTerminals) {
            cout << "First(" << left << setw(max(10, (int)nonTerm.length())) << nonTerm << ") = { ";
            // Sort terminals within the set for consistent output
            set<string>& fSet = first[nonTerm];
            vector<string> sortedFirst(fSet.begin(), fSet.end());
            sort(sortedFirst.begin(), sortedFirst.end());
            string sep = "";
            for (const auto& term : sortedFirst) {
                 cout << sep << term;
                 sep = ", ";
            }
            cout << " }" << endl;
        }


        cout << "\nFollow Sets:" << endl;
        // iterate over all non-terminals and print items of each non-terminal's follow set
        for (const auto& nonTerm : sortedNonTerminals) {
            cout << "Follow(" << left << setw(max(10, (int)nonTerm.length())) << nonTerm << ") = { ";
             // Sort terminals within the set for consistent output
            set<string>& flSet = follow[nonTerm];
            vector<string> sortedFollow(flSet.begin(), flSet.end());
            sort(sortedFollow.begin(), sortedFollow.end());
            string sep = "";
            for (const auto& term : sortedFollow) {
                cout << sep << term;
                sep = ", ";
            }
            cout << " }" << endl;
        }
    }

    int computeParsingTable() {
        // Make sure First and Follow sets are computed
        if (first.empty()) computeFirst();
        if (follow.empty()) computeFollow();


        // Clear the existing parsing table
        parsingTable.clear();

        // Add $ as a terminal for end of input if not already present
        terminals.insert("$");

        // Iterative over rules in the cfg: A -> α
        for (const auto& rule : cfg) {
            string nonTerm_A = rule.first;

            for (const string& prodStr : rule.second) { // α
                 vector<string> prod_alpha = tokenizeProduction(prodStr);

                // Compute FIRST(α)
                set<string> firstOfAlpha;
                bool alphaDerivesEpsilon = computeFirstOfSequence(prod_alpha, firstOfAlpha);


                // Rule 1: For each terminal 'a' in FIRST(α), add A -> α to M[A, a]
                for (const string& term_a : firstOfAlpha) {
                    if (term_a != "ε") {
                        pair<string, string> tableKey = make_pair(nonTerm_A, term_a);

                        // Check for conflicts (non-LL(1) grammar)
                        if (parsingTable.find(tableKey) != parsingTable.end() && parsingTable[tableKey] != prodStr) {
                            cerr << "\nLL(1) Conflict Detected!" << endl;
                            cerr << "  At Table[" << nonTerm_A << ", " << term_a << "]:" << endl;
                            cerr << "  Existing production: " << nonTerm_A << " -> " << parsingTable[tableKey] << endl;
                            cerr << "  New production:      " << nonTerm_A << " -> " << prodStr << endl;
                            cerr << "  FIRST(" << prodStr << ") = {";
                            for(const auto& f : firstOfAlpha) cerr << f << ","; cerr << "}" << endl;
                            cerr << "  FOLLOW(" << nonTerm_A << ") = {";
                            for(const auto& f : follow[nonTerm_A]) cerr << f << ","; cerr << "}" << endl;
                            // Optionally return an error code or throw exception
                        }

                        // Add the original string production to the parsing table
                        parsingTable[tableKey] = prodStr;
                    }
                }

                // Rule 2: If ε is in FIRST(α), then for each terminal 'b' in FOLLOW(A), add A -> α to M[A, b]
                if (alphaDerivesEpsilon) { // Check if the whole sequence α can derive ε
                    for (const string& term_b : follow[nonTerm_A]) { // term_b includes '$' if applicable
                        pair<string, string> tableKey = make_pair(nonTerm_A, term_b);

                        // Check for conflicts
                         if (parsingTable.find(tableKey) != parsingTable.end() && parsingTable[tableKey] != prodStr) {
                            cerr << "\nLL(1) Conflict Detected (Epsilon Rule)!" << endl;
                            cerr << "  At Table[" << nonTerm_A << ", " << term_b << "]:" << endl;
                            cerr << "  Existing production: " << nonTerm_A << " -> " << parsingTable[tableKey] << endl;
                            cerr << "  New production:      " << nonTerm_A << " -> " << prodStr << " (due to FOLLOW set)" << endl;
                             cerr << "  FIRST(" << prodStr << ") = {";
                            for(const auto& f : firstOfAlpha) cerr << f << ","; cerr << "}" << endl;
                            cerr << "  FOLLOW(" << nonTerm_A << ") = {";
                            for(const auto& f : follow[nonTerm_A]) cerr << f << ","; cerr << "}" << endl;
                            // Optionally return an error code or throw exception
                        }

                        // Add the original string production to the parsing table
                        parsingTable[tableKey] = prodStr;
                    }
                }
            }
        }

        return 1; // Indicate success (though conflicts might have been printed)
    }

    void printParsingTable() {
        cout << "\nLL(1) Parsing Table:" << endl;

        const int colWidth = 20;
        map<pair<string, string>, string> truncatedEntries;

        // Top border
        cout << "+" << string(colWidth, '-');
        for (const auto& term : terminals) cout << "+" << string(colWidth, '-');
        cout << "+" << endl;

        // Header row
        cout << "|" << setw(colWidth) << left << " NT \\ Terminal";
        for (const auto& term : terminals) cout << "|" << setw(colWidth) << left << term;
        cout << "|" << endl;

        // Separator line
        cout << "+" << string(colWidth, '-');
        for (size_t i = 0; i < terminals.size(); i++) cout << "+" << string(colWidth, '-');
        cout << "+" << endl;

        // MAIN PARSING TABLE
        for (const auto& nonTerm : nonTerminals) {
            cout << "|" << setw(colWidth) << left << nonTerm;

            for (const auto& term : terminals) {
                pair<string, string> key = { nonTerm, term };
                string cellContent;

                if (parsingTable.count(key)) {
                    string production = parsingTable[key];

                    /*
                    // replace epsilon with ^
                    size_t pos;
                    while ((pos = production.find("\u03B5")) != string::npos) production.replace(pos, 2, "^");
                    */

                    cellContent = nonTerm + " → " + production;

                    // truncation
                    if (cellContent.length() > colWidth) {
                        // truncate after the arrow
                        size_t truncPos = cellContent.find(' ', 10);

                        if (truncPos == string::npos) truncPos = colWidth - 4;

                        truncatedEntries[key] = production;
                        cellContent = cellContent.substr(0, truncPos) + "...";
                    }

                }
                else {
                    // explicitly pad empty cells with spaces
                    cellContent = string(colWidth, ' ');
                }

                cout << "|" << setw(colWidth) << left << cellContent;
            }
            cout << "|" << endl;
        }

        // bottom border
        cout << "+" << string(colWidth, '-');
        for (size_t i = 0; i < terminals.size(); i++) cout << "+" << string(colWidth, '-');
        cout << "+" << endl;

        // Key for truncated entries
        if (!truncatedEntries.empty()) {
            cout << "\nKey:\n";
            cout << "* Truncated Entries (full production):\n";
            for (const auto& entry : truncatedEntries) cout << "  - " << entry.first.first << " → " << entry.second << endl;
        }
        cout << "* Empty cells indicate no production\n";
    }

    void writeParsingTableToCSV(const string& filename) {
        ofstream csvFile(filename);
        if (!csvFile.is_open()) {
            cerr << "Error: Could not open file " << filename << endl;
            return;
        }

        // Write CSV header (terminals)
        csvFile << "Non-Terminal";
        for (const auto& term : terminals) csvFile << "," << term;
        csvFile << "\n";

        // Write rows for each non-terminal
        for (const auto& nonTerm : nonTerminals) {
            csvFile << nonTerm; // First column: non-terminal

            for (const auto& term : terminals) {
                auto key = make_pair(nonTerm, term);
                string production;

                if (parsingTable.find(key) != parsingTable.end()) {
                    production = nonTerm + " → " + parsingTable[key];

                    /*
                    // Replace ε with ^
                    size_t pos;
                    while ((pos = production.find("ε")) != string::npos) production.replace(pos, 2, "^");
                    */
                }

                csvFile << "," << production;
            }
            csvFile << "\n";
        }

        csvFile.close();
        cout << "Parsing table saved to " << filename << endl;
    }

};

int main() {
    string fileName = "cfg.txt";
    Grammar cfg;

    // Redirect cout and cerr to both console and file
    ofstream outFile("output.log");

    // TeeBuf to duplicate cout output to both the console and the file
    TeeBuf coutTeeBuf(cout.rdbuf(), outFile.rdbuf());

    // TeeBuf to duplicate cerr output to both the console and the file
    TeeBuf cerrTeeBuf(cerr.rdbuf(), outFile.rdbuf());

    // redirect cout to use the custom TeeBuf, saving the original buffer
    streambuf* originalCout = cout.rdbuf(&coutTeeBuf);

    // redirect cerr to use the custom TeeBuf, saving the original buffer
    streambuf* originalCerr = cerr.rdbuf(&cerrTeeBuf);

    if (cfg.readGrammar(fileName)) {
        cout << "Original Grammar:" << endl;
        cfg.printGrammar();

        // left factoring
        cfg.leftFactoring();
        cout << "\nGrammar after left factoring:" << endl;
        cfg.printGrammar();


        // left recursion elimination
        cfg.leftRecursion();
        cout << "\nGrammar after left recursion elimination:" << endl;
        cfg.printGrammar();


        // Compute First and Follow sets
        cfg.computeFirst();
        cfg.computeFollow();
        cout << "\nGrammar after computing first follow:" << endl;
        cfg.printFirstAndFollow();


        // Compute LL(1) Parsing Table
        cfg.computeParsingTable();
        cout << "\nGrammar after computing parsing table:" << endl;
        cfg.printParsingTable();

        cfg.writeParsingTableToCSV("ll1_parsing_table.csv");

        }

    // restore original buffers and close file
    cout.rdbuf(originalCout);
    cerr.rdbuf(originalCerr);
    outFile.close();

    return 0;
}

