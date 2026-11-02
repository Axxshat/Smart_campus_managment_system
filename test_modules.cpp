// Self-checking tests:  make test
#include <algorithm>
#include <cstdio>
#include <iostream>

#include "campus_data.h"
#include "course_planner.h"
#include "exam_timetable.h"
#include "integration.h"
#include "navigation.h"
#include "network_planning.h"
#include "service_requests.h"
#include "student_records.h"
#include "trie.h"

using namespace campus;

static int failed = 0, total = 0;
#define CHECK(cond)                                                                           \
    do {                                                                                      \
        ++total;                                                                              \
        if (!(cond)) { ++failed; std::cout << "FAIL line " << __LINE__ << ": " #cond "\n"; } \
    } while (0)

int main() {
    CampusData d;
    CHECK(d.loadMap("data/campus_map.txt"));
    CHECK(d.loadCourses("data/courses.txt"));

    // ---------- Module 1: navigation ----------
    int gate = d.locationId("Main_Gate"), lab = d.locationId("Computer_Lab"), hostel = d.locationId("Hostel");
    PathResult p = shortestPath(d, gate, lab);
    CHECK(p.reachable);
    CHECK(p.distance == 150 + 120 + 90 + 70);                    // Gate-Admin-BlockA-BlockB-Lab
    CHECK(p.path.front() == gate && p.path.back() == lab);
    CHECK(shortestPath(d, gate, gate).distance == 0);
    CHECK(fewestStops(d, gate, lab).distance >= 1);
    CHECK(bfsOrder(d, gate).size() == d.locations.size());
    CHECK(dfsOrder(d, gate).size() == d.locations.size());
    CHECK(distancesFrom(d, gate)[lab] == p.distance);            // multi-target Dijkstra agrees
    CampusData iso = d;                                          // add an unreachable location
    iso.locations.push_back("Island");
    iso.adj.emplace_back();
    CHECK(!shortestPath(iso, gate, static_cast<int>(iso.locations.size()) - 1).reachable);

    // ---------- Module 2: network planning ----------
    MSTResult m = planNetwork(d);
    CHECK(m.connected);
    CHECK(m.chosen.size() == d.locations.size() - 1);
    CHECK(m.totalLength == 1115);
    DisjointSet dsu(4);
    CHECK(dsu.unite(0, 1) && dsu.unite(2, 3) && !dsu.unite(1, 0));
    CHECK(dsu.find(0) != dsu.find(2));
    dsu.unite(1, 2);
    CHECK(dsu.find(0) == dsu.find(3));

    // ---------- Module 3: course planner ----------
    PlanResult cp = planCourses(d);
    CHECK(cp.valid);
    int placed = 0;
    for (auto& s : cp.semesters) placed += static_cast<int>(s.size());
    CHECK(placed == static_cast<int>(d.courseCodes.size()));
    CHECK(cp.semesters.size() == 4);
    auto semOf = [&](const char* code) {
        int id = d.courseId(code);
        for (size_t s = 0; s < cp.semesters.size(); ++s)
            if (std::find(cp.semesters[s].begin(), cp.semesters[s].end(), id) != cp.semesters[s].end()) return static_cast<int>(s);
        return -1;
    };
    CHECK(semOf("CS101") < semOf("CS201"));                      // prerequisite always earlier
    CHECK(semOf("CS301") < semOf("CS401"));
    CampusData cyc;
    cyc.courseCodes = {"A", "B"};
    cyc.courseNames = {"A", "B"};
    cyc.unlocks = {{1}, {0}};
    cyc.prereqCount = {1, 1};
    CHECK(!planCourses(cyc).valid);
    CHECK(planCourses(cyc).stuck.size() == 2);

    // ---------- Module 4: hash table + trie ----------
    StudentTable t(7);                                           // tiny table on purpose -> collisions
    Student a{"R1", "A", "CSE", 2, 8.0}, b{"R2", "B", "CSE", 2, 8.5};
    CHECK(t.insert(a));
    CHECK(t.insert(b));
    CHECK(!t.insert(a));
    CHECK(t.search("R2") != nullptr);
    CHECK(t.remove("R1"));
    CHECK(t.search("R1") == nullptr);
    CHECK(!t.remove("R1"));
    CHECK(t.size() == 1);
    StudentTable many(5);
    for (int i = 0; i < 200; ++i) many.insert(Student{"X" + std::to_string(i), "n", "b", 1, 7.0});
    CHECK(many.size() == 200);
    bool allFound = true;
    for (int i = 0; i < 200; ++i) allFound = allFound && many.search("X" + std::to_string(i));
    CHECK(allFound);

    Trie tr;
    tr.insert("Ananya", "r1");
    tr.insert("Anand", "r2");
    tr.insert("Rohan", "r3");
    CHECK(tr.startsWith("an").size() == 2);
    CHECK(tr.startsWith("AN").size() == 2);                      // case-insensitive
    CHECK(tr.startsWith("z").empty());
    CHECK(tr.startsWith("").size() == 3);

    StudentDirectory dir;
    CHECK(dir.loadFromFile("data/students.txt"));
    CHECK(dir.size() == 40);
    CHECK(dir.find("2501330100004") != nullptr);
    auto byName = dir.searchByNamePrefix("ana");
    CHECK(!byName.empty());
    bool prefixOk = true;
    for (auto* s : byName) {
        std::string n = s->name;
        for (auto& c : n) c = static_cast<char>(std::tolower(c));
        prefixOk = prefixOk && (n.find("ana") == 0 || n.find(" ana") != std::string::npos);
    }
    CHECK(prefixOk);
    CHECK(dir.remove("2501330100004"));
    bool stillListed = false;
    for (auto* s : dir.searchByNamePrefix("ananya")) stillListed = stillListed || s->roll == "2501330100004";
    CHECK(!stillListed);                                         // deleted students vanish from prefix search

    // ---------- Module 5: service requests ----------
    RequestHeap h;
    int r1 = h.submit(3, "Library", "AC not working", "electrical");
    int r2 = h.submit(1, "Hostel", "Water leakage", "plumbing");
    int r3 = h.submit(3, "Computer_Lab", "Projector issue", "it_support");
    Request r;
    CHECK(h.peek(r) && r.id == r2);
    CHECK(h.updatePriority(r3, 2));                              // r3 jumps ahead of r1
    CHECK(!h.updatePriority(999, 1));
    CHECK(h.processNext(r) && r.id == r2);
    CHECK(h.processNext(r) && r.id == r3);
    CHECK(h.processNext(r) && r.id == r1);
    CHECK(!h.processNext(r));
    // FIFO among equal priority
    RequestHeap fifo;
    for (int i = 0; i < 20; ++i) fifo.submit(3, "Library", "req" + std::to_string(i));
    bool inOrder = true;
    for (int i = 0; i < 20; ++i) { fifo.processNext(r); inOrder = inOrder && r.description == "req" + std::to_string(i); }
    CHECK(inOrder);
    // heap order is always sorted by priority when drained
    RequestHeap mix;
    int prios[] = {5, 2, 4, 1, 3, 2, 5, 1, 4, 3};
    for (int pr : prios) mix.submit(pr, "Library", "x");
    int last = 0;
    bool sorted = true;
    while (mix.processNext(r)) { sorted = sorted && r.priority >= last; last = r.priority; }
    CHECK(sorted);
    // escalation
    RequestHeap esc;
    int low = esc.submit(4, "Hostel", "slow wifi", "it_support");
    esc.advanceTime(10);
    int fresh = esc.submit(4, "Library", "bulb", "electrical");
    CHECK(esc.escalateOlderThan(10) == 1);                       // only the old one is escalated
    CHECK(esc.peek(r) && r.id == low && r.priority == 3);
    (void)fresh;
    // save / load round trip
    CHECK(esc.saveToFile("build/test_requests.txt"));
    RequestHeap loaded;
    CHECK(loaded.loadFromFile("build/test_requests.txt"));
    CHECK(loaded.size() == 2 && loaded.now() == 10);
    CHECK(loaded.peek(r) && r.id == low);
    std::remove("build/test_requests.txt");

    // ---------- Module 5 (cont.): exam timetable ----------
    ExamTimetable et;
    CHECK(et.loadEnrolments("data/enrolments.txt"));
    CHECK(et.subjectCount() == 11);
    CHECK(et.studentCount() == 40);
    Timetable tt = et.schedule();
    CHECK(et.valid(tt));
    CHECK(tt.slots >= et.optimalSlots());                        // greedy never beats the optimum
    CHECK(et.optimalSlots() >= 4);                               // year-2 cohort writes 4 mutually conflicting papers
    int seats = totalSeats("data/rooms.txt");
    CHECK(seats == 90);
    int biggest = 0;
    for (size_t i = 0; i < et.subjectCount(); ++i) biggest = std::max(biggest, et.enrolled(i));
    Timetable capped = et.schedule(biggest);                     // every single subject fits in one slot
    CHECK(et.valid(capped));
    CHECK(capped.capacityRespected);
    CHECK(capped.slots >= tt.slots);                             // capacity can only add slots
    bool capOk = true;
    for (int load : capped.slotLoad) capOk = capOk && load <= biggest;
    CHECK(capOk);
    CHECK(!et.schedule(biggest - 1).capacityRespected);          // a class larger than the hall is flagged
    auto mine = et.examsOf("2501330100001", tt);
    CHECK(mine.size() >= 4);
    for (size_t i = 1; i < mine.size(); ++i) CHECK(mine[i - 1].second != mine[i].second);   // no two exams in one slot
    CHECK(et.examsOf("no-such-roll", tt).empty());
    // greedy is optimal on a simple path graph and on a triangle
    ExamTimetable tri;
    std::remove("build/tri.txt");
    {
        FILE* f = std::fopen("build/tri.txt", "w");
        std::fputs("A B\nB C\nA C\nC D\n", f);
        std::fclose(f);
    }
    CHECK(tri.loadConflicts("build/tri.txt"));
    CHECK(tri.optimalSlots() == 3);
    CHECK(tri.schedule().slots == 3);
    std::remove("build/tri.txt");

    // ---------- Integration ----------
    auto staff = loadStaff("data/staff.txt", d);
    CHECK(staff.size() == 7);
    Request job;
    job.category = "electrical";
    job.location = "Hostel";
    Dispatch dp = dispatchNearest(d, staff, job);
    CHECK(dp.found && dp.staff.name == "Suresh" && dp.distance == 0);   // Suresh is already at the Hostel
    job.location = "Computer_Lab";
    dp = dispatchNearest(d, staff, job);
    CHECK(dp.found && dp.staff.name == "Ramesh");
    CHECK(dp.path.front() == dp.staff.location && dp.path.back() == d.locationId("Computer_Lab"));
    job.category = "plumbing";
    job.location = "Nowhere";
    CHECK(!dispatchNearest(d, staff, job).found);                // unknown location
    job.location = "Library";
    job.category = "astrology";
    CHECK(!dispatchNearest(d, staff, job).found);                // no staff with that role
    (void)hostel;

    std::cout << (total - failed) << "/" << total << " checks passed\n";
    return failed ? 1 : 0;
}
