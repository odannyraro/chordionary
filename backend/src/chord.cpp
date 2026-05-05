#include "chord.hpp"
#include "lexer.hpp"
#include <string>
#include <vector>

Chord::Chord(std::string str) : _name(str) {
    ChordLexer lexer(str);
    std::vector<Token> tokens = lexer.tokenize();
    for (auto token : tokens) {
        
    }
}

