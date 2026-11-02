#include "integration.h"

#include <fstream>
#include <limits>

#include "navigation.h"

namespace campus {

std::vector<Staff> loadStaff(const std::string& path, const CampusData& d) {
    std::vector<Staff> out;
    std::ifstream in(path);
    if (!in) return out;
    std::string line;
    while (std::getline(in, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#') continue;
        auto p = split(line, '|');
        if (p.size() != 3) continue;
        int loc = d.locationId(p[2]);
        if (loc < 0) continue;
        out.push_back({p[0], p[1], loc});
    }
    return out;
}

Dispatch dispatchNearest(const CampusData& d, const std::vector<Staff>& staff, const Request& r) {
    Dispatch best;
    int target = d.locationId(r.location);
    if (target < 0) return best;

    std::vector<int> dist = distancesFrom(d, target);       // undirected graph: symmetric distances
    const int INF = std::numeric_limits<int>::max();
    int bestDist = INF;
    const Staff* chosen = nullptr;
    for (const Staff& s : staff) {
        if (s.role != r.category) continue;
        if (dist[s.location] < bestDist) {
            bestDist = dist[s.location];
            chosen = &s;
        }
    }
    if (!chosen) return best;

    best.found = true;
    best.staff = *chosen;
    best.distance = bestDist;
    best.path = shortestPath(d, chosen->location, target).path;
    return best;
}

}  // namespace campus
