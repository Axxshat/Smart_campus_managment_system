// Integrated Smart Campus Management System
// DSA-II PBL (CCSE0301) - menu driven console application
//
// Usage:  ./smartcampus [data_directory]      (default: data/)

#include <iomanip>
#include <iostream>
#include <string>

#include "campus_data.h"
#include "course_planner.h"
#include "exam_timetable.h"
#include "integration.h"
#include "navigation.h"
#include "network_planning.h"
#include "service_requests.h"
#include "student_records.h"

using namespace campus;

static std::string dataDir;

static int readInt(const std::string& prompt, int lo, int hi) {
    while (true) {
        std::cout << prompt;
        std::string line;
        if (!std::getline(std::cin, line)) return lo;
        try {
            int v = std::stoi(line);
            if (v >= lo && v <= hi) return v;
        } catch (...) {}
        std::cout << "  Please enter a number between " << lo << " and " << hi << ".\n";
    }
}

static std::string readLine(const std::string& prompt) {
    std::cout << prompt;
    std::string s;
    std::getline(std::cin, s);
    return trim(s);
}

static void printLocations(const CampusData& d) {
    for (size_t i = 0; i < d.locations.size(); ++i)
        std::cout << "  " << std::setw(2) << i + 1 << ". " << d.locations[i] << "\n";
}

static void printRoute(const CampusData& d, const std::vector<int>& path) {
    std::cout << "  Route: ";
    for (size_t i = 0; i < path.size(); ++i)
        std::cout << d.locations[path[i]] << (i + 1 < path.size() ? " -> " : "\n");
}

static void printPath(const CampusData& d, const PathResult& r, const char* unit) {
    if (!r.reachable) {
        std::cout << "  No route found.\n";
        return;
    }
    printRoute(d, r.path);
    std::cout << "  Total: " << r.distance << " " << unit << "\n";
}

// ---------------- Module 1 ----------------
static void navigationMenu(const CampusData& d) {
    std::cout << "\n--- Campus Navigation ---\n";
    printLocations(d);
    int n = static_cast<int>(d.locations.size());
    int a = readInt("From (number): ", 1, n) - 1;
    int b = readInt("To   (number): ", 1, n) - 1;
    int mode = readInt("1) Shortest distance (Dijkstra)  2) Fewest stops (BFS)  3) Explore from source (BFS/DFS): ", 1, 3);
    if (mode == 1) {
        printPath(d, shortestPath(d, a, b), "metres");
    } else if (mode == 2) {
        printPath(d, fewestStops(d, a, b), "stops");
    } else {
        std::cout << "  BFS order: ";
        for (int v : bfsOrder(d, a)) std::cout << d.locations[v] << "  ";
        std::cout << "\n  DFS order: ";
        for (int v : dfsOrder(d, a)) std::cout << d.locations[v] << "  ";
        std::cout << "\n";
    }
}

// ---------------- Module 2 ----------------
static void networkMenu(const CampusData& d) {
    std::cout << "\n--- Infrastructure Network Planning (Kruskal MST) ---\n";
    MSTResult r = planNetwork(d);
    for (const auto& e : r.chosen)
        std::cout << "  " << std::left << std::setw(18) << d.locations[e.u] << " - "
                  << std::setw(18) << d.locations[e.v] << std::right << std::setw(5) << e.weight << " m\n";
    std::cout << "  Minimum total cable length: " << r.totalLength << " m\n";
    if (!r.connected) std::cout << "  Warning: some buildings are not connected to the rest.\n";
}

// ---------------- Module 3 ----------------
static void courseMenu(const CampusData& d) {
    std::cout << "\n--- Course Prerequisite Planner (Topological Sort) ---\n";
    PlanResult r = planCourses(d);
    for (size_t s = 0; s < r.semesters.size(); ++s) {
        std::cout << "  Semester " << s + 1 << ": ";
        for (int c : r.semesters[s]) std::cout << d.courseCodes[c] << "  ";
        std::cout << "\n";
    }
    if (!r.valid) {
        std::cout << "  Circular prerequisites detected! Courses stuck: ";
        for (int c : r.stuck) std::cout << d.courseCodes[c] << " ";
        std::cout << "\n";
    }
}

// ---------------- Module 4 ----------------
static void printStudent(const Student& s) {
    std::cout << "  " << s.roll << " | " << s.name << " | " << s.branch << " | Year " << s.year
              << " | CGPA " << std::fixed << std::setprecision(2) << s.cgpa << "\n";
}

