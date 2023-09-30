//
// Created by jens on 02/09/23.
//

#ifndef VEC2POLY_LINESEG_H
#define VEC2POLY_LINESEG_H

#include <exception>
#include <optional>
#include <list>
#include <set>
#include <iosfwd>
#include <utility>
#include <memory>
#include <functional>
#include "point.h"

class BadLineSegment : public std::exception {
    char const *const msg_;
public:
    BadLineSegment() : msg_("Bad Line Segment") {};
    BadLineSegment(char const *msg) : msg_(msg) {}
    char const *what() const noexcept override { return msg_; }
};
class BadPath : public std::exception {
    char const *const msg_;
public:
    BadPath() : msg_("Bad Path") {};
    BadPath(char const *msg) : msg_(msg) {}
    char const *what() const noexcept override { return msg_; }
};

class lineseg {
private:
    /** The line segment is a point from A to B */
    point a_, b_;
    /** Once the points are defined we can calculate the vector A->B */
    double dx_, dy_;
public:
    lineseg(point a, point b) : a_(a), b_(b), dx_(b_->x()-a_->x()), dy_(b_->y()-a_->y())
    {
#if 0
        if(dx_*dx_+dy_*dy_ < point::tol2)
            throw BadLineSegment();
#endif
    }

    /* Equality: are line segments equal if one is the reverse of the other? */
    bool operator==(lineseg const &other) const noexcept
    {
        return a_ == other.a_ && b_ == other.b_;
    }
    point first() const noexcept { return a_; }
    point second() const noexcept { return b_; }
    point last() const noexcept { return b_; }
    std::pair<point,point> endpoints() const noexcept { return std::pair(a_,b_); }

    /** Reverse in-place */
    void rev() noexcept { std::swap(a_,b_); }

    /** Split a line segment into two parts at a given point along its length
     * The line segment is shortened and the remaining segment is returned.
     *
     * If the present line segment is A->B and it is split at C, then the current
     * object is turned into A->C and a new segment C->B is returned.
     *
     * There is no check that C is on the line segment so in general we always get A->C->B.
     * However, there _is_ a check that the split point is not already an endpoint
     * (which would make one line segment invalid)
     *
     * @param p The point to split at
     * @return The remaining segment (from the split point to B)
     */
    lineseg split_at(point p);
    bool is_endpoint(point p) const noexcept { return p == a_ || p == b_; }
    friend std::optional<point> intersects(pntalloc &alloc, lineseg const &v, lineseg const &w);
    friend std::ostream &operator<<(std::ostream &, lineseg const &);
};


class world;

/** A path is a sequence of line segments where each segment begins where the previous finished
 *
 */
class path {
private:
    /** List of line segments forming this path.
     * Line segments should be connected and non-degenerate (not a point)
     */
    std::list<lineseg> path_;
    /** Whether we have been assigned a polygon to live in */
    bool used_;
    /** Empty path constructer is private */
    path() : path_{}, used_(false) {}
public:
    /** Construct path connecting at least two points */
    path(std::initializer_list<point> q);
    /** Clunky low level c'tor used only for test code */
    path(pntalloc &alloc, std::initializer_list<std::pair<double, double>> q);
    path(path const &) = delete;
    path(path &&) = default;
    path &operator=(path const &) = delete;
    path &operator=(path &&) = default;

    /** Split the path at every one of the given points
     * Additional paths will be passed to the iterator
     * @tparam newpath inserter callback for new paths
     * @tparam at Points to split at
     */
    void split_path(std::function<void(path &&)> newpath, std::set<point> const &at);

    bool is_used() const noexcept { return used_; }
    void set_used() noexcept { used_ = true; }

    bool operator==(path const &) const noexcept = default;

    auto begin() const noexcept { return path_.begin(); }
    auto end() const noexcept { return path_.end(); }

    /* This breaks encapsulation but can be Fixed Later(tm) */
    friend class world;
    friend std::ostream &operator<<(std::ostream &, path const &);
};


#endif //VEC2POLY_LINESEG_H
