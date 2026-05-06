#include "chord.hpp"
#include "lexer.hpp"
#include <string>
#include <vector>
#include <stdexcept>
#include <set>

Notes parseNote(const std::string& str) {
    if (str.empty()) throw std::invalid_argument("Empty note string");
    char base = str[0];
    int val = 0;
    switch (base) {
        case 'C': val = 0; break;
        case 'D': val = 2; break;
        case 'E': val = 4; break;
        case 'F': val = 5; break;
        case 'G': val = 7; break;
        case 'A': val = 9; break;
        case 'B': val = 11; break;
        default: throw std::invalid_argument("Invalid base note");
    }
    if (str.length() > 1) {
        if (str[1] == '#') val++;
        else if (str[1] == 'b') val--;
    }
    val = (val % 12 + 12) % 12; // Normalize
    return static_cast<Notes>(val);
}

Chord::Chord(std::string str) : _name(str) {
    ChordLexer lexer(str);
    std::vector<Token> tokens = lexer.tokenize();
    
    if (tokens.empty() || tokens[0].type != TokenType::ROOT) {
        throw std::invalid_argument("Invalid chord string: must start with a ROOT note");
    }

    this->root = parseNote(tokens[0].value);
    this->bass = this->root;

    std::set<int> intervals = {4, 7}; // Default: Major 3rd, Perfect 5th
    bool isMajFlag = false;

    for (size_t i = 1; i < tokens.size(); i++) {
        const auto& token = tokens[i];
        
        if (token.type == TokenType::QUALITY) {
            if (token.value == "m" || token.value == "min") {
                intervals.erase(4);
                intervals.insert(3);
            } else if (token.value == "dim") {
                intervals.erase(4);
                intervals.insert(3);
                intervals.erase(7);
                intervals.insert(6);
            } else if (token.value == "aug" || token.value == "+") {
                intervals.erase(7);
                intervals.insert(8);
            } else if (token.value == "maj") {
                isMajFlag = true;
            }
        } else if (token.type == TokenType::EXTENSION) {
            if (token.value == "7") {
                intervals.insert(isMajFlag ? 11 : 10);
            } else if (token.value == "9") {
                intervals.insert(isMajFlag ? 11 : 10);
                intervals.insert(14);
            } else if (token.value == "b9") {
                intervals.insert(13);
            } else if (token.value == "#9") {
                intervals.insert(15);
            } else if (token.value == "11") {
                intervals.insert(isMajFlag ? 11 : 10);
                intervals.insert(14);
                intervals.insert(17);
            } else if (token.value == "#11") {
                intervals.insert(18);
            } else if (token.value == "13") {
                intervals.insert(isMajFlag ? 11 : 10);
                intervals.insert(14);
                intervals.insert(21);
            } else if (token.value == "b13") {
                intervals.insert(20);
            }
        } else if (token.type == TokenType::MODIFIER) {
            if (token.value == "sus2") {
                intervals.erase(3);
                intervals.erase(4);
                intervals.insert(2);
            } else if (token.value == "sus4" || token.value == "sus") {
                intervals.erase(3);
                intervals.erase(4);
                intervals.insert(5);
            } else if (token.value == "add9" || token.value == "add2") {
                intervals.insert(14);
            } else if (token.value == "add11" || token.value == "add4") {
                intervals.insert(17);
            } else if (token.value == "omit3" || token.value == "no3") {
                intervals.erase(3);
                intervals.erase(4);
            } else if (token.value == "omit5" || token.value == "no5") {
                intervals.erase(6);
                intervals.erase(7);
                intervals.erase(8);
            }
        } else if (token.type == TokenType::SLASH) {
            if (i + 1 < tokens.size() && tokens[i+1].type == TokenType::ROOT) {
                this->bass = parseNote(tokens[i+1].value);
                i++; // Skip the processed ROOT token
            }
        }
    }

    // Convert intervals to actual Notes and store in extensions vector
    for (int interval : intervals) {
        int noteVal = (static_cast<int>(this->root) + interval) % 12;
        this->extensions.push_back(static_cast<Notes>(noteVal));
    }

    // Calculate total unique notes
    this->noteCount = 1 + this->extensions.size(); // root + extensions
    if (this->bass != this->root) {
        bool bassInExt = false;
        for (Notes ext : this->extensions) {
            if (this->bass == ext) {
                bassInExt = true;
                break;
            }
        }
        if (!bassInExt) {
            this->noteCount++;
        }
    }
}

