#pragma once

#include <cstdint>
#include <vector>
#include <string>

enum class Notes : uint8_t {
    C = 0,
    C_SHARP = 1,
    D = 2,
    D_SHARP = 3,
    E = 4,
    F = 5,
    F_SHARP = 6,
    G = 7,
    G_SHARP = 8,
    A = 9,
    A_SHARP = 10,
    B = 11
};

Notes parseNote(const std::string& str);

class Chord {
public:
    Notes root;
    Notes bass;
    uint8_t noteCount;
    std::vector<Notes> extensions;
    std::string _name;
    Chord(std::string str);
};