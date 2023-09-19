//
// Created by jens on 17/09/23.
//

#ifndef VEC2POLY_WORLD_H
#define VEC2POLY_WORLD_H

#include <vector>
#include "lineseg.h"

/** World - the home of all paths */

class world {
private:
    std::vector<path> map_;

    /* This iterator works like ranges::views::join, but we need both iterators
     * so we can insert stuff at the list iterator position (which does not
     * invalidate the iterator).
     * Otherwise, this approach is not recommended; much too fiddly.
     * Correctness is helped also by the fact that paths are never empty. */
    struct iterator {
        // current position and sentinel
        std::vector<path>::iterator cc_, cs_;
        decltype(path::path_)::iterator dc_, ds_;

        iterator(decltype(map_) &m, bool begin = true): cc_(begin ? m.begin() : m.end()), cs_(m.end())
        {
            if(m.begin() != cs_) {
                ds_ = cc_->path_.end();
                dc_ = begin ? cc_->path_.begin() : ds_;
            }
        }
        iterator &operator++() noexcept;
	iterator operator++(int) noexcept { auto i{*this}; ++(*this); return i; }
        bool operator==(const iterator &other)
	{
	    /* Could check the sentinels (cs_ and other.cs_):
	     * If the sentinels differ, then we are iterators for different sequences,
	     * or one iterator has been invalidated. */
	    if(cc_ != other.cc_)
		return false;
	    /* dc_ iterator is valid only if cc_ is not equal to cs_ */
	    if(cc_ == cs_)
		return other.cc_ == other.cs_;
	    else
		if(other.cc_ == other.cs_)
		    return false;
	    return dc_ == other.dc_;
	}
        auto &operator*() { return *dc_; }

        /** Insert a line segment _after_ the current one.
	 * Insertion does not validate the iterator, though add_path may.
	 * It is not possible to insert at beginning (with this call).
	 * */
        void insert_after(lineseg &&);
    };
    iterator begin() { return iterator(map_); }
    iterator end() { return iterator(map_, false); }

public:
    void add_path(path &&p) { map_.push_back(std::move(p)); }
    void split_paths();

    // testing
    friend bool test_poly1();
};


#endif //VEC2POLY_WORLD_H
