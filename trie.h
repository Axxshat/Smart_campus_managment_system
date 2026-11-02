// trie.h  --  prefix tree used for name search in the Student Records module
#pragma once
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace campus {

class Trie {
public:
    Trie();
    // stores `payload` under `word` (case-insensitive)
    void insert(const std::string& word, const std::string& payload);
    // payloads of every word that starts with `prefix`, in alphabetical word order
    std::vector<std::string> startsWith(const std::string& prefix, size_t limit = 50) const;
    size_t wordCount() const { return words_; }

private:
    struct Node {
        std::map<char, std::unique_ptr<Node>> next;
        std::vector<std::string> payloads;
    };
    std::unique_ptr<Node> root_;
    size_t words_ = 0;
    static std::string lower(const std::string& s);
    void collect(const Node* n, std::vector<std::string>& out, size_t limit) const;
};

}  // namespace campus
