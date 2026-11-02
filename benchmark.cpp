// benchmark.cpp  --  compares the data structures used in the project with simpler alternatives
//   make bench      (results are also written to docs/benchmark_results.csv)
//
// 1. Student lookup : linear scan  vs  binary search on sorted array  vs  hash table (chaining)
// 2. Request queue  : sorted array (insert in place)  vs  binary min-heap
// 3. Shortest paths : Dijkstra with O(V^2) array scan  vs  Dijkstra with min-heap
// Every measurement is repeated 3 times and the fastest run is reported.

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <functional>
#include <iomanip>
#include <iostream>
#include <limits>
#include <random>
#include <string>
#include <vector>

#include "campus_data.h"
#include "navigation.h"
#include "service_requests.h"
#include "student_records.h"

using namespace campus;
using Clock = std::chrono::steady_clock;

static volatile long long sink = 0;   // stops the compiler from removing the work

static double bestOf3(const std::function<void()>& fn) {
    double best = 1e18;
    for (int i = 0; i < 3; ++i) {
        auto t0 = Clock::now();
        fn();
        double ms = std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
        best = std::min(best, ms);
    }
    return best;
}

static std::string rollOf(int i) {
    char buf[32];
    std::snprintf(buf, sizeof buf, "25013301%06d", i);
    return buf;
}

