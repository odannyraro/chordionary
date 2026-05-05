#include "lexer.hpp"
#include <iostream>
#include <cctype>
#include <utility>

std::string Token::typeStr() const {
    switch (type) {
        case TokenType::ROOT:      return "ROOT";
        case TokenType::QUALITY:   return "QUALITY";
        case TokenType::EXTENSION: return "EXTENSION";
        case TokenType::MODIFIER:  return "MODIFIER";
        case TokenType::SLASH:     return "SLASH";
        case TokenType::L_PAREN:   return "L_PAREN";
        case TokenType::R_PAREN:   return "R_PAREN";
        default:                   return "UNKNOWN";
    }
}

ChordLexer::ChordLexer(std::string str) : input(std::move(str)), pos(0) {}

char ChordLexer::peek() const { 
    return (pos < input.size()) ? input[pos] : '\0'; 
}

char ChordLexer::advance() { 
    return input[pos++]; 
}

std::vector<Token> ChordLexer::tokenize() {
    std::vector<Token> tokens;

    while (pos < input.size()) {
        char current = peek();

        if (std::isspace(current)) {
            advance();
            continue;
        }

        // 1. ROOT: A-G + Acidente Opcional (Acoplamento imediato)
        if (current >= 'A' && current <= 'G') {
            std::string val;
            val += advance();
            if (peek() == '#' || peek() == 'b') {
                val += advance();
            }
            tokens.push_back({TokenType::ROOT, val});
            continue;
        }

        // 2. ACIDENTE ANTECIPADO: Se b ou # aparecer fora da Root
        // indica acoplamento com a PRÓXIMA extensão (ex: b9)
        if (current == '#' || current == 'b') {
            std::string val;
            val += advance(); // pega # ou b
            while (std::isdigit(peek())) {
                val += advance();
            }
            tokens.push_back({TokenType::EXTENSION, val});
            continue;
        }

        // 3. NÚMEROS (Extensões sem acidente)
        if (std::isdigit(current)) {
            std::string val;
            while (std::isdigit(peek())) {
                val += advance();
            }
            tokens.push_back({TokenType::EXTENSION, val});
            continue;
        }

        // 4. SLASH, PAREN
        if (current == '/') { tokens.push_back({TokenType::SLASH, std::string(1, advance())}); continue; }
        if (current == '(') { tokens.push_back({TokenType::L_PAREN, std::string(1, advance())}); continue; }
        if (current == ')') { tokens.push_back({TokenType::R_PAREN, std::string(1, advance())}); continue; }

        // 5. PALAVRAS (m, maj, sus, add)
        if (std::isalpha(current) || current == '+') {
            std::string val;
            while (std::isalpha(peek()) || peek() == '+') {
                val += advance();
            }
            if (val == "sus" || val == "add" || val == "omit")
                tokens.push_back({TokenType::MODIFIER, val});
            else
                tokens.push_back({TokenType::QUALITY, val});
            continue;
        }

        advance(); // Unknown character fallback
    }
    return tokens;
}