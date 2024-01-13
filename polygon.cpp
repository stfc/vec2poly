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



/** Utility class to "walk" through a trail - a sequence of paths, calling a callback on each point on the trail.
 *
 * For a trail consisting of N paths, the callback will be called N+1 times.
 * The problem is that some paths need to be traversed backwards as they are bidirectional.
 */
class trail_walk {
public:
    using callback_t = std::function<void(pathpoint)>;
private:
    path_lookup lookup_;
    callback_t cb_;
public:
    trail_walk(world const &w, callback_t cb): lookup_(w), cb_(cb) {}
    [[nodiscard]] std::pair<pathpoint,pathpoint> walk(polygon::poly_iterator begin, polygon::poly_iterator end);
};


std::pair<pathpoint,pathpoint> trail_walk::walk(polygon::poly_iterator begin, polygon::poly_iterator end)
{
    // A single-point polygon is not allowed
    if(begin == end)
        throw BadPolygon(poly_errno_t::POLY_EMPTY);

    auto [a,b] = lookup_(*begin++).endpoints();

    while (begin != end) {
        auto p = lookup_(*begin).endpoints();
        // path may need to be reversed to fit the polygon
        if (a == p.first) {
            cb_(a);
            a = p.second;
        } else if (a == p.second) {
            cb_(a);
            a = p.first;
        } else if (b == p.first) {
            cb_(b);
            b = p.second;
        } else if (b == p.second) {
            cb_(b);
            b = p.first;
            cb_(b);
        } else
            throw BadPolygon(poly_errno_t::POLY_BROKENPATH);
        ++begin;
    }
    return {a,b};
}




poly_errno_t polygon::is_valid(const world &w) const noexcept
{
    // We need the world map to map edge numbers to paths and real-world locations
    path_lookup const lookup(w);

    std::vector<pathpoint> visited;
    // Remember the polygon iterator works backwards through paths, from start back to start
    auto e = begin(), m = end();
    try {
        auto add_to_visited = [&visited](pathpoint p) { visited.push_back(p); };
        trail_walk trail(w, add_to_visited);
        auto final = trail.walk(e, m);
        // Polygon not closed
        if(final.first != final.second)
            return poly_errno_t::POLY_NOTCLOSED;
    }
    catch(BadPolygon bp) {
        return bp.get_errno();
    }
    // Check non-intersection other than at start/end - could probably be optimised
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
    path_lookup lookup(w);
    auto const paths = edges_
        | std::views::transform(lookup);
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


void polygon::replace_paths(world const &w, std::vector<edge_t> &paths, edge_t keep)
{
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