int main() {
    std::mt19937 rng(42);
    FILE* csv = std::fopen("docs/benchmark_results.csv", "w");
    if (csv) std::fputs("test,size,method,time_ms\n", csv);
    auto row = [&](const char* test, long n, const char* method, double ms) {
        if (csv) std::fprintf(csv, "%s,%ld,%s,%.4f\n", test, n, method, ms);
    };

    // ---------------- 1. student lookup ----------------
    std::cout << "1) Student lookup - total time for 2000 successful searches (ms)\n";
    std::cout << std::setw(10) << "records" << std::setw(14) << "linear" << std::setw(14) << "binary" << std::setw(14) << "hash" << "\n";
    const int Q = 2000;
    struct Cfg { int n; size_t buckets; };
    for (Cfg c : {Cfg{1000, 1009}, Cfg{10000, 10007}, Cfg{100000, 100003}}) {
        std::vector<Student> arr;
        for (int i = 0; i < c.n; ++i) arr.push_back(Student{rollOf(i), "name", "CSE", 2, 8.0});
        std::shuffle(arr.begin(), arr.end(), rng);
        StudentTable table(c.buckets);
        for (const Student& s : arr) table.insert(s);
        std::vector<Student> sorted = arr;
        std::sort(sorted.begin(), sorted.end(), [](const Student& a, const Student& b) { return a.roll < b.roll; });

        std::vector<std::string> keys;
        for (int i = 0; i < Q; ++i) keys.push_back(rollOf(static_cast<int>(rng() % c.n)));

        double tl = bestOf3([&] {
            long long f = 0;
            for (const auto& k : keys)
                for (const Student& s : arr)
                    if (s.roll == k) { ++f; break; }
            sink += f;
        });
        double tb = bestOf3([&] {
            long long f = 0;
            for (const auto& k : keys) {
                auto it = std::lower_bound(sorted.begin(), sorted.end(), k,
                                           [](const Student& s, const std::string& key) { return s.roll < key; });
                if (it != sorted.end() && it->roll == k) ++f;
            }
            sink += f;
        });
        double th = bestOf3([&] {
            long long f = 0;
            for (const auto& k : keys) f += (table.search(k) != nullptr);
            sink += f;
        });
        std::cout << std::setw(10) << c.n << std::fixed << std::setprecision(3) << std::setw(14) << tl
                  << std::setw(14) << tb << std::setw(14) << th << "\n";
        row("student_lookup", c.n, "linear", tl);
        row("student_lookup", c.n, "binary", tb);
        row("student_lookup", c.n, "hash", th);
    }

    // ---------------- 2. request queue ----------------
    std::cout << "\n2) Request queue - insert n requests then serve all in priority order (ms)\n";
    std::cout << std::setw(10) << "requests" << std::setw(18) << "sorted array" << std::setw(14) << "min-heap" << "\n";
    for (int n : {2000, 10000, 20000}) {
        std::vector<int> prio(n);
        for (int& p : prio) p = 1 + static_cast<int>(rng() % 5);

        double ta = bestOf3([&] {
            std::vector<Request> v;
            int seq = 0;
            auto less = [](const Request& a, const Request& b) {
                return a.priority != b.priority ? a.priority < b.priority : a.seq < b.seq;
            };
            for (int p : prio) {
                Request r;
                r.priority = p;
                r.seq = ++seq;
                v.insert(std::upper_bound(v.begin(), v.end(), r, less), r);   // O(n) shift
            }
            long long s = 0;
            for (const Request& r : v) s += r.priority;                        // serve in order
            sink += s;
        });
        double th = bestOf3([&] {
            RequestHeap h;
            for (int p : prio) h.submit(p, "Library", "x");
            Request r;
            long long s = 0;
            while (h.processNext(r)) s += r.priority;
            sink += s;
        });
        std::cout << std::setw(10) << n << std::setw(18) << ta << std::setw(14) << th << "\n";
        row("request_queue", n, "sorted_array", ta);
        row("request_queue", n, "min_heap", th);
    }

    // ---------------- 3. shortest paths ----------------
    std::cout << "\n3) Single-source shortest paths on a random sparse graph, E = 4V (ms for 10 sources)\n";
    std::cout << std::setw(10) << "V" << std::setw(18) << "array O(V^2)" << std::setw(18) << "heap O(E log V)" << "\n";
    for (int V : {1000, 3000, 6000}) {
        CampusData g;
        g.locations.resize(V);
        g.adj.assign(V, {});
        for (int i = 1; i < V; ++i) {                       // random tree keeps the graph connected
            int j = static_cast<int>(rng() % i), w = 1 + static_cast<int>(rng() % 100);
            g.adj[i].push_back({j, w});
            g.adj[j].push_back({i, w});
        }
        for (int e = 0; e < 3 * V; ++e) {
            int a = static_cast<int>(rng() % V), b = static_cast<int>(rng() % V), w = 1 + static_cast<int>(rng() % 100);
            if (a == b) continue;
            g.adj[a].push_back({b, w});
            g.adj[b].push_back({a, w});
        }
        std::vector<int> sources;
        for (int i = 0; i < 10; ++i) sources.push_back(static_cast<int>(rng() % V));

        double ta = bestOf3([&] {
            const int INF = std::numeric_limits<int>::max();
            long long s = 0;
            for (int src : sources) {
                std::vector<int> dist(V, INF);
                std::vector<bool> done(V, false);
                dist[src] = 0;
                for (int it = 0; it < V; ++it) {
                    int u = -1;
                    for (int v = 0; v < V; ++v)             // linear scan for the minimum
                        if (!done[v] && dist[v] != INF && (u == -1 || dist[v] < dist[u])) u = v;
                    if (u == -1) break;
                    done[u] = true;
                    for (const Edge& e : g.adj[u])
                        if (dist[u] + e.weight < dist[e.to]) dist[e.to] = dist[u] + e.weight;
                }
                s += dist[V - 1];
            }
            sink += s;
        });
        double th = bestOf3([&] {
            long long s = 0;
            for (int src : sources) s += distancesFrom(g, src)[V - 1];
            sink += s;
        });
        std::cout << std::setw(10) << V << std::setw(18) << ta << std::setw(18) << th << "\n";
        row("dijkstra", V, "array", ta);
        row("dijkstra", V, "heap", th);
    }

    if (csv) std::fclose(csv);
    std::cout << "\nSaved docs/benchmark_results.csv\n";
    return 0;
}
