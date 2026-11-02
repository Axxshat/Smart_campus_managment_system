#include "course_planner.h"

#include <queue>

namespace campus {

PlanResult planCourses(const CampusData& d) {
    PlanResult res;
    int n = static_cast<int>(d.courseCodes.size());
    std::vector<int> indeg = d.prereqCount;

    std::vector<int> current;
    for (int i = 0; i < n; ++i)
        if (indeg[i] == 0) current.push_back(i);

    int processed = 0;
    while (!current.empty()) {
        res.semesters.push_back(current);
        std::vector<int> next;
        for (int u : current) {
            ++processed;
            for (int v : d.unlocks[u])
                if (--indeg[v] == 0) next.push_back(v);
        }
        current = next;
    }

    if (processed < n) {
        res.valid = false;
        for (int i = 0; i < n; ++i)
            if (indeg[i] > 0) res.stuck.push_back(i);
    }
    return res;
}

std::vector<int> flatOrder(const CampusData& d) {
    std::vector<int> order;
    PlanResult r = planCourses(d);
    for (const auto& sem : r.semesters)
        for (int c : sem) order.push_back(c);
    return order;
}

}  // namespace campus
