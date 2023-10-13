//
// Created by jens on 23/09/23.
//


#include <iostream>
#include "pntalloc.h"


pntalloc::pntalloc() noexcept {}


pathpoint pntalloc::make_point(point z)
{
    auto u = lookup(z);
    std::cerr << "MP " << u << std::endl;
    if( u == -1 ) {
        mem_.emplace_back(z);
        return mem_.back();
    }
    mem_[u].incf();
    return mem_[u];
}
