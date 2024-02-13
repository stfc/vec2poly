//
// Created by jens on 23/09/23.
//


#include <iostream>
#include "pntalloc.h"


pntalloc::pntalloc(double tol) noexcept : tol_(tol)
{
    // XXX temporary work
    mem_.reserve(1000);
}


pathpoint pntalloc::make_point(point z)
{
    auto u = lookup(z);
    if( u == -1 ) {
        mem_.emplace_back(std::move(z));
        return &mem_.back();
    }
    mem_[u].incf();
    return &mem_[u];
}
