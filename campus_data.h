// campus_data.h
// Central data layer shared by every module (so the same data is never stored twice).
#pragma once
#include <string>
#include <unordered_map>
#include <vector>

namespace campus {

struct Edge {
    int to;
    int weight;
};

struct WeightedEdge {
    int u, v, weight;
};

class CampusData {
public:
    // ---- campus map (undirected weighted graph, adjacency list) ----
    std::vector<std::string> locations;
    std::vector<std::vector<Edge>> adj;
    std::vector<WeightedEdge> edges;

    // ---- courses (directed graph: prerequisite -> course) ----
    std::vector<std::string> courseCodes;
    std::vector<std::string> courseNames;
    std::vector<std::vector<int>> unlocks;   // unlocks[p] = courses that need p
    std::vector<int> prereqCount;            // number of prerequisites of each course

    bool loadMap(const std::string& path);
    bool loadCourses(const std::string& path);

    int locationId(const std::string& name) const;
    int courseId(const std::string& code) const;

private:
    std::unordered_map<std::string, int> locIndex_;
    std::unordered_map<std::string, int> courseIndex_;
    int addLocation(const std::string& name);
};

// small helper used by all loaders
std::string trim(const std::string& s);
std::vector<std::string> split(const std::string& s, char delim);

}  // namespace campus
