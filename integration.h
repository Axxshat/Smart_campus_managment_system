// integration.h  --  glue that combines several modules on top of the shared data layer
//   dispatchNearest : Service Requests (heap) + Navigation (Dijkstra)  -> nearest suitable staff
#pragma once
#include <string>
#include <vector>

#include "campus_data.h"
#include "service_requests.h"

namespace campus {

struct Staff {
    std::string name;
    std::string role;       // matches Request::category
    int location = -1;      // location id on the campus map
};

struct Dispatch {
    bool found = false;
    Staff staff;
    int distance = 0;            // metres from the staff member's location to the request
    std::vector<int> path;       // route staff -> request
};

std::vector<Staff> loadStaff(const std::string& path, const CampusData& d);

// Picks the closest staff member whose role matches the request category.
// One Dijkstra run from the request location, O((V + E) log V + S).
Dispatch dispatchNearest(const CampusData& d, const std::vector<Staff>& staff, const Request& r);

}  // namespace campus
