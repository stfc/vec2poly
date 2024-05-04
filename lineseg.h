//
// Created by jens on 02/09/23.
//

#ifndef VEC2POLY_LINESEG_H
#define VEC2POLY_LINESEG_H

#include <optional>
#include <list>
#include <vector>
#include <set>
#include <iosfwd>
#include <utility>
#include <memory>
#include <functional>
#include "point.h"
#include "except.h"


class BadLineSegment : public Vec2PolyException {
public:
    BadLineSegment() : Vec2PolyException("Bad Line Segment") {};
    BadLineSegment(char const *msg) : Vec2PolyException(msg) {}
};
class BadPath : public Vec2PolyException {
public:
    BadPath() : Vec2PolyException("Bad Path") {};
    BadPath(char const *msg) : Vec2PolyException(msg) {}
};


class pntalloc;

class lineseg {
private:
    /** The line segment is a point from A to B */
    pathpoint a_, b_;
    /** Once the points are defined we can calculate the vector A->B */
    double dx_, dy_;
    void recalculate(double tol)
    {
        dx_ = b_->x() - a_->x();
        dy_ = b_->y() - a_->y();
#if 0
        if(dx_*dx_+dy_*dy_ < point::tol2)
            throw BadLineSegment();
#endif
    }

    // Constructor become private as the point allocator pntalloc now constructs line segments, too
    lineseg(pntalloc &, pathpoint a, pathpoint b) noexcept;

public:
    /* Note copying a line segment should not increase the use counter */
    lineseg(lineseg const &) = default;
    lineseg(lineseg &&) = default;
    lineseg &operator=(lineseg const &) = delete;
    lineseg &operator=(lineseg &&) = default;
    ~lineseg() {}

    /* Equality: are line segments equal if one is the reverse of the other? */
    bool operator==(lineseg const &other) const noexcept
    {
        return a_ == other.a_ && b_ == other.b_;
    }
    [[nodiscard]] const pathpoint first() const noexcept { return a_; }
    [[nodiscard]] const pathpoint second() const noexcept { return b_; }
    [[nodiscard]] const pathpoint last() const noexcept { return b_; }
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

    // access to constructor
    friend class pntalloc;

    friend std::optional<point> intersects(lineseg const &v, lineseg const &w);
    friend std::ostream &operator<<(std::ostream &, lineseg const &);
};


/** Check whether a ray going right from the test point intersects a line segment.
 *
 * @return Number of half intersections
 */

unsigned intersects(lineseg const &, const point);


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
    /** Empty path constructor is private as worlds are not allowed to have empty paths */
    path() : path_{} {}
public:
    /** Construct path connecting at least two points */
    path(pntalloc &alloc, std::initializer_list<point> q);
    path(path const &);
    path(path &&) = default;
    path &operator=(path const &) = default;
    path &operator=(path &&) = default;

    /** Return a pair of first and last point */
    auto endpoints() const
    {
        return std::make_pair(path_.front().first(), path_.back().last());
    }

    /** Split the path at every one of the given points.
     * Additional paths will be added to the result vector.
     * @tparam newpath inserter callback for new paths
     * @tparam at Points to split at
     */
    void split_path(std::vector<path> &result, const std::vector<point> &at);

    bool operator==(path const &) const noexcept = default;

    /** Return a point somewhere internal to the path */
    point testpoint() const;

    auto begin() const noexcept { return path_.begin(); }
    auto end() const noexcept { return path_.end(); }
    auto rbegin() const noexcept { return path_.rbegin(); }
    auto rend() const noexcept { return path_.rend(); }

    /** call back for every point on the path */
    void points(std::function<void(pathpoint)>) const;

    /** Return size of list, so potentially O(N) complexity */
    auto size() const noexcept { return path_.size(); }
    /* This breaks encapsulation but can be Fixed Later(tm) */
    friend class world;
    friend std::ostream &operator<<(std::ostream &, path const &);
};


#endif //VEC2POLY_LINESEG_H
