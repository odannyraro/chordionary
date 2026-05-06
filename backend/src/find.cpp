#include "find.hpp"
#include <iostream>
#include <algorithm>

Shape::Shape(std::string n, uint8_t f1, uint8_t f2, uint8_t f3, uint8_t f4, uint8_t f5, uint8_t f6, uint8_t m)
    : name(std::move(n)), _first(f1), _second(f2), _third(f3), _fourth(f4), _fifth(f5), _sixth(f6), _muted(m) {}

void to_json(nlohmann::json& j, const Shape& s) {
    j = nlohmann::json{{"name", s.name}};
    
    // Helper lambda to check bitmask
    auto getFret = [&s](uint8_t fret, int bit) -> nlohmann::json {
        if (s._muted & (1 << bit)) return nullptr; // Muted returns null in JSON
        return fret;
    };

    // Frontend standard array order: [_sixth, _fifth, _fourth, _third, _second, _first]
    j["frets"] = {
        getFret(s._sixth, 5),
        getFret(s._fifth, 4),
        getFret(s._fourth, 3),
        getFret(s._third, 2),
        getFret(s._second, 1),
        getFret(s._first, 0)
    };
}

static bool isNoteInChord(Notes note, const Chord& chord) {
    if (note == chord.root || note == chord.bass) return true;
    for (Notes ext : chord.extensions) {
        if (note == ext) return true;
    }
    return false;
}

static int calculateFingers(uint8_t frets[6]) {
    int pressed_count = 0;
    int min_fret = 14;
    for (int i = 0; i < 6; ++i) {
        if (frets[i] != 0 && frets[i] != 254) {
            pressed_count++;
            if (frets[i] < min_fret) min_fret = frets[i];
        }
    }
    if (pressed_count == 0) return 0;

    int fingers_no_barre = pressed_count;

    // Try barre at min_fret
    int min_fret_count = 0;
    int barre_start = -1; // lowest string index
    for (int i = 0; i < 6; ++i) {
        if (frets[i] == min_fret) {
            min_fret_count++;
            if (barre_start == -1) barre_start = i;
        }
    }

    if (min_fret_count >= 2) {
        bool can_barre = true;
        // Check for open strings covered by the barre
        for (int i = barre_start; i < 6; ++i) {
            if (frets[i] == 0) {
                can_barre = false;
                break;
            }
        }
        if (can_barre) {
            int fingers_with_barre = 1 + (pressed_count - min_fret_count);
            return std::min(fingers_no_barre, fingers_with_barre);
        }
    }
    return fingers_no_barre;
}

static bool isValidShape(const Notes fretboard[6][14], uint8_t current_frets[6], const Chord& chord) {
    if (calculateFingers(current_frets) > 4) return false;

    bool hasRoot = false;
    bool hasBass = false;
    std::vector<Notes> required_extensions = chord.extensions;
    
    int lowest_played_string = -1;
    for (int i = 0; i < 6; ++i) {
        if (current_frets[i] != 254) {
            lowest_played_string = i;
            break;
        }
    }
    
    if (lowest_played_string == -1) return false;

    Notes played_bass = fretboard[lowest_played_string][current_frets[lowest_played_string]];
    if (played_bass != chord.bass) return false;

    for (int i = 0; i < 6; ++i) {
        if (current_frets[i] == 254) continue;
        Notes note = fretboard[i][current_frets[i]];
        if (note == chord.root) hasRoot = true;
        if (note == chord.bass) hasBass = true;
        
        auto it = std::find(required_extensions.begin(), required_extensions.end(), note);
        if (it != required_extensions.end()) {
            required_extensions.erase(it);
        }
    }

    return hasRoot && hasBass && required_extensions.empty();
}

static void find_recursive(
    const Notes fretboard[6][14],
    const Chord& chord,
    int current_string,
    int current_fret,
    uint8_t current_frets[6],
    std::vector<Shape>& results
) {
    if (current_string == 6) {
        if (isValidShape(fretboard, current_frets, chord)) {
            uint8_t m = 0;
            uint8_t f[6] = {0};
            for (int i = 0; i < 6; ++i) {
                if (current_frets[i] == 254) {
                    m |= (1 << i);
                } else {
                    f[i] = current_frets[i];
                }
            }
            results.emplace_back("Position " + std::to_string(results.size() + 1), f[0], f[1], f[2], f[3], f[4], f[5], m);
        }
        return;
    }

    int chosen_notes = 0;
    bool has_played_note = false;
    for (int i = 0; i < current_string; ++i) {
        if (current_frets[i] != 254) {
            chosen_notes++;
            has_played_note = true;
        }
    }

    int remaining_strings = 6 - current_string;
    
    // Starvation pruning
    if (chosen_notes + remaining_strings < chord.noteCount) {
        return;
    }

    // Ergonomic constraint variables
    int current_min_fret = 14;
    int current_max_fret = -1;
    for (int i = 0; i < current_string; ++i) {
        if (current_frets[i] != 254 && current_frets[i] != 0) { // Not muted and not open string
            if (current_frets[i] < current_min_fret) current_min_fret = current_frets[i];
            if (current_frets[i] > current_max_fret) current_max_fret = current_frets[i];
        }
    }

    // Branching: Pick-or-Skip for the current string
    for (int fret = current_fret; fret < 14; ++fret) {
        Notes note = fretboard[current_string][fret];
        
        // Ergonomic Check
        if (fret > 0) {
            int new_min = current_min_fret;
            int new_max = current_max_fret;
            if (fret < new_min) new_min = fret;
            if (fret > new_max) new_max = fret;
            
            // Limit the span of pressed frets to 4 frets difference
            if (new_max != -1 && new_min != 14 && (new_max - new_min > 4)) {
                continue;
            }
        }

        if (!has_played_note) {
            if (note != chord.bass) continue;
        } else {
            if (!isNoteInChord(note, chord)) continue;
        }

        // Ramo 1: Usa a nota e vai para a proxima corda
        current_frets[current_string] = fret;
        find_recursive(fretboard, chord, current_string + 1, 0, current_frets, results);
        current_frets[current_string] = 255; // Backtrack
        
        // O proprio avanco do loop for atua como o Ramo 2 (Ignorar a nota e testar outras casas)
    }

    // Ramo 3: Mutar a corda
    bool can_mute = true;
    if (current_string >= 2) {
        bool all_prev_muted = true;
        for (int i = 0; i < current_string; ++i) {
            if (current_frets[i] != 254) {
                all_prev_muted = false;
                break;
            }
        }
        if (all_prev_muted) {
            can_mute = false;
        }
    }

    if (can_mute) {
        current_frets[current_string] = 254;
        find_recursive(fretboard, chord, current_string + 1, 0, current_frets, results);
        current_frets[current_string] = 255;
    }
}

std::vector<Shape> findShapes(const Chord& chord, const std::array<Notes, 6>& tuning) {
    Notes fretboard[6][14];
    
    for (int stringIdx = 0; stringIdx < 6; ++stringIdx) {
        int openNoteVal = static_cast<int>(tuning[stringIdx]);
        for (int fret = 0; fret < 14; ++fret) {
            fretboard[stringIdx][fret] = static_cast<Notes>((openNoteVal + fret) % 12);
        }
    }

    std::vector<Shape> shapes;
    uint8_t current_frets[6] = {255, 255, 255, 255, 255, 255};
    
    find_recursive(fretboard, chord, 0, 0, current_frets, shapes);

    return shapes;
}
