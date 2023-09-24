//
// Created by jens on 02/09/23.
//

#ifndef VEC2POLY_POINT_H
#define VEC2POLY_POINT_H

#include <cmath>
#include <iosfwd>
#include <memory>


class pntalloc;


class basepoint {
private:
    double x_, y_;
public:
    basepoint(double x, double y): x_(x), y_(y) {}
    basepoint(basepoint const &) = default;
    basepoint(basepoint &&) = default;
    basepoint &operator=(basepoint const &) = default;
    basepoint &operator=(basepoint &&) = default;

    bool operator==(basepoint const &other) const noexcept
    {
        auto sqr = [](double z) { return z*z; };
        return sqr(other.x_ - x_) + sqr(other.y_ - y_) < tol2;
    }
    bool operator!=(basepoint const &other) const noexcept
    { return !(*this == other); }

    double x() const noexcept { return x_; }
    double y() const noexcept { return y_; }

    static bool zero(double z) noexcept { return fabs(z)<tol; }

    /** Tolerance for equality */
    static double tol;
    /** Tolerance for equality, squared */
    static double tol2;
};


using point = std::shared_ptr<basepoint>;

// Equality between points is equivalent to equality of pointers due to their construction
// (in pntalloc)
#if 0
bool operator==(point const &p, point const &q) { return *p == *q; }
#endif

std::ostream &operator<<(std::ostream &, point const &);


#endif //VEC2POLY_POINT_H
