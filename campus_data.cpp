#include "campus_data.h"

#include <fstream>
#include <iostream>

namespace campus {

std::string trim(const std::string& s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a == std::string::npos) return "";
    size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
}

std::vector<std::string> split(const std::string& s, char delim) {
    std::vector<std::string> out;
    std::string cur;
    for (char c : s) {
        if (c == delim) {
            out.push_back(trim(cur));
            cur.clear();
        } else {
            cur += c;
        }
    }
    out.push_back(trim(cur));
    return out;
}

int CampusData::addLocation(const std::string& name) {
    auto it = locIndex_.find(name);
    if (it != locIndex_.end()) return it->second;
    int id = static_cast<int>(locations.size());
    locIndex_[name] = id;
    locations.push_back(name);
    adj.emplace_back();
    return id;
}

int CampusData::locationId(const std::string& name) const {
    auto it = locIndex_.find(name);
    return it == locIndex_.end() ? -1 : it->second;
}

int CampusData::courseId(const std::string& code) const {
    auto it = courseIndex_.find(code);
    return it == courseIndex_.end() ? -1 : it->second;
}

bool CampusData::loadMap(const std::string& path) {
    std::ifstream in(path);
    if (!in) return false;
    std::string line;
    while (std::getline(in, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#') continue;
        auto parts = split(line, ',');
        if (parts.size() != 3) continue;
        int w = std::stoi(parts[2]);
        int u = addLocation(parts[0]);
        int v = addLocation(parts[1]);
        adj[u].push_back({v, w});
        adj[v].push_back({u, w});
        edges.push_back({u, v, w});
    }
    return !locations.empty();
}

bool CampusData::loadCourses(const std::string& path) {
    std::ifstream in(path);
    if (!in) return false;

    // pass 1: read all lines
    struct Row { std::string code, name, prereqs; };
    std::vector<Row> rows;
    std::string line;
    while (std::getline(in, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#') continue;
        auto parts = split(line, '|');
        if (parts.size() < 2) continue;
        rows.push_back({parts[0], parts[1], parts.size() > 2 ? parts[2] : ""});
    }

    // pass 2: assign ids
    for (const auto& r : rows) {
        courseIndex_[r.code] = static_cast<int>(courseCodes.size());
        courseCodes.push_back(r.code);
        courseNames.push_back(r.name);
    }
    unlocks.assign(courseCodes.size(), {});
    prereqCount.assign(courseCodes.size(), 0);

    // pass 3: build edges prerequisite -> course
    for (const auto& r : rows) {
        int c = courseIndex_[r.code];
        if (r.prereqs.empty()) continue;
        for (const auto& p : split(r.prereqs, ',')) {
            int pid = courseId(p);
            if (pid < 0) {
                std::cerr << "warning: unknown prerequisite '" << p << "' for " << r.code << "\n";
                continue;
            }
            unlocks[pid].push_back(c);
            prereqCount[c]++;
        }
    }
    return !courseCodes.empty();
}

}  // namespace campus
