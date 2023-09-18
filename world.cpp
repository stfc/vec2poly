//
// Created by jens on 17/09/23.
//

#include <iostream>
#include "world.h"


world::iterator &world::iterator::operator++() noexcept
{
    /* The dc_ iterator is valid only if the cc_ iterator is not equal to cs_.
     * Tracking two iterators is complicated, but we need access to the "inner" one.
     */
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


void world::iterator::insert(lineseg &&elt)
{
    auto at{dc_};
    /* When we're called dc_ is never at the end and we need to insert at the next position
     * (which may be at the end) without incrementing dc_
     */
    ++at;
    cc_->path_.insert(at, std::forward<lineseg>(elt));
}


void world::split_paths()
{
    // Dummy test code
    for( auto const &u : *this ) {
        std::cout << u << ' ';
    }
    std::cout << std::endl;
}
