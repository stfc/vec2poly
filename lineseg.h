//
// Created by jens on 02/09/23.
//

#ifndef VEC2POLY_LINESEG_H
#define VEC2POLY_LINESEG_H

#include <optional>
#include <vector>
#include "point.h"

class BadLineSegment {
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
        if(dx_*dx_+dy_*dy_ < point::tol2)
            throw BadLineSegment();
    }
    /** Split a line segment into two parts at a given point along its length
     * The line segment is shortened and the remaining segment is returned.
     *
     * If the present line segment is A->B and it is split at C, then the current
     * object is turned into A->C and a new segment C->B is returned.
     *
     * @param p The point to split at
     * @return The remaining segment (from the split point to B)
     */
    lineseg split_at(point p);
    friend std::optional<point> intersects(lineseg const &v, lineseg const &w);
};

/** A path is a sequence of line segments where each segment begins where the previous finished
 *
 */
class path {
private:
    std::vector<lineseg> v_;

};


#endif //VEC2POLY_LINESEG_H
