// navigation.h  --  Module 1: Campus Navigation
// Concepts: Graph (adjacency list), Dijkstra with min-heap, BFS, DFS
#pragma once
#include <vector>
#include "campus_data.h"

namespace campus {

struct PathResult {
    bool reachable = false;
    int distance = 0;          // metres (Dijkstra) or number of hops (BFS)
    std::vector<int> path;     // location ids from source to destination
};

// shortest walking distance, O((V + E) log V)
PathResult shortestPath(const CampusData& d, int src, int dst);

// route with the fewest stops (unweighted), O(V + E)
PathResult fewestStops(const CampusData& d, int src, int dst);

// distance from `src` to every location (INT_MAX if unreachable), O((V + E) log V)
std::vector<int> distancesFrom(const CampusData& d, int src);

// traversals from a source, O(V + E)
std::vector<int> bfsOrder(const CampusData& d, int src);
std::vector<int> dfsOrder(const CampusData& d, int src);

}  // namespace campus
