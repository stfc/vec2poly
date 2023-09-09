//
// Created by jens on 02/09/23.
//

#include <cassert>
#include "lineseg.h"


lineseg lineseg::split(double t)
{
    assert(point::tol < t && t < 1.0-point::tol);
    point p(a_.x_+dx_*t, a_.y_+dy_*t), q{b_};
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