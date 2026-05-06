#include <iostream>
#include <string>
#include <vector>
#include "chord.hpp"
#include "find.hpp"
#include <nlohmann/json.hpp>
#include <httplib.h>

using json = nlohmann::json;

int main() {
    httplib::Server svr;

    svr.Post("/api/chord", [](const httplib::Request& req, httplib::Response& res) {
        // Set CORS headers so the frontend can call this API
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "POST, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type");

        try {
            // Parse JSON request
            auto req_json = json::parse(req.body);
            std::string chordName = req_json.value("chordName", "");

            if (chordName.empty()) {
                res.status = 400;
                res.set_content(json{{"error", "No chordName provided"}}.dump(), "application/json");
                return;
            }

            // Extract tuning from JSON (frontend sends 1st string to 6th string)
            std::array<Notes, 6> tuningArray;
            if (req_json.contains("tuning") && req_json["tuning"].is_array() && req_json["tuning"].size() == 6) {
                auto tuning_json = req_json["tuning"];
                for (int i = 0; i < 6; i++) {
                    // tuning_json[0] is 1st string (High E). We want tuningArray[0] to be 6th string (Low E).
                    tuningArray[i] = parseNote(tuning_json[5 - i].get<std::string>());
                }
            } else {
                // Default tuning: E A D G B E
                tuningArray = {Notes::E, Notes::A, Notes::D, Notes::G, Notes::B, Notes::E};
            }

            // Test semantic analyzer logic
            Chord chord(chordName);
            std::cout << "Received chord: " << chord._name << std::endl;

            // Generate matrix and find shapes
            std::vector<Shape> shapes = findShapes(chord, tuningArray);

            if (shapes.empty()) {
                res.status = 404;
                res.set_content(json{{"error", "Nenhum shape encontrado para este acorde."}}.dump(), "application/json");
                return;
            }

            // Respond with shapes
            json response_json;
            response_json["shapes"] = shapes;
            res.set_content(response_json.dump(), "application/json");

        } catch (const std::exception& e) {
            res.status = 500;
            res.set_content(json{{"error", std::string("Internal error: ") + e.what()}}.dump(), "application/json");
        }
    });

    // Handle CORS preflight requests
    svr.Options("/api/chord", [](const httplib::Request&, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "POST, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type");
        res.status = 204;
    });

    std::cout << "Starting Chordionary Backend API on http://0.0.0.0:8080" << std::endl;
    svr.listen("0.0.0.0", 8080);

    return 0;
}