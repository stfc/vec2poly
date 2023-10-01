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


lineseg lineseg::split_at(point p)
{
    if(is_endpoint(p))
        throw BadLineSegment();
    point q{b_};
    b_ = p;
    return lineseg(p, q);
}


std::optional<point> intersects(pntalloc &alloc, lineseg const &v, lineseg const &w)
{
    // Determinant
    auto det = w.dx_ * v.dy_ - w.dy_ * v.dx_;
    if( basepoint::zero(det) )
        return std::nullopt;
    // Vector to solve against (see docs for details)
    lineseg k(v.b_, w.b_);
    // s is the coefficient along v; t is the coefficient along w
    double s = (w.dy_ * k.dx_ - w.dx_ * k.dy_)/det;
    double t = (v.dy_ * k.dx_ - v.dx_ * k.dy_)/det;
    if( -basepoint::tol < s && s < 1.0+basepoint::tol &&
        -basepoint::tol < t && t < 1.0+basepoint::tol )
    {
        basepoint z{w.b_->x(), w.b_->y()};
        return alloc.make_point(z.x()-t*w.dx_,z.y()-t*w.dy_);
    }
    return std::nullopt;
}


std::ostream &operator<<(std::ostream &os, lineseg const &line)
{
    os << '[' << line.a_ << ',' << line.b_ << ']';
    return os;
}


path::path(std::initializer_list<point> q) : path_(), used_(false)
{
    if(q.size() < 1)
        throw BadPath("Path too short");
    // adjacent/pairwise not available until C++23... and zip
    point prev{*q.begin()};
    for( auto const &y : std::ranges::views::drop(q, 1)) {
        path_.push_back(lineseg(prev, y));
        prev = y;
    }
}


path::path(pntalloc &alloc, std::initializer_list<std::pair<double, double>> q) : path_(), used_(false)
{
    if(q.size() < 1)
        throw BadPath("Path too short");
    auto start = *q.begin();
    auto prev = alloc.make_point(start.first, start.second);
    for( auto [x,y] : std::ranges::views::drop(q,1) ) {
        point w = alloc.make_point(x,y);
        path_.push_back(lineseg(prev, w));
        prev = w;
    }
}


void path::split_path(std::function<void(path &&)> newpath, std::set<point> const &at)
{
    if(at.empty()) return;
    auto p = path_.begin();
    auto const q = path_.end();
    // The path containing the first segment of the original path is a special case
    // because it may need joining up to the very last path
    // (if the path is a loop not starting in a point in the at set)
    path first;
    while( p != q ) {
        // Find a line segment starting in any one of the target points
        auto u = std::find_if(p, q,
                              [&at](auto const &y) -> bool
                              {
                                  return at.contains(y.first());
                              });
        if(u == q) {
            // No at-points at all on the path, nothing to do
            if(first.path_.empty())
                return;
            // Otherwise check if we can splice the first path to the last
            if(first.path_.back().last() == path_.front().first()) {
                path_.splice(path_.end(), first.path_);
                // No insertion call for this path; it becomes *this
                return;
            }
            // If we get here, first is non-empty but cannot be connected
            // There will be trouble, later, but for now, save it
            newpath(std::move(first));
            return;
        }
        if(first.path_.empty())
            first.path_.splice(first.path_.begin(), path_,
                               p, u);
        else {
            // Otherwise p->u is to become a separate path
            path next;
            // Splicing lists does not invalidate iterators
            next.path_.splice(next.begin(), path_, p, u);
            newpath(std::move(next));
        }
        // And we continue processing the current path from u
        p = ++u;
    }
    if(!first.path_.empty())
        path_.splice(path_.end(), first.path_);
}


std::ostream &operator<<(std::ostream &os, path const &p)
{
    for(auto const &p : p.path_)
	os << p;
    return os;
}
