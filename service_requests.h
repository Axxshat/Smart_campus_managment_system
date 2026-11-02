// service_requests.h  --  Module 5: Service Request Handling
// Concepts: Binary min-heap / priority queue (own implementation) with an index map for decrease-key
//
//   submit            O(log n)      processNext        O(log n)
//   peek              O(1)          updatePriority     O(log n)   (decrease/increase-key)
//   escalateOlderThan O(n + k log n) (k = requests escalated)
#pragma once
#include <string>
#include <unordered_map>
#include <vector>

namespace campus {

struct Request {
    int id = 0;
    int priority = 3;          // 1 = emergency ... 5 = low
    std::string location;      // must match a location name in the campus map
    std::string category;      // electrical / plumbing / it_support / housekeeping / general
    std::string description;
    int seq = 0;               // arrival order, used to break ties
    int createdAt = 0;         // logical clock value when the request (or its last escalation) happened
};

class RequestHeap {
public:
    int submit(int priority, const std::string& location, const std::string& description,
               const std::string& category = "general");
    bool peek(Request& out) const;
    bool processNext(Request& out);               // removes and returns the most urgent request
    bool updatePriority(int id, int newPriority); // false if the id is not pending

    void advanceTime(int ticks) { clock_ += ticks; }
    int now() const { return clock_; }
    int escalateOlderThan(int maxWait);           // returns how many requests were escalated

    size_t size() const { return heap_.size(); }
    bool empty() const { return heap_.empty(); }
    const std::vector<Request>& pending() const { return heap_; }   // heap order, not sorted

    bool saveToFile(const std::string& path) const;
    bool loadFromFile(const std::string& path);

private:
    std::vector<Request> heap_;
    std::unordered_map<int, size_t> pos_;         // request id -> index in heap_
    int nextId_ = 1;
    int clock_ = 0;

    static bool higher(const Request& a, const Request& b);   // a more urgent than b?
    void swapNodes(size_t i, size_t j);
    void siftUp(size_t i);
    void siftDown(size_t i);
    void pushNode(const Request& r);
};

}  // namespace campus
