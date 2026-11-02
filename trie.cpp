#include "trie.h"

#include <cctype>

namespace campus {

Trie::Trie() : root_(new Node()) {}

std::string Trie::lower(const std::string& s) {
    std::string r = s;
    for (char& c : r) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return r;
}

void Trie::insert(const std::string& word, const std::string& payload) {
    Node* cur = root_.get();
    for (char c : lower(word)) {
        auto& slot = cur->next[c];
        if (!slot) slot.reset(new Node());
        cur = slot.get();
    }
    if (cur->payloads.empty()) ++words_;
    cur->payloads.push_back(payload);
}

void Trie::collect(const Node* n, std::vector<std::string>& out, size_t limit) const {
    if (out.size() >= limit) return;
    for (const auto& p : n->payloads) out.push_back(p);
    for (const auto& kv : n->next) {
        if (out.size() >= limit) return;
        collect(kv.second.get(), out, limit);
    }
}

std::vector<std::string> Trie::startsWith(const std::string& prefix, size_t limit) const {
    const Node* cur = root_.get();
    for (char c : lower(prefix)) {
        auto it = cur->next.find(c);
        if (it == cur->next.end()) return {};
        cur = it->second.get();
    }
    std::vector<std::string> out;
    collect(cur, out, limit);
    return out;
}

}  // namespace campus
