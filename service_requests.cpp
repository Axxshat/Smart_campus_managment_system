#include "service_requests.h"

#include <algorithm>
#include <fstream>

#include "campus_data.h"

namespace campus {

bool RequestHeap::higher(const Request& a, const Request& b) {
    if (a.priority != b.priority) return a.priority < b.priority;
    return a.seq < b.seq;                         // first come, first served among equals
}

void RequestHeap::swapNodes(size_t i, size_t j) {
    std::swap(heap_[i], heap_[j]);
    pos_[heap_[i].id] = i;
    pos_[heap_[j].id] = j;
}

void RequestHeap::siftUp(size_t i) {
    while (i > 0) {
        size_t p = (i - 1) / 2;
        if (!higher(heap_[i], heap_[p])) break;
        swapNodes(i, p);
        i = p;
    }
}

void RequestHeap::siftDown(size_t i) {
    size_t n = heap_.size();
    while (true) {
        size_t l = 2 * i + 1, r = 2 * i + 2, best = i;
        if (l < n && higher(heap_[l], heap_[best])) best = l;
        if (r < n && higher(heap_[r], heap_[best])) best = r;
        if (best == i) break;
        swapNodes(i, best);
        i = best;
    }
}

void RequestHeap::pushNode(const Request& r) {
    heap_.push_back(r);
    pos_[r.id] = heap_.size() - 1;
    siftUp(heap_.size() - 1);
}

int RequestHeap::submit(int priority, const std::string& location, const std::string& description,
                        const std::string& category) {
    Request r;
    r.id = nextId_++;
    r.priority = std::max(1, std::min(5, priority));
    r.location = location;
    r.category = category;
    r.description = description;
    r.seq = r.id;
    r.createdAt = clock_;
    pushNode(r);
    return r.id;
}

bool RequestHeap::peek(Request& out) const {
    if (heap_.empty()) return false;
    out = heap_.front();
    return true;
}

bool RequestHeap::processNext(Request& out) {
    if (heap_.empty()) return false;
    out = heap_.front();
    pos_.erase(out.id);
    if (heap_.size() > 1) {
        heap_.front() = heap_.back();
        pos_[heap_.front().id] = 0;
        heap_.pop_back();
        siftDown(0);
    } else {
        heap_.pop_back();
    }
    return true;
}

bool RequestHeap::updatePriority(int id, int newPriority) {
    auto it = pos_.find(id);
    if (it == pos_.end()) return false;
    size_t i = it->second;
    heap_[i].priority = std::max(1, std::min(5, newPriority));
    siftUp(i);
    siftDown(pos_[id]);                           // priority may have moved either way
    return true;
}

int RequestHeap::escalateOlderThan(int maxWait) {
    std::vector<std::pair<int, int>> todo;        // (id, new priority)
    for (const Request& r : heap_)
        if (clock_ - r.createdAt >= maxWait && r.priority > 1) todo.push_back({r.id, r.priority - 1});
    for (auto& t : todo) {
        updatePriority(t.first, t.second);
        heap_[pos_[t.first]].createdAt = clock_;  // restart the waiting timer
    }
    return static_cast<int>(todo.size());
}

bool RequestHeap::saveToFile(const std::string& path) const {
    std::ofstream out(path);
    if (!out) return false;
    out << "#state|" << nextId_ << "|" << clock_ << "\n";
    for (const Request& r : heap_) {
        std::string d = r.description;
        std::replace(d.begin(), d.end(), '|', '/');
        out << r.id << '|' << r.priority << '|' << r.location << '|' << r.category << '|'
            << r.createdAt << '|' << d << "\n";
    }
    return true;
}

bool RequestHeap::loadFromFile(const std::string& path) {
    std::ifstream in(path);
    if (!in) return false;
    heap_.clear();
    pos_.clear();
    std::string line;
    int maxId = 0;
    while (std::getline(in, line)) {
        line = trim(line);
        if (line.empty()) continue;
        if (line[0] == '#') {
            auto p = split(line, '|');
            if (p.size() == 3) { nextId_ = std::stoi(p[1]); clock_ = std::stoi(p[2]); }
            continue;
        }
        auto p = split(line, '|');
        if (p.size() < 6) continue;
        Request r;
        r.id = std::stoi(p[0]);
        r.priority = std::stoi(p[1]);
        r.location = p[2];
        r.category = p[3];
        r.createdAt = std::stoi(p[4]);
        r.description = p[5];
        r.seq = r.id;
        maxId = std::max(maxId, r.id);
        pushNode(r);
    }
    nextId_ = std::max(nextId_, maxId + 1);
    return true;
}

}  // namespace campus
