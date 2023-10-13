//
// Created by jens on 02/09/23.
//

#ifndef VEC2POLY_POINT_H
#define VEC2POLY_POINT_H

#include <cmath>
#include <iosfwd>
#include <memory>


class pntalloc;


/** point is a POD class holding the basic version of point */
class point {
private:
    /** x and y coordinates of point */
    double x_, y_;
public:
    point(double x, double y): x_(x), y_(y) {}

    bool operator==(point const &other) const noexcept
    {
        auto sqr = [](double z) { return z*z; };
        return sqr(other.x_ - x_) + sqr(other.y_ - y_) < tol2;
    }
    bool operator!=(point const &other) const noexcept
    { return !(*this == other); }
    bool operator<(point const &other) const noexcept
    {
        if(*this == other) return false;
        if(x_ < other.x_-tol) {
            if(y_ < other.y_-tol)
                return true;
        }
        // Lexicographic: x1==x2, y1<y2
        return x_ < other.x_+tol && y_ < other.y_-tol;
    }

    [[nodiscard]] double x() const noexcept { return x_; }
    [[nodiscard]] double y() const noexcept { return y_; }

    static bool zero(double z) noexcept { return fabs(z)<tol; }

    /** Tolerance for equality */
    static double tol;
    /** Tolerance for equality, squared */
    static double tol2;

};


std::ostream &operator<<(std::ostream &, point const &);


class xpathpoint;
using pathpoint = xpathpoint &;

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

    bool equals(point o)
    {
        // Compare as points, ignoring the usage count
        return *this == o;
    }

    friend std::ostream &operator<<(std::ostream &, pathpoint);
};


#endif //VEC2POLY_POINT_H
