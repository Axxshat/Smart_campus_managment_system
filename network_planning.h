// network_planning.h  --  Module 2: Infrastructure Network Planning
// Concepts: Minimum Spanning Tree (Kruskal) + Disjoint Set Union (Union-Find)
#pragma once
#include <vector>
#include "campus_data.h"

namespace campus {

// Union-Find with path compression and union by rank, ~O(alpha(n)) per operation
class DisjointSet {
public:
    explicit DisjointSet(int n);
    int find(int x);
    bool unite(int a, int b);   // false if already in the same set
private:
    std::vector<int> parent_, rank_;
};

struct MSTResult {
    std::vector<WeightedEdge> chosen;
    int totalLength = 0;
    bool connected = false;     // false => campus graph has isolated parts
};

// Kruskal, O(E log E): minimum total cable / pipe length that links all buildings
MSTResult planNetwork(const CampusData& d);

}  // namespace campus
