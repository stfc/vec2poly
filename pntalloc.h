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
    std::vector<point> mem_;

    pntalloc() noexcept;
public:
    point make_point(double x, double y);

    std::ranges::view auto points() noexcept
    {
        return std::ranges::views::all(mem_);
    }

    // only world can create us
    friend class world;
    // and the unit tests
    friend bool test_pntalloc();
    friend bool test_lineseg();
};


#endif //VEC2POLY_PNTALLOC_H
