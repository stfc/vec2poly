//
// Created by jens on 02/09/23.
//

#include <cassert>
#include <ranges>
#include <iostream>
#include "lineseg.h"


lineseg lineseg::split_at(point p)
{
    if(is_endpoint(p))
        throw BadLineSegment();
    point q{b_};
    b_ = p;
    return lineseg(p, q);
}


std::optional<point> intersects(lineseg const &v, lineseg const &w)
{
    // Determinant
    auto det = w.dx_ * v.dy_ - w.dy_ * v.dx_;
    if( point::zero(det) )
        return std::nullopt;
    // Vector to solve against (see docs for details)
    lineseg k(v.b_, w.b_);
    // s is the coefficient along v; t is the coefficient along w
    double s = (w.dy_ * k.dx_ - w.dx_ * k.dy_)/det;
    double t = (v.dy_ * k.dx_ - v.dx_ * k.dy_)/det;
    if( -point::tol < s && s < 1.0+point::tol &&
        -point::tol < t && t < 1.0+point::tol)
    {
        point z{w.b_};
        return point(z.x()-t*w.dx_,z.y()-t*w.dy_);
    }
    return std::nullopt;
}


std::ostream &operator<<(std::ostream &os, lineseg const &line)
{
    os << '[' << line.a_ << ',' << line.b_ << ']';
    return os;
}


path::path(std::initializer_list<point> q)
{
    if(q.size() < 1)
        throw BadPath();
    // adjacent/pairwise not available until C++23... and zip
    point prev{*q.begin()};
    for( auto const &y : std::ranges::views::drop(q, 1)) {
        path_.push_back(lineseg(prev, y));
        prev = y;
    }
}


std::ostream &operator<<(std::ostream &os, path const &p)
{
    for(auto const &p : p.path_)
	os << p;
    return os;
}
