//
// Point factory - points are shared so they can be counted
// Created by jens on 23/09/23.
//

#ifndef VEC2POLY_PNTALLOC_H
#define VEC2POLY_PNTALLOC_H

#include <memory>
#include <ranges>
#include <vector>
#include <concepts>
#include "point.h"
#include "lineseg.h"

class world;

class pntalloc {
private:
    std::vector<xpathpoint> mem_;
    /** tolerance for snapping points to grid */
    double tol_;

    pntalloc(double tol) noexcept;
public:
    pathpoint make_point(point z);

    pathpoint make_point(double x, double y)
    {
	// XXX check for loss of precision
        point q(std::round(x/tol_),std::round(y/tol_));
        return make_point(q);
    }

    template<typename A, typename B>
    requires std::convertible_to<A,double> && std::convertible_to<B,double>
    pathpoint make_point(A const x, B const y)
    {
         return make_point(static_cast<double>(x), static_cast<double>(y));
    }

    std::ranges::view auto points() noexcept
    {
        return std::ranges::views::all(mem_) | std::views::transform([](auto &x) { return &x;});
    }

    /** Look up a base point to see if it is a point */
    ssize_t lookup(point bp)
    {
        auto y = std::ranges::find(mem_, bp);
        if(y == mem_.cend())
            return -1;
        return y-mem_.cbegin();
    }

    lineseg make_lineseg(pathpoint a, point b)
    {
        return make_lineseg(a, make_point(b));
    }
    lineseg make_lineseg(point a, pathpoint b)
    {
        return make_lineseg(make_point(a), b);
    }
    lineseg make_lineseg(point a, point b)
    {
        return make_lineseg(make_point(a), make_point(b));
    }

    lineseg make_lineseg(pathpoint a, pathpoint b)
    {
        return lineseg(*this, a, b);
    }

    double tol() const noexcept { return tol_; }

    // only world can create us
    friend class world;
    // and the unit tests
    friend bool test_pntalloc();
    friend bool test_lineseg();
    friend bool test_split_seg();
    friend bool test_interior1();
};


#endif //VEC2POLY_PNTALLOC_H
