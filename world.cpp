//
// Created by jens on 17/09/23.
//

#include <iostream>
#include "world.h"


world::iterator &world::iterator::operator++() noexcept
{
    /* The dc_ iterator is valid only if the cc_ iterator is not equal to cs_ */
    if(cc_ == cs_)
	return *this;
    if(++dc_ == ds_)
        if(cc_ != cs_) {
	    ++cc_;
            ds_ = cc_->path_.end();
            dc_ = cc_->path_.begin();
        }
    return *this;
}


void world::split_paths()
{
    // Dummy test code
#if 0
    for( auto const &u : world ) {
        std::cout << u << '\n';
    }
#else
    auto p = begin(), q = end();
    while(p != q) {
        lineseg u{*p};
        ++p;
        std::cout << u << '\n';
    }
#endif
}
