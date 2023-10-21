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
#include "pntalloc.h"

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
    pathpoint a_, b_;
    /** Once the points are defined we can calculate the vector A->B */
    double dx_, dy_;
    void recalculate()
    {
        dx_ = b_->x() - a_->x();
        dy_ = b_->y() - a_->y();
    }
public:
    lineseg(pntalloc &alloc, point a, point b) : a_(alloc.make_point(a)), b_(alloc.make_point(b)), dx_(b.x() - a.x()), dy_(b.y() - a.y())
    {
#if 0
        if(dx_*dx_+dy_*dy_ < point::tol2)
            throw BadLineSegment();
#endif
    }
    lineseg(pntalloc &alloc, point a, pathpoint b) : a_(alloc.make_point(a)), b_(b)
    {
        recalculate();
    }
    lineseg(pntalloc &, pathpoint a, pathpoint b) noexcept : a_(a), b_(b)
    {
        recalculate();
    }
    lineseg(lineseg const &) = delete;
    lineseg(lineseg &&) = default;
    lineseg &operator=(lineseg const &) = delete;
    lineseg &operator=(lineseg &&) = default;
    ~lineseg() {}

    /* Equality: are line segments equal if one is the reverse of the other? */
    bool operator==(lineseg const &other) const noexcept
    {
        return a_ == other.a_ && b_ == other.b_;
    }
    pathpoint first() const noexcept { return a_; }
    pathpoint second() const noexcept { return b_; }
    pathpoint last() const noexcept { return b_; }
    //std::pair<pathpoint,pathpoint> endpoints() const noexcept { return std::pair(a_, b_); }

    /** Reverse in-place */
    //void rev() noexcept { std::swap(a_,b_); }

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
    lineseg split_at(pntalloc &alloc, point p);
    bool is_endpoint(point p) const noexcept
    {
        return a_->equals(p) || b_->equals(p);
    }
    friend std::optional<point> intersects(lineseg const &v, lineseg const &w);
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
    path(pntalloc &alloc, std::initializer_list<point> q);
    path(path const &) = delete;
    path(path &&) = default;
    path &operator=(path const &) = delete;
    path &operator=(path &&) = default;

    /** Split the path at every one of the given points
     * Additional paths will be passed to the iterator
     * @tparam newpath inserter callback for new paths
     * @tparam at Points to split at
     */
    void split_path(std::function<void(path &&)> newpath, const std::set<point> &at);

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
