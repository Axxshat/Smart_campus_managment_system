// exam_timetable.h  --  Module 6 (part of Module 5 in the report): Exam Timetable Scheduling
// Concepts: Graph colouring (greedy, Welsh-Powell order) with seat-capacity limit,
//           exact backtracking colouring used to validate the greedy result on small inputs
#pragma once
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include <set>

namespace campus {

struct Timetable {
    std::vector<std::string> subjects;
    std::vector<int> slotOf;       // slotOf[i] = slot number (0-based) of subjects[i]
    std::vector<int> slotLoad;     // students writing in each slot
    int slots = 0;
    bool capacityRespected = true; // false if a single subject is larger than the capacity
};

class ExamTimetable {
public:
    bool loadConflicts(const std::string& path);      // "SUBJ1 SUBJ2" pairs
    bool loadEnrolments(const std::string& path);     // "roll|S1,S2,..." -> builds conflict graph

    // capacity = seats available in one slot (0 = unlimited)
    Timetable schedule(int capacity = 0) const;       // O(V log V + V*deg)
    bool valid(const Timetable& t) const;             // no two conflicting subjects share a slot
    int optimalSlots() const;                         // exact minimum, -1 if more than 16 subjects

    size_t subjectCount() const { return names_.size(); }
    size_t conflictCount() const { return edgeSet_.size(); }
    size_t studentCount() const { return enrol_.size(); }
    int enrolled(size_t subject) const { return counts_[subject]; }
    const std::vector<std::string>& subjects() const { return names_; }

    // exams of one student with their slots, sorted by slot
    std::vector<std::pair<std::string, int>> examsOf(const std::string& roll, const Timetable& t) const;

private:
    std::vector<std::string> names_;
    std::vector<std::vector<int>> adj_;
    std::vector<int> counts_;
    std::unordered_map<std::string, int> index_;
    std::unordered_map<std::string, std::vector<int>> enrol_;
    std::set<std::pair<int, int>> edgeSet_;

    int idOf(const std::string& name);
    void addEdge(int u, int v);
    bool canColour(int k, const std::vector<int>& order, size_t pos, std::vector<int>& col) const;
};

// total seats in rooms.txt  ("room|seats")
int totalSeats(const std::string& path);

}  // namespace campus
