#pragma once

#include "chord.hpp"
#include <vector>
#include <array>
#include <string>
#include <nlohmann/json.hpp>

class Shape {
public:
    std::string name;
    // Fret pressed on each string. 0 means open string.
    uint8_t _first, _second, _third, _fourth, _fifth, _sixth;
    uint8_t _muted; // Bitmask: bit 0 is _first, bit 1 is _second, etc. (1 means muted)

    Shape(std::string n, uint8_t f1, uint8_t f2, uint8_t f3, uint8_t f4, uint8_t f5, uint8_t f6, uint8_t m);

    // Convert Shape to nlohmann::json
    friend void to_json(nlohmann::json& j, const Shape& s);
};

// Generates the fretboard and finds shapes for the given chord and tuning
std::vector<Shape> findShapes(const Chord& chord, const std::array<Notes, 6>& tuning);
