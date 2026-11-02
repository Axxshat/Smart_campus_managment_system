#include "navigation.h"

#include <algorithm>
#include <functional>
#include <limits>
#include <queue>

namespace campus {

static std::vector<int> buildPath(const std::vector<int>& parent, int src, int dst) {
    std::vector<int> path;
    for (int v = dst; v != -1; v = parent[v]) path.push_back(v);
    std::reverse(path.begin(), path.end());
    if (path.empty() || path.front() != src) path.clear();
    return path;
}

PathResult shortestPath(const CampusData& d, int src, int dst) {
    const int INF = std::numeric_limits<int>::max();
    int n = static_cast<int>(d.locations.size());
    std::vector<int> dist(n, INF), parent(n, -1);

    using P = std::pair<int, int>;  // (distance, node)
    std::priority_queue<P, std::vector<P>, std::greater<P>> pq;
    dist[src] = 0;
    pq.push({0, src});

    while (!pq.empty()) {
        auto [du, u] = pq.top();
        pq.pop();
        if (du > dist[u]) continue;          // stale entry
        if (u == dst) break;                 // early exit
        for (const Edge& e : d.adj[u]) {
            if (du + e.weight < dist[e.to]) {
                dist[e.to] = du + e.weight;
                parent[e.to] = u;
                pq.push({dist[e.to], e.to});
            }
        }
    }

    PathResult r;
    if (dist[dst] == INF) return r;
    r.reachable = true;
    r.distance = dist[dst];
    r.path = buildPath(parent, src, dst);
    return r;
}

std::vector<int> distancesFrom(const CampusData& d, int src) {
    const int INF = std::numeric_limits<int>::max();
    std::vector<int> dist(d.locations.size(), INF);
    using P = std::pair<int, int>;
    std::priority_queue<P, std::vector<P>, std::greater<P>> pq;
    dist[src] = 0;
    pq.push({0, src});
    while (!pq.empty()) {
        auto [du, u] = pq.top();
        pq.pop();
        if (du > dist[u]) continue;
        for (const Edge& e : d.adj[u])
            if (du + e.weight < dist[e.to]) {
                dist[e.to] = du + e.weight;
                pq.push({dist[e.to], e.to});
            }
    }
    return dist;
}

PathResult fewestStops(const CampusData& d, int src, int dst) {
    int n = static_cast<int>(d.locations.size());
    std::vector<int> level(n, -1), parent(n, -1);
    std::queue<int> q;
    level[src] = 0;
    q.push(src);
    while (!q.empty()) {
        int u = q.front();
        q.pop();
        for (const Edge& e : d.adj[u]) {
            if (level[e.to] == -1) {
                level[e.to] = level[u] + 1;
                parent[e.to] = u;
                q.push(e.to);
            }
        }
    }
    PathResult r;
    if (level[dst] == -1) return r;
    r.reachable = true;
    r.distance = level[dst];
    r.path = buildPath(parent, src, dst);
    return r;
}

std::vector<int> bfsOrder(const CampusData& d, int src) {
    std::vector<int> order;
    std::vector<bool> seen(d.locations.size(), false);
    std::queue<int> q;
    seen[src] = true;
    q.push(src);
    while (!q.empty()) {
        int u = q.front();
        q.pop();
        order.push_back(u);
        for (const Edge& e : d.adj[u]) {
            if (!seen[e.to]) {
                seen[e.to] = true;
                q.push(e.to);
            }
        }
    }
    return order;
}

std::vector<int> dfsOrder(const CampusData& d, int src) {
    std::vector<int> order;
    std::vector<bool> seen(d.locations.size(), false);
    std::function<void(int)> go = [&](int u) {
        seen[u] = true;
        order.push_back(u);
        for (const Edge& e : d.adj[u])
            if (!seen[e.to]) go(e.to);
    };
    go(src);
    return order;
}

}  // namespace campus
