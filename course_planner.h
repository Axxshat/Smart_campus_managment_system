// course_planner.h  --  Module 3: Course Prerequisite Planner
// Concepts: DAG, Topological Sort (Kahn's algorithm), cycle detection
#pragma once
#include <vector>
#include "campus_data.h"

namespace campus {

struct PlanResult {
    bool valid = true;                          // false => circular prerequisites
    std::vector<std::vector<int>> semesters;    // semesters[i] = courses that can be taken in semester i+1
    std::vector<int> stuck;                     // courses that are part of / blocked by a cycle
};

// Kahn's algorithm processed level by level, O(V + E)
// Cycle exists when processed count < V.
PlanResult planCourses(const CampusData& d);

// single flat valid order (same algorithm, one queue)
std::vector<int> flatOrder(const CampusData& d);

}  // namespace campus
