//
// Point factory - points are shared so they can be counted
// Created by jens on 23/09/23.
//

#ifndef VEC2POLY_PNTALLOC_H
#define VEC2POLY_PNTALLOC_H

#include <memory>
#include <ranges>
#include <vector>
#include "point.h"

class world;

class pntalloc {
private:
    std::vector<pathpoint> mem_;

    pntalloc() noexcept;
public:
    pathpoint make_point(point z);
    pathpoint make_point(double x, double y)
    {
        mem_.emplace_back(x,y);
        return mem_.back();
    }

    std::ranges::view auto points() noexcept
    {
        return std::ranges::views::all(mem_);
    }

    /** Look up a base point to see if it is a point */
    ssize_t lookup(point bp)
    {
        auto y = std::ranges::find(mem_, bp);
        if(y == mem_.cend())
            return -1;
        return y-mem_.cbegin();
    }

    // only world can create us
    friend class world;
    // and the unit tests
    friend bool test_pntalloc();
    friend bool test_lineseg();
};


#endif //VEC2POLY_PNTALLOC_H