static void studentMenu(StudentDirectory& dir) {
    while (true) {
        std::cout << "\n--- Student Records (Hash Table + Trie) ---\n"
                  << "  1. Search by roll no.\n  2. Search by name prefix\n  3. Add student\n"
                  << "  4. Delete student\n  5. List all\n  6. Table statistics\n  0. Back\n";
        int ch = readInt("Choice: ", 0, 6);
        if (ch == 0) return;
        if (ch == 1) {
            const Student* s = dir.find(readLine("Roll no.: "));
            if (s) printStudent(*s); else std::cout << "  Not found.\n";
        } else if (ch == 2) {
            auto res = dir.searchByNamePrefix(readLine("Name starts with: "));
            if (res.empty()) std::cout << "  No match.\n";
            for (const Student* s : res) printStudent(*s);
        } else if (ch == 3) {
            Student s;
            s.roll = readLine("Roll no.: ");
            s.name = readLine("Name: ");
            s.branch = readLine("Branch: ");
            s.year = readInt("Year (1-4): ", 1, 4);
            s.cgpa = readInt("CGPA x10 (e.g. 85 for 8.5): ", 0, 100) / 10.0;
            std::cout << (dir.insert(s) ? "  Added.\n" : "  Roll no. already exists.\n");
        } else if (ch == 4) {
            std::cout << (dir.remove(readLine("Roll no.: ")) ? "  Deleted.\n" : "  Not found.\n");
        } else if (ch == 5) {
            dir.forEach(printStudent);
        } else {
            const StudentTable& t = dir.table();
            std::cout << "  Records: " << t.size() << "  Buckets: " << t.bucketCount()
                      << "  Load factor: " << std::fixed << std::setprecision(2) << t.loadFactor()
                      << "  Longest chain: " << t.longestChain() << "\n";
        }
    }
}

// ---------------- Module 5 ----------------
static void printRequest(const Request& r) {
    std::cout << "#" << r.id << " [P" << r.priority << "] " << r.category << " @ " << r.location
              << " - " << r.description;
}

static void requestMenu(RequestHeap& h, const std::string& file) {
    while (true) {
        std::cout << "\n--- Service Requests (Min-Heap)  clock=" << h.now() << "  pending=" << h.size() << " ---\n"
                  << "  1. Submit request\n  2. View most urgent\n  3. Process most urgent\n"
                  << "  4. Change priority of a request\n  5. Advance time and escalate waiting requests\n"
                  << "  6. Save pending requests\n  7. Load pending requests\n  0. Back\n";
        int ch = readInt("Choice: ", 0, 7);
        if (ch == 0) return;
        if (ch == 1) {
            int p = readInt("Priority (1 = emergency ... 5 = low): ", 1, 5);
            std::string loc = readLine("Location (name from campus map): ");
            std::string cat = readLine("Category (electrical/plumbing/it_support/housekeeping/general): ");
            std::string desc = readLine("Description: ");
            std::cout << "  Request #" << h.submit(p, loc, desc, cat.empty() ? "general" : cat) << " submitted.\n";
        } else if (ch == 2 || ch == 3) {
            Request r;
            bool ok = (ch == 2) ? h.peek(r) : h.processNext(r);
            if (!ok) { std::cout << "  No pending requests.\n"; continue; }
            std::cout << (ch == 2 ? "  Next: " : "  Processed: ");
            printRequest(r);
            std::cout << "\n";
        } else if (ch == 4) {
            int id = readInt("Request id: ", 1, 1000000);
            int p = readInt("New priority (1-5): ", 1, 5);
            std::cout << (h.updatePriority(id, p) ? "  Updated.\n" : "  No such pending request.\n");
        } else if (ch == 5) {
            int t = readInt("Advance clock by (ticks): ", 1, 1000);
            int w = readInt("Escalate requests waiting at least (ticks): ", 1, 1000);
            h.advanceTime(t);
            std::cout << "  Escalated " << h.escalateOlderThan(w) << " request(s).\n";
        } else if (ch == 6) {
            std::cout << (h.saveToFile(file) ? "  Saved to " + file + "\n" : "  Could not save.\n");
        } else {
            std::cout << (h.loadFromFile(file) ? "  Loaded.\n" : "  Nothing to load.\n");
        }
    }
}

// ---------------- Module 5 (continued): exam timetable ----------------
static void timetableMenu(const ExamTimetable& et, int seats) {
    std::cout << "\n--- Exam Timetable (Graph Colouring) ---\n"
              << "  Subjects: " << et.subjectCount() << "  Students: " << et.studentCount()
              << "  Conflicts: " << et.conflictCount() << "  Seats per slot: " << seats << "\n";
    Timetable t = et.schedule(seats);
    for (int s = 0; s < t.slots; ++s) {
        std::cout << "  Slot " << s + 1 << " (" << t.slotLoad[s] << " students): ";
        for (size_t i = 0; i < t.subjects.size(); ++i)
            if (t.slotOf[i] == s) std::cout << t.subjects[i] << "  ";
        std::cout << "\n";
    }
    int opt = et.optimalSlots();
    std::cout << "  Slots used: " << t.slots << "   Clash free: " << (et.valid(t) ? "yes" : "NO");
    if (opt > 0) std::cout << "   Minimum possible (exact search, no seat limit): " << opt;
    std::cout << "\n";
    std::string roll = readLine("Roll no. to show personal exam slots (blank to skip): ");
    if (!roll.empty()) {
        auto ex = et.examsOf(roll, t);
        if (ex.empty()) std::cout << "  No enrolment found.\n";
        for (auto& e : ex) std::cout << "  " << e.first << " -> Slot " << e.second + 1 << "\n";
    }
}

