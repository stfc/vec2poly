//
// Created by jens on 02/09/23.
//

#include <cassert>
#include <ranges>
#include <iostream>
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


std::ostream &operator<<(std::ostream &os, path const &p)
{
    for(auto const &p : p.path_)
	os << p;
    return os;
}
