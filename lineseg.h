//
// Created by jens on 02/09/23.
//

#ifndef VEC2POLY_LINESEG_H
#define VEC2POLY_LINESEG_H

#include <optional>
#include <vector>
#include "point.h"

class lineseg {
private:
    /** The line segment is a point from A to B */
    point a_, b_;
    /** Once the points are defined we can calculate the vector A->B */
    double dx_, dy_;
public:
    lineseg(point a, point b) : a_(a), b_(b), dx_(b_.x_-a_.x_), dy_(b_.y_-a_.y_) {}
    /** Split a line segment into two parts at t (0 < t < 1) along its length
     * The line segment is shortened
     * @param t The position between A (t==0) and B (t==1) to split
     * @return The remaining segment (from the split point to B)
     */
    lineseg split(double t);
    friend std::optional<point> intersects(lineseg const &v, lineseg const &w);
};


class path {
private:
    std::vector<lineseg> v_;
};


#endif //VEC2POLY_LINESEG_H
