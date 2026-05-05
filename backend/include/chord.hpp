#pragma once

#include <cstdint>

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

class Chord {
    Notes root;
    Notes bass;
    std::vector<Notes> extensions;
    std::string _name;
    Chord(std::string str);
}

class Shape {
    // A small int that represents the fret pressed on each string. 0 means open string, 1 means first fret, etc.   
    uint8_t _first, _second, _third, _fourth, _fifth, _sixth;
    uint8_t _muted; // A small int that represents which strings are muted. 0 means not muted, 1 means muted.
};