//
// Created by jens on 23/09/23.
//


#include <algorithm>
#include "pntalloc.h"


pntalloc::pntalloc() noexcept {}


pathpoint pntalloc::make_point(point z)
{
    auto const first{mem_.cbegin()}, last{mem_.cend()};
    auto u = lookup(z);
    if( u == -1 ) {
        mem_.emplace_back(z);
        return mem_.back();
    }
    mem_[u].incf();
    return mem_[u];
}
