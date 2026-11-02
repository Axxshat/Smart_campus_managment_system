#include "exam_timetable.h"

#include <algorithm>
#include <fstream>
#include <sstream>

#include "campus_data.h"

namespace campus {

int ExamTimetable::idOf(const std::string& name) {
    auto it = index_.find(name);
    if (it != index_.end()) return it->second;
    int id = static_cast<int>(names_.size());
    index_[name] = id;
    names_.push_back(name);
    adj_.emplace_back();
    counts_.push_back(0);
    return id;
}

void ExamTimetable::addEdge(int u, int v) {
    if (u == v) return;
    auto key = std::make_pair(std::min(u, v), std::max(u, v));
    if (!edgeSet_.insert(key).second) return;     // conflict already known
    adj_[u].push_back(v);
    adj_[v].push_back(u);
}

bool ExamTimetable::loadConflicts(const std::string& path) {
    std::ifstream in(path);
    if (!in) return false;
    std::string line;
    while (std::getline(in, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#') continue;
        std::istringstream ss(line);
        std::string a, b;
        if (!(ss >> a >> b)) continue;
        addEdge(idOf(a), idOf(b));
    }
    return !names_.empty();
}

bool ExamTimetable::loadEnrolments(const std::string& path) {
    std::ifstream in(path);
    if (!in) return false;
    std::string line;
    while (std::getline(in, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#') continue;
        auto parts = split(line, '|');
        if (parts.size() != 2) continue;
        std::vector<int> ids;
        for (const auto& s : split(parts[1], ',')) {
            if (s.empty()) continue;
            int id = idOf(s);
            ids.push_back(id);
            counts_[id]++;
        }
        // two subjects of the same student can never share a slot
        for (size_t i = 0; i < ids.size(); ++i)
            for (size_t j = i + 1; j < ids.size(); ++j) addEdge(ids[i], ids[j]);
        enrol_[parts[0]] = ids;
    }
    return !enrol_.empty();
}

Timetable ExamTimetable::schedule(int capacity) const {
    int n = static_cast<int>(names_.size());
    Timetable t;
    t.subjects = names_;
    t.slotOf.assign(n, -1);

    // Welsh-Powell: highest degree first (ties: larger class first)
    std::vector<int> order(n);
    for (int i = 0; i < n; ++i) order[i] = i;
    std::sort(order.begin(), order.end(), [&](int a, int b) {
        if (adj_[a].size() != adj_[b].size()) return adj_[a].size() > adj_[b].size();
        return counts_[a] > counts_[b];
    });

    std::vector<int> load;
    for (int u : order) {
        std::vector<bool> used(n + 1, false);
        for (int v : adj_[u])
            if (t.slotOf[v] != -1) used[t.slotOf[v]] = true;
        int c = 0;
        while (true) {
            if (!used[c]) {
                int cur = (c < static_cast<int>(load.size())) ? load[c] : 0;
                bool fits = (capacity <= 0) || (cur + counts_[u] <= capacity) || cur == 0;
                if (fits) break;
            }
            ++c;
        }
        if (c >= static_cast<int>(load.size())) load.resize(c + 1, 0);
        t.slotOf[u] = c;
        load[c] += counts_[u];
        if (capacity > 0 && counts_[u] > capacity) t.capacityRespected = false;
        t.slots = std::max(t.slots, c + 1);
    }
    t.slotLoad = load;
    return t;
}

bool ExamTimetable::valid(const Timetable& t) const {
    for (size_t u = 0; u < adj_.size(); ++u)
        for (int v : adj_[u])
            if (t.slotOf[u] == t.slotOf[v]) return false;
    return true;
}

bool ExamTimetable::canColour(int k, const std::vector<int>& order, size_t pos, std::vector<int>& col) const {
    if (pos == order.size()) return true;
    int u = order[pos];
    int maxUsed = -1;
    for (int c : col) maxUsed = std::max(maxUsed, c);
    for (int c = 0; c < k && c <= maxUsed + 1; ++c) {     // c <= maxUsed+1 removes symmetric branches
        bool ok = true;
        for (int v : adj_[u])
            if (col[v] == c) { ok = false; break; }
        if (!ok) continue;
        col[u] = c;
        if (canColour(k, order, pos + 1, col)) return true;
        col[u] = -1;
    }
    return false;
}

int ExamTimetable::optimalSlots() const {
    int n = static_cast<int>(names_.size());
    if (n == 0) return 0;
    if (n > 16) return -1;
    std::vector<int> order(n);
    for (int i = 0; i < n; ++i) order[i] = i;
    std::sort(order.begin(), order.end(), [&](int a, int b) { return adj_[a].size() > adj_[b].size(); });
    for (int k = 1; k <= n; ++k) {
        std::vector<int> col(n, -1);
        if (canColour(k, order, 0, col)) return k;
    }
    return n;
}

std::vector<std::pair<std::string, int>> ExamTimetable::examsOf(const std::string& roll, const Timetable& t) const {
    std::vector<std::pair<std::string, int>> out;
    auto it = enrol_.find(roll);
    if (it == enrol_.end()) return out;
    for (int id : it->second) out.push_back({names_[id], t.slotOf[id]});
    std::sort(out.begin(), out.end(), [](const auto& a, const auto& b) { return a.second < b.second; });
    return out;
}

int totalSeats(const std::string& path) {
    std::ifstream in(path);
    if (!in) return 0;
    int total = 0;
    std::string line;
    while (std::getline(in, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#') continue;
        auto p = split(line, '|');
        if (p.size() == 2) total += std::stoi(p[1]);
    }
    return total;
}

}  // namespace campus
