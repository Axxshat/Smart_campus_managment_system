// student_records.h  --  Module 4: Student Record Management
// Concepts: Hash table with separate chaining (own implementation) + Trie for name-prefix search
#pragma once
#include <functional>
#include <list>
#include <string>
#include <vector>

#include "trie.h"

namespace campus {

struct Student {
    std::string roll;
    std::string name;
    std::string branch;
    int year = 0;
    double cgpa = 0.0;
};

class StudentTable {
public:
    explicit StudentTable(size_t buckets = 101);   // prime size keeps the spread even

    bool insert(const Student& s);                 // false if roll no. already exists
    const Student* search(const std::string& roll) const;
    bool remove(const std::string& roll);

    size_t size() const { return count_; }
    size_t bucketCount() const { return table_.size(); }
    double loadFactor() const { return static_cast<double>(count_) / table_.size(); }
    size_t longestChain() const;

    void forEach(const std::function<void(const Student&)>& fn) const;
    bool loadFromFile(const std::string& path);

private:
    std::vector<std::list<Student>> table_;
    size_t count_ = 0;
    size_t hash(const std::string& key) const;     // polynomial rolling hash
};

// Hash table (lookup by roll no.) + Trie (search by name prefix) kept in sync.
class StudentDirectory {
public:
    explicit StudentDirectory(size_t buckets = 101) : table_(buckets) {}

    bool insert(const Student& s);
    const Student* find(const std::string& roll) const { return table_.search(roll); }
    bool remove(const std::string& roll) { return table_.remove(roll); }   // trie entry cleaned lazily
    std::vector<const Student*> searchByNamePrefix(const std::string& prefix, size_t limit = 10) const;
    bool loadFromFile(const std::string& path);

    size_t size() const { return table_.size(); }
    const StudentTable& table() const { return table_; }
    void forEach(const std::function<void(const Student&)>& fn) const { table_.forEach(fn); }

private:
    StudentTable table_;
    Trie names_;
};

}  // namespace campus
