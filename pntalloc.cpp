//
// Created by jens on 23/09/23.
//


#include <algorithm>
#include "pntalloc.h"


pntalloc::pntalloc() noexcept {}


point pntalloc::make_point(double x, double y)
{
    basepoint z(x,y);
    auto u = std::find_if(mem_.cbegin(), mem_.cend(), [&z](auto const &p){ return z == *p; });
    if( u == mem_.cend() ) {
        mem_.push_back(std::make_shared<basepoint>(x, y));
        // push back may have invalidated the iterators
        return mem_.back();
    }
    return *u;
}
