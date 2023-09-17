//
// Created by jens on 02/09/23.
//

#ifndef VEC2POLY_LINESEG_H
#define VEC2POLY_LINESEG_H

#include <optional>
#include <list>
#include <iosfwd>
#include "point.h"

class BadLineSegment {
};
class BadPath {
};

class lineseg {
private:
    /** The line segment is a point from A to B */
    point a_, b_;
    /** Once the points are defined we can calculate the vector A->B */
    double dx_, dy_;
public:
    lineseg(point a, point b) : a_(a), b_(b), dx_(b_.x_-a_.x_), dy_(b_.y_-a_.y_)
    {
#if 0
        if(dx_*dx_+dy_*dy_ < point::tol2)
            throw BadLineSegment();
#endif
    }
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
    friend std::optional<point> intersects(lineseg const &v, lineseg const &w);
    friend std::ostream &operator<<(std::ostream &, lineseg const &);
};


class world;

/** A path is a sequence of line segments where each segment begins where the previous finished
 *
 */
class path {
private:
    std::list<lineseg> path_;
public:
    path(std::initializer_list<point> q);
    path(path const &) = delete;
    path(path &&) = default;
    path &operator=(path const &) = delete;
    path &operator=(path &&) = default;
    /* This breaks encapsulation but can be Fixed Later(tm) */
    friend class world;
};


#endif //VEC2POLY_LINESEG_H
