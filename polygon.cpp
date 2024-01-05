//
// Created by jens on 29/11/23.
//

#include <iostream>
#include <vector>
#include <numeric>
#include "polygon.h"
#include "world.h"


/** Adaptor object to convert a edge number (edge_t) to an actual path */
class path_lookup final {
    world const &w_;
public:
    path_lookup(world const &w): w_(w) {}
    path const &operator()(edge_t e) const { return w_.map_[e]; }
};


poly_errno_t polygon::is_valid(const world &w) const noexcept
{
    // We need the world map to map edge numbers to paths and real-world locations
    path_lookup const lookup(w);

    std::vector<pathpoint> visited;
    // Remember the polygon iterator works backwards through paths, from start back to start
    auto e = begin(), m = end();
    // A single-point polygon is not allowed
    if(e == m)
        return poly_errno_t::POLY_EMPTY;
    // a and b are initialised to the endpoints of the first (last) edge
    auto [a,b] = lookup(*e++).endpoints();

    while (e != m) {
        auto p = lookup(*e).endpoints();
        // path may need to be reversed to fit the polygon
        if (a == p.first) {
            a = p.second;
            visited.push_back(a);
        } else if (a == p.second) {
            a = p.first;
            visited.push_back(a);
        } else if (b == p.first) {
            b = p.second;
            visited.push_back(b);
        } else if (b == p.second) {
            b = p.first;
            visited.push_back(b);
        } else
            return poly_errno_t::POLY_BROKENPATH;
        ++e;
    }
    // Polygon not closed
    if(a != b)
        return poly_errno_t::POLY_NOTCLOSED;
    // Check non-intersection other than at start/end - could probably be optimised
    visited.pop_back();
    auto all = visited.end();
    std::sort(visited.begin(), all);
    if(std::unique(visited.begin(), visited.end()) != all)
        return poly_errno_t::POLY_SELFINTERSECT;
    return poly_errno_t::POLY_GOOD;
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
bool polygon::interior(world &w, point p) const
{
    path_lookup lookup(w);
    std::cerr << "Interior " << p << std::endl;
    // This will go through all line segments in arbitrary order but that is OK for this purpose
    auto const seqs = edges_
            | std::views::transform(lookup)
            | std::views::join
            | std::views::transform([p](lineseg const &s) { return intersects(s,p); });
    // Neither reduce nor accumulate is available in constrained form
    auto const score = std::reduce(std::begin(seqs), std::end(seqs), 0u);
    // Remember, double the score is returned, and p is interior if the score is odd
    if(score & 1u)
        throw BadPath("interior double score uneven");  // can't happen?
    // interior if twice an odd number or congruent to two mod four
    return (score & 2u) == 2u;
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


char const *poly_errno_string(poly_errno_t err) {
    switch(err) {
        case poly_errno_t::POLY_GOOD:
            return "OK";
        case poly_errno_t::POLY_SELFINTERSECT:
            return "self intersecting";
        case poly_errno_t::POLY_NOTCLOSED:
            return "not closed";
        case poly_errno_t::POLY_BROKENPATH:
            return "disconnected paths";
        case poly_errno_t::POLY_EMPTY:
            return "degenerate (single-point) polygon";
    }
    return "canthappen polygon error";
}