// ---------------- Integrated services ----------------
static void integratedMenu(const CampusData& d, StudentDirectory& dir, const ExamTimetable& et,
                           RequestHeap& h, const std::vector<Staff>& staff, int seats) {
    while (true) {
        std::cout << "\n--- Integrated Services ---\n"
                  << "  1. Dispatch nearest staff for the most urgent service request\n"
                  << "  2. Student dashboard (record + exam slots + route to exam hall)\n  0. Back\n";
        int ch = readInt("Choice: ", 0, 2);
        if (ch == 0) return;
        if (ch == 1) {
            Request r;
            if (!h.processNext(r)) { std::cout << "  No pending requests.\n"; continue; }
            std::cout << "  Handling: ";
            printRequest(r);
            std::cout << "\n";
            Dispatch dp = dispatchNearest(d, staff, r);
            if (!dp.found) { std::cout << "  No suitable staff / unknown location.\n"; continue; }
            std::cout << "  Assigned: " << dp.staff.name << " (" << dp.staff.role << ") from "
                       << d.locations[dp.staff.location] << ", " << dp.distance << " m away\n";
            printRoute(d, dp.path);
        } else {
            const Student* s = dir.find(readLine("Roll no.: "));
            if (!s) { std::cout << "  Student not found.\n"; continue; }
            printStudent(*s);
            Timetable t = et.schedule(seats);
            for (auto& e : et.examsOf(s->roll, t)) std::cout << "  Exam " << e.first << " -> Slot " << e.second + 1 << "\n";
            int from = d.locationId("Hostel"), hall = d.locationId("Auditorium");
            if (from >= 0 && hall >= 0) {
                PathResult p = shortestPath(d, from, hall);
                std::cout << "  Hostel to Auditorium (exam hall): " << p.distance << " m\n";
                printRoute(d, p.path);
            }
        }
    }
}

int main(int argc, char** argv) {
    dataDir = (argc > 1) ? argv[1] : "data";
    if (!dataDir.empty() && dataDir.back() != '/') dataDir += '/';

    CampusData data;
    StudentDirectory students;
    ExamTimetable exams;
    RequestHeap requests;

    if (!data.loadMap(dataDir + "campus_map.txt")) std::cerr << "Could not load campus_map.txt\n";
    if (!data.loadCourses(dataDir + "courses.txt")) std::cerr << "Could not load courses.txt\n";
    if (!students.loadFromFile(dataDir + "students.txt")) std::cerr << "Could not load students.txt\n";
    if (!exams.loadEnrolments(dataDir + "enrolments.txt")) std::cerr << "Could not load enrolments.txt\n";
    std::vector<Staff> staff = loadStaff(dataDir + "staff.txt", data);
    int seats = totalSeats(dataDir + "rooms.txt");
    std::string requestFile = dataDir + "pending_requests.txt";

    std::cout << "=====================================================\n"
              << "   Integrated Smart Campus Management System\n"
              << "   DSA-II PBL  |  Loaded: " << data.locations.size() << " locations, "
              << data.courseCodes.size() << " courses, " << students.size() << " students, "
              << staff.size() << " staff\n"
              << "=====================================================\n";

    while (true) {
        std::cout << "\nMAIN MENU\n"
                  << "  1. Campus Navigation\n"
                  << "  2. Infrastructure Network Planning\n"
                  << "  3. Course Prerequisite Planner\n"
                  << "  4. Student Records\n"
                  << "  5. Service Requests\n"
                  << "  6. Exam Timetable\n"
                  << "  7. Integrated Services\n"
                  << "  0. Exit\n";
        int ch = readInt("Choice: ", 0, 7);
        switch (ch) {
            case 1: navigationMenu(data); break;
            case 2: networkMenu(data); break;
            case 3: courseMenu(data); break;
            case 4: studentMenu(students); break;
            case 5: requestMenu(requests, requestFile); break;
            case 6: timetableMenu(exams, seats); break;
            case 7: integratedMenu(data, students, exams, requests, staff, seats); break;
            default: std::cout << "Goodbye!\n"; return 0;
        }
    }
}
