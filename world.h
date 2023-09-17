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

    /* This iterator works like ranges::views::join but we need both */
    struct iterator {
        // current and sentinel
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
        bool operator==(const iterator &other) { return cc_ == other.cc_ && ds_ == other.ds_; }
        auto &operator*() { return *dc_; }
    };
    iterator begin() { return iterator(map_); }
    iterator end() { return iterator(map_, false); }

public:
    void add_path(path &&p) { map_.push_back(std::move(p)); }
    void split_paths();
};


#endif //VEC2POLY_WORLD_H
