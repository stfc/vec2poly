//
// Created by jens on 02/09/23.
//

#include <cassert>
#include <ranges>
#include <iostream>
#include <algorithm>
#include <functional>
#include "lineseg.h"
#include "pntalloc.h"


lineseg lineseg::split_at(pntalloc &alloc, point p)
{
    if(is_endpoint(p))
        throw BadLineSegment();
    pathpoint const q{b_};
    b_ = alloc.make_point(p);
    dx_ = b_->x() - a_->x(); dy_ = b_->y() - a_->y();
    return lineseg(alloc, p, q);
}


std::optional<point> intersects(lineseg const &v, lineseg const &w)
{
    // Determinant
    auto det = w.dx_ * v.dy_ - w.dy_ * v.dx_;
    if( point::zero(det) )
        return std::nullopt;
    // Vector to solve against (see docs for details)
//    lineseg k(alloc, v.b_, w.b_);
    auto kdx = w.b_->x()-v.b_->x(), kdy = w.b_->y()-v.b_->y();
    // s is the coefficient along v; t is the coefficient along w
    double s = (w.dy_ * kdx - w.dx_ * kdy)/det;
    double t = (v.dy_ * kdx - v.dx_ * kdy)/det;
    if(-point::tol < s && s < 1.0 + point::tol &&
       -point::tol < t && t < 1.0 + point::tol )
    {
        point z{w.b_->x(), w.b_->y()};
        return point(z.x()-t*w.dx_,z.y()-t*w.dy_);
    }
    return std::nullopt;
}


/** Helper function for polygon::interior().
 * Check whether a ray to the right (X>0) from point intersects segment
 *
 * @param line linesegment we're trying to hit (see ref for interior())
 * @param p test point
 * @return number of half-intersections
 */
unsigned intersects(lineseg const &line, const point p)
{
    point a{static_cast<point>(*line.first())},
            b{static_cast<point>(*line.second())};
    if(between(a.y(), p.y(), b.y())) {
        double t = (p.y()-static_cast<double>(a.y()))/(b.y()-a.y());
	// x coord of intersection with line segment
        double x1 = std::lerp(a.x(), b.x(), t);
	// Intersection must be (strictly) to the right
        return x1 > p.x() ? 2 : 0;
    }
    unsigned k = 0;
    if(p.y() == a.y() && p.x() < a.x())
        ++k;
    if(p.y() == b.y() && p.x() < b.x())
        ++k;
    return k;
}


std::ostream &operator<<(std::ostream &os, lineseg const &line)
{
    os << '[' << line.a_ << ',' << line.b_ << ']';
    return os;
}


path::path(pntalloc &alloc, std::initializer_list<point> q) : path_()
{
    if(q.size() < 1)
        throw BadPath("Path too short");
    // adjacent/pairwise not available until C++23... and zip
    point prev{*q.begin()};
    for( auto const &y : std::ranges::views::drop(q, 1)) {
        path_.emplace_back(alloc, prev, y);
        prev = y;
    }
}


// TODO Should this just be default?
path::path(path const &other) : path_(other.path_)
{
}


void path::split_path(std::vector<path> &result, const std::vector<point> &at)
{
    if(at.empty()) return;
    // p is the main iterator, tracking the search for points to split at;
    // r lags behind p - or equals p - to track the start of the next path
    auto p = path_.begin(), r = p;
    auto const q = path_.end();
    // The path containing the first segment of the original path is a special case
    // because it may need joining up to the very last path
    // (if the path is a loop not starting in a point in the at set)
    path first;
    bool done_first = false;
    while( p != q ) {
        // Find a line segment starting in any one of the target points
        auto u = std::find_if(p, q,
                              [&at](auto const &y) -> bool
                              {
                                 point w = *(y.first());
                                 return std::find( at.cbegin(), at.cend(), w ) != at.cend();
                              });
        if(u == q) {
            // No remaining at-points on the path, nothing left to do
            // so the current path (*this) will be the last
            if(first.path_.empty())
                return;
            // Otherwise check if we can splice the first path to the last
            if(first.path_.back().last() == path_.front().first()) {
                path_.splice(path_.end(), first.path_);
                // No insertion call for this path; it remains *this
                return;
            }
            // If we get here, first is non-empty but cannot be connected
            // There will be trouble, later, but for now, save it
            result.push_back(first);
            return;
        }
        if(!done_first) {
            first.path_.splice(first.path_.begin(), path_,
                               r, u);
            r = u;
            done_first = true;
        } else {
            // Otherwise p->u is to become a separate path
            path next;
            // Splicing lists does not invalidate iterators
            next.path_.splice(next.begin(), path_, r, u);
            r = u;
            result.push_back(next);
        }
        // And we continue processing the current path from u
        p = ++u;
    }
    if(!first.path_.empty())
        path_.splice(path_.end(), first.path_);
    return;
}


std::ostream &operator<<(std::ostream &os, path const &p)
{
    for(auto const &p : p.path_)
        os << p;
    return os;
}
