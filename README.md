# Integrated Smart Campus Management System

A C++17 project built for the **Data Structures and Algorithms II (CCSE0301)** PBL at NIET Greater Noida.
It shows how data structures and graph algorithms can power one integrated campus platform
instead of separate systems for academics, administration, facilities and student services.

- **Problem statement:** Design an integrated smart campus platform that manages institutional data efficiently, optimises operations and improves the student experience through intelligent decision-making.
- **SDG:** 4 – Quality Education
- **Type:** Individual assignment (solo – Akshat Chaurasia)
- **Status:** ✅ Final version (Month 3) – all five modules implemented, integrated and tested

## Architecture

All modules read from one shared data layer, so the same campus data is never stored twice.

```
                         +---------------------------+
                         |  main.cpp (menu driven)   |
                         +-------------+-------------+
                                       |
   +------------+-----------+----------+---------+------------+---------------+
   |            |           |                    |            |               |
+--v----+  +----v----+  +---v-----+       +------v-----+  +---v--------+  +---v---------+
|M1     |  |M2       |  |M3       |       |M4          |  |M5          |  | Integration |
|Navi-  |  |Network  |  |Course   |       |Student     |  |Requests +  |  | dispatch +  |
|gation |  |Planning |  |Planner  |       |Records     |  |Exam slots  |  | dashboard   |
+---+---+  +----+----+  +----+----+       +------+-----+  +-----+------+  +------+------+
    |           |            |                   |              |                |
    +-----------+------------+---------+---------+--------------+----------------+
                                       |
                         +-------------v-------------+
                         |   CampusData (shared)     |
                         |   data/*.txt input files  |
                         +---------------------------+
```

## Modules and DSA concepts

| # | Module | Data structure / algorithm | Time complexity |
|---|--------|----------------------------|-----------------|
| 1 | Campus Navigation | Graph (adjacency list), Dijkstra + min-heap, BFS, DFS | O((V+E) log V) / O(V+E) |
| 2 | Infrastructure Network Planning | Kruskal's MST + Disjoint Set Union | O(E log E) |
| 3 | Course Prerequisite Planner | DAG, Topological Sort (Kahn), cycle detection | O(V+E) |
| 4 | Student Records | Hash table with separate chaining + Trie (name-prefix search) | O(1) average / O(L) per prefix |
| 5 | Service Requests and Exam Timetable | Min-heap with decrease-key, escalation; Welsh–Powell graph colouring with seat limit, exact backtracking check | O(log n) / O(V+E) |
| – | Integration | Dijkstra from the request location + role matching to dispatch the nearest staff; student dashboard | O((V+E) log V) |

## Build and run

Requires `g++` with C++17 support.

```bash
make            # builds ./smartcampus
./smartcampus   # menu driven app (reads the data/ folder)
make test       # 86 self-checking tests
make bench      # data structure benchmark -> docs/benchmark_results.csv
make clean
```

Without `make` (e.g. Windows):

```bash
g++ -std=c++17 -Iinclude src/*.cpp -o smartcampus
smartcampus.exe
```

## Input files (`data/`)

| File | Format |
|------|--------|
| `campus_map.txt` | `LocationA,LocationB,distance_in_metres` |
| `courses.txt` | `CODE\|Course Name\|prereq1,prereq2` |
| `students.txt` | `roll\|name\|branch\|year\|cgpa` (sample data, names are fictional) |
| `enrolments.txt` | `roll\|subject1,subject2,...` (conflict graph is built from this) |
| `rooms.txt` | `room\|seats` |
| `staff.txt` | `name\|role\|home_location` |
| `exam_conflicts.txt` | `SUBJECT1 SUBJECT2` (optional manual conflicts) |

## Results

**Tests:** 86/86 checks pass; also run clean under AddressSanitizer + UBSan.
`docs/sample_output.txt` contains a full demo session.

**Benchmark** (`make bench`, best of 3 runs, -O2; your numbers will differ with the machine):

| Test | Size | Simple method | Used in project |
|------|------|---------------|-----------------|
| Student lookup (2000 searches) | 100,000 records | linear scan 537.5 ms | hash table 0.14 ms (binary search 0.92 ms) |
| Request queue (insert + serve all) | 20,000 requests | sorted array 281.8 ms | min-heap 12.3 ms |
| Single-source shortest paths (10 sources) | 6,000 nodes, 24,000 edges | array Dijkstra O(V²) 505.2 ms | heap Dijkstra 14.6 ms |

## Project structure

```
Smart_campus_managment_system/
├── include/            header files
├── src/                implementations + main.cpp
├── data/               sample input data
├── tests/              test suite
├── bench/              benchmark program
├── docs/               sample output, benchmark csv, screenshots/
├── Makefile
└── README.md
```

## Limitations and future work

- Console application with text-file storage (no database, no GUI)
- Exam timetable gives slots only; spreading exams over dates and invigilator allocation are not implemented
- Hash table has a fixed size (no automatic rehashing)
- Possible extensions: database back end, web/mobile front end, live data from campus sensors, weighted course load balancing

## References

- T. H. Cormen, C. E. Leiserson, R. L. Rivest and C. Stein, *Introduction to Algorithms*, 4th ed., MIT Press, 2022.
- E. Horowitz, S. Sahni and S. Anderson-Freed, *Fundamentals of Data Structures in C*, 2nd ed., Silicon Press, 2008.
- R. V. Imbar et al., "Smart Campus Framework: Definition, Model, Measurement from Anthropocentric, Systemic and Technological Perspectives," *J. ICT Res. Appl.*, vol. 18, no. 3, pp. 258–269, 2025.
- X. Cheng and R. Xue, "Construction of Smart Campus System Based on Cloud Computing," ICASET 2016, Atlantis Press.
- W. Li et al., "Design of smart campus management system based on Internet of Things technology," *J. Intell. Fuzzy Syst.*, vol. 40, no. 2, pp. 3159–3168, 2021.
