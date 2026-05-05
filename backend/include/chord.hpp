#pragma once

#include <cstdint>

class Shape {
    // A small int that represents the fret pressed on each string. 0 means open string, 1 means first fret, etc.   
    uint8_t _first, _second, _third, _fourth, _fifth, _sixth;
    uint8_t _muted; // A small int that represents which strings are muted. 0 means not muted, 1 means muted.
    void print();
};