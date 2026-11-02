#include "network_planning.h"

#include <algorithm>
#include <numeric>

namespace campus {

DisjointSet::DisjointSet(int n) : parent_(n), rank_(n, 0) {
    std::iota(parent_.begin(), parent_.end(), 0);
}

int DisjointSet::find(int x) {
    if (parent_[x] != x) parent_[x] = find(parent_[x]);   // path compression
    return parent_[x];
}

bool DisjointSet::unite(int a, int b) {
    a = find(a);
    b = find(b);
    if (a == b) return false;
    if (rank_[a] < rank_[b]) std::swap(a, b);              // union by rank
    parent_[b] = a;
    if (rank_[a] == rank_[b]) rank_[a]++;
    return true;
}

MSTResult planNetwork(const CampusData& d) {
    MSTResult res;
    int n = static_cast<int>(d.locations.size());
    std::vector<WeightedEdge> sorted = d.edges;
    std::sort(sorted.begin(), sorted.end(),
              [](const WeightedEdge& a, const WeightedEdge& b) { return a.weight < b.weight; });

    DisjointSet dsu(n);
    for (const WeightedEdge& e : sorted) {
        if (dsu.unite(e.u, e.v)) {
            res.chosen.push_back(e);
            res.totalLength += e.weight;
            if (static_cast<int>(res.chosen.size()) == n - 1) break;
        }
    }
    res.connected = (static_cast<int>(res.chosen.size()) == n - 1);
    return res;
}

}  // namespace campus
