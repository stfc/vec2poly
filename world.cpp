//
// Created by jens on 17/09/23.
//

#include <iostream>
#include "world.h"


world::iterator &world::iterator::operator++() noexcept
{
    if(++dc_ == ds_) {
        if(++cc_ != cs_) {
            ds_ = cc_->path_.end();
            dc_ = cc_->path_.begin();
        }
    }
    return *this;
}


void world::split_paths()
{
    // Dummy test code
    iterator p(map_), q(map_, false);
    while(p != q) {
        lineseg u{*p};
        ++p;
        std::cout << u << '\n';
    }
}
