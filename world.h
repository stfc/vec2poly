//
// Created by jens on 17/09/23.
//

#ifndef VEC2POLY_WORLD_H
#define VEC2POLY_WORLD_H

#include <vector>
#include <exception>
#include <ranges>
#include "lineseg.h"
#include "pntalloc.h"


struct BadWorld : public std::exception
{
private:
    char const *msg_;
public:
    BadWorld() noexcept : msg_("Bad World") {};
    // Note no copy is made of the string so it better be static
    BadWorld(char const *msg) noexcept : msg_(msg) {}
    char const *what() const noexcept override { return msg_; }
};

/** World - the home of all paths */

class world {
private:
    std::vector<path> map_;

    /* Point factory */
    pntalloc alloc_;

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
            else if(other.cc_ == other.cs_)
                return false;
            return dc_ == other.dc_;
        }
        auto &operator*() { return *dc_; }
        auto *operator->() { return &(*dc_); }

        /** Insert a line segment _after_ the current one.
         *
         * Insertion does not validate the iterator, though add_path may.
         * It is not possible to insert at beginning (with this call).
         * */
        void insert_after(lineseg &&);
    };
    iterator begin() { return iterator(map_); }
    iterator end() { return iterator(map_, false); }

public:
    /* We must prevent the world from being copied
     * as it would currently invalidate the shared pointer counter
     * which is used to track incidence to every point. */
    world() = default;
    world(world const &) = delete;
    world(world &&) = default;
    world &operator=(world const &) = delete;
    world &operator=(world &&) = default;

    void add_path(path &&p) { map_.emplace_back(std::forward<path>(p)); }
    void add_path(std::initializer_list<point> const &p)
    {
        map_.emplace_back(alloc_, p);
    }
    /** Import paths by moving them */
    void import(std::vector<path> &);

    /** Split line segments at intersection points */
    void split_segments();
    /** Reorder paths into proper paths by ensuring endpoints in the set bps
     * Default (if bps is empty) is to use the branch points */
    void proper_paths(std::vector<point> bps);

    /** Iterator over all line segments */
    auto segments() { return std::ranges::views::join(map_); }

    /** All branch points
     * Throws BadWorld if an isolated point is found */
    auto branch_points()
    {
        auto filter = [](pathpoint x) -> bool
        {
            // use_count is normally incidence plus one; here x is another copy
            // so incidence is use count minus two
            auto i = x->use_count();
            if(i == 1)
                throw BadWorld("Unconnected line segment found");
            return i>2;
        };
        return alloc_.points() | std::ranges::views::filter(filter);
    }

    auto points() { return alloc_.points(); }

    /** Forward point construction */
    pathpoint make_point(double x, double y) { return alloc_.make_point(x, y); }

    friend std::ostream &operator<<(std::ostream &, world const &);

    // regression testing functions break encapsulation
    friend pntalloc &test_allocator(world &);
    friend decltype(world::map_) &test_paths(world &);
    friend bool test_poly1();
};


#endif //VEC2POLY_WORLD_H
