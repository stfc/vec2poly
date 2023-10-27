//
// Created by jens on 02/09/23.
//

#ifndef VEC2POLY_POINT_H
#define VEC2POLY_POINT_H

#include <cmath>
#include <iosfwd>
#include <memory>
// vector is only needed for the ostream of vectors of points
#include <vector>


class pntalloc;


/** point is a POD class holding the basic version of point */
class point {
private:
    /** x and y coordinates of point */
    signed long x_, y_;
public:
    point(double x, double y): x_(std::round(x/tol)), y_(std::round(y/tol)) {}

    bool operator==(point const &other) const noexcept = default;
    bool operator!=(point const &other) const noexcept = default;
    bool operator<(point const &other) const noexcept
    {
        if(*this == other) return false;
        if(x_ < other.x_) {
            if(y_ < other.y_)
                return true;
        }
        return x_ == other.x_ && y_ < other.y_;
    }

    [[nodiscard]] double x() const noexcept { return x_*tol; }
    [[nodiscard]] double y() const noexcept { return y_*tol; }

    static bool zero(double z) noexcept { return fabs(z)<tol; }

    /** Tolerance for equality */
    static double tol;

};


std::ostream &operator<<(std::ostream &, point const &);

std::ostream &operator<<(std::ostream &, std::vector<point> const &);




class xpathpoint;
using pathpoint = xpathpoint *;

/** pathpoint is the point inside of a path */
class xpathpoint : public point
{
    unsigned use_count_;
public:
    xpathpoint(double x, double y) : point(x, y), use_count_(1) {}
    xpathpoint(point bp) : point(bp), use_count_(1) {}
    xpathpoint(xpathpoint const &) = default;
    xpathpoint(xpathpoint &&) = default;
    xpathpoint &operator=(xpathpoint &o) = default;
    xpathpoint &operator=(xpathpoint &&) = default;
    ~xpathpoint() {}

    [[nodiscard]] auto use_count() const noexcept { return use_count_; }

    void incf() noexcept { ++use_count_; }
    void decf() { if(!(use_count_--)) throw std::out_of_range("decr use count below zero"); }

    bool equals(point o)
    {
        // Compare as points, ignoring the usage count
        return *this == o;
    }

    friend std::ostream &operator<<(std::ostream &, pathpoint);
};


#endif //VEC2POLY_POINT_H
