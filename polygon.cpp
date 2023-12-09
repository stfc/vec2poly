//
// Created by jens on 29/11/23.
//

#include <iostream>
#include <vector>
#include <numeric>
#include "polygon.h"
#include "world.h"


bool polygon::is_valid(const world &w) const noexcept
{
    // We need the world map to map edge numbers to paths and real-world locations
    auto const &worldmap{w.map()};

    // A single-point polygon is not allowed
    if(edges_.empty())
        return false;
    std::vector<pathpoint> visited;
    auto e = begin(), m = end();
//    point current{start_};

    visited.push_back(worldmap[*e].endpoints().first);
    while (e != m) {
        // Endpoint of the current path
        auto p = worldmap[*e].endpoints().second;
        visited.push_back(p);
        ++e;
    }
    // Polygon not closed
    if(visited.front() != visited.back())
        return false;
    // Check non-intersection other than at start/end - could probably be optimised
    visited.pop_back();
    auto all = visited.end();
    std::sort(visited.begin(), all);
    if(std::unique(visited.begin(), visited.end()) != all)
        return false;
    return true;
}


/** Adaptor object to convert a edge number (edge_t) to an actual path */
class path_lookup final {
    world const &w_;
public:
    path_lookup(world const &w): w_(w) {}
    path const &operator()(edge_t e) { return w_.map_[e]; }
};


void polygon::linesegs(world const &w)
{
    path_lookup lookup(w);
    for( auto &s :
        edges_ 
        | std::views::transform(lookup)
        | std::views::join )
        std::cerr << s << '\n';
}


/** Tidy a polygon by reducing it in size until it has nothing inside.
 *
 * This function is the last major algorithmic component: loop through
 * all paths not already on the polygon, if any is interior to the current
 * polygon then replace a piece of the polygon with this path, thus reducing
 * the number of interior paths by one - and repeat.
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
    //while(!clean_path())
}


/** Determine whether a point is interior to the polygon.
 * We may assume the test point never lies on a line segment
 * @param p test point
 * @return true if the test point is interior
 *
 * @cite https://www.eecs.umich.edu/courses/eecs380/HANDOUTS/PROJ2/InsidePoly.html
 */
bool polygon::interior(world &, point p) const
{
    //unsigned
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
