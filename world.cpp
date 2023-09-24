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


void world::iterator::insert_after(lineseg &&elt)
{
    auto at{dc_};
    /* When we're called dc_ is never at the end and we need to insert at the next position
     * (which may be at the end) without incrementing dc_
     */
    if(dc_->last() != elt.first())
        throw BadPath("Start of inserted segment does not match previous");
    ++at;
    if(at != ds_ && elt.last() != at->first())
        throw BadPath("End of inserted segment does not match next");
    cc_->path_.insert(at, std::forward<lineseg>(elt));
}


void world::split_paths()
{
    iterator p = begin();
    iterator const z = end();
    /* A pairwise comparison of every line segment with every other one.
     * The invariant is that every segment up to p intersects any other segments
     * only at its endpoints, if at all.
     */
    while(p != z ) {
        iterator q = p; ++q;
        while(q != z ) {
            auto u{intersects(alloc_, *p, *q)};
            if(u.has_value()) {
                auto v{u.value()};
                if(!p->is_endpoint(v)) {
                    // split_at shortens the current line segment and returns the one to insert
                    p.insert_after(p->split_at(v));
                }
                if(!q->is_endpoint(v)) {
                    q.insert_after(q->split_at(v));
                }
            }
            ++q;
        }
        ++p;
    }
}
