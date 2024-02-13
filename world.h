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


// defined in polygon.cpp
class path_lookup;


/** World - the home of all paths
 *
 * The world is not really an object, it is the entire program...
 * but it's convenient to have it structured as an object to aid testing
 */

class world final {
private:
    std::vector<path> map_;

    /* Point factory */
    pntalloc alloc_;

    /** Iterator over all line segments in all paths in the world.
     *
     * This iterator works like ranges::views::join, but we need both iterators
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

    world(double tol) : map_(), alloc_(tol) {}
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
    void proper_paths(std::vector<point> bps = {});

    /** Iterator over all line segments */
    auto segments() { return std::ranges::views::join(map_); }

    /** Iterate over all paths */
    auto paths() const { return std::ranges::views::all(map_); }

    /** Provide read-only access to the paths container */
    decltype(map_) const &map() const noexcept { return map_; }

    /** All branch points
     * Throws BadWorld if an isolated point is found */
    [[nodiscard]] auto branch_points()
    {
        auto filter = [](pathpoint x) -> bool
        {
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

    // defined in polygon.cpp
    friend class path_lookup;

    // regression testing functions break encapsulation
    friend pntalloc &test_allocator(world &);
    friend decltype(world::map_) &test_paths(world &);
    friend bool test_poly1();
};


#endif //VEC2POLY_WORLD_H
