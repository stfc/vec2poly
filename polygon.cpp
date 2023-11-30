//
// Created by jens on 29/11/23.
//

#include <iostream>
#include <vector>
#include "polygon.h"
#include "world.h"


bool polygon::is_valid(const world &w) const noexcept
{
    // We need the world map to map edge numbers to paths and real-world locations
    auto &worldmap{w.map()};

    // A single-point polygon is not allowed
    if(edges_.empty())
        return false;
    std::vector<pathpoint> visited;
    auto e = begin(), m = end();
    visited.push_back(worldmap[*e].endpoints().first);
    while (e != m) {
        // Endpoint of the current path
        auto p = worldmap[*e].endpoints().second;
        // If a point is re-visited and it's not back-to-the-start, then something's wrong
        visited.push_back(p);
        ++e;
    }
    // Polygon not closed
    if(visited.front() != visited.back())
        return false;
    // Check non-intersection other than at start/end - could probably be optimised
    visited.pop_back();
    std::sort(visited.begin(), visited.end());
    auto len = visited.size();
    std::unique(visited.begin(), visited.end());
    if(visited.size() != len)
        return false;
    return true;
}


/** Tidy a polygon by reducing it in size until it has nothing inside.
 *
 * This function is the last major algorithmic component: loop through
 * all paths not already on the polygon, if any is interior to the current
 * polygon then replace a piece of the polygon with this path, thus reducing
 * the number of internal paths by one - and repeat.
 *
 * Thanks to making paths "proper," if a path has /any/ point interior to
 * the polygon, then the /entire path/ is inside the polygon.
 *
 * @param w World object, containing all the paths in the world
 */

void polygon::tidy(const world &w)
{
    // We need the world map to map edge numbers to paths and real-world locations
    auto &worldmap{w.map()};

}

std::ostream &operator<<(std::ostream &os, const polygon &p)
{
    auto const end{p.come_from_.size()};
    for(size_t i = 0; i < end; ++i) {
        if(p.come_from_[i] == p.invalid_)
            os << '*';
        else
            os << p.come_from_[i];
        os << " -> " << i << '\n';
    }
    return os;
}
