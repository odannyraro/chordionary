#ifndef LEXER_HPP
#define LEXER_HPP

#include <string>
#include <vector>

enum class TokenType {
    ROOT,           // Ex: C#, Eb
    QUALITY,        // Ex: m, maj, aug, +
    EXTENSION,      // Ex: 7, b9, #11
    MODIFIER,       // Ex: sus, add
    SLASH,          // /
    L_PAREN,        // (
    R_PAREN,        // )
    UNKNOWN
};

struct Token {
    TokenType type;
    std::string value;

    std::string typeStr() const;
};

class ChordLexer {
private:
    std::string input;
    size_t pos;

    char peek() const;
    char advance();

public:
    ChordLexer(std::string str);

    std::vector<Token> tokenize();
};

#endif // LEXER_HPP
