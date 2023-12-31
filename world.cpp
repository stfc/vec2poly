//
// Created by jens on 17/09/23.
//

#include <iostream>
#include <set>
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


void world::import(std::vector<path> &paths)
{
    auto s1 = map_.size(), s2 = paths.size();
    map_.reserve(s1+s2);
    for( decltype(s2) j = 0; j < s2; ++j) {
        // order doesn't matter, though it will copy the paths
        map_.push_back(paths.back());
        paths.pop_back();
    }
}


void world::split_segments()
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
            auto u{intersects(*p, *q)};
            if(u.has_value()) {
                auto v{u.value()};
                if(!p->is_endpoint(v))
                    // split_at shortens the current line segment and returns the one to insert
                    p.insert_after(p->split_at(alloc_,v));
                if(!q->is_endpoint(v))
                    q.insert_after(q->split_at(alloc_,v));
            }
            ++q;
        }
        ++p;
    }
}


void world::proper_paths(std::vector<point> bps)
{
    if(map_.empty()) return;
    // Proper paths begin and end in branch points
    // Collapse the view into a vector, as in views.to(...) from C++23
    // (sets don't work with points, at least not out of the box)
    if(bps.empty())
        for( auto y: branch_points() )
            bps.push_back(*y);
    std::vector<path> results;
    // Iterators are not invalidated as we gather results before adding them
    for( path &p : map_ )
        p.split_path(results, bps);
    import(results);
}


std::ostream &operator<<(std::ostream &os, world const &w)
{
    for( auto const &y : w.map_ )
        os << y << '\n';
    return os;
}
