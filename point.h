//
// Created by jens on 02/09/23.
//

#ifndef VEC2POLY_POINT_H
#define VEC2POLY_POINT_H

#include <cmath>
#include <iosfwd>

class lineseg;

class point {
private:
    double x_, y_;
    int pid_;

    /** Counter for unique point id */
    static int pid_ctr;

public:
    point(double x, double y): x_(x), y_(y), pid_(++pid_ctr) {}
    point(point const &) = default;
    point(point &&) = default;
    point &operator=(point const &) = default;
    point &operator=(point &&) = default;

    bool operator==(point const &other) const noexcept
    {
        auto sqr = [](double z) { return z*z; };
        return sqr(other.x_ - x_) + sqr(other.y_ - y_) < tol2;
    }
    bool operator!=(point const &other) const noexcept
    { return !(*this == other); }

    double x() const noexcept { return x_; }
    double y() const noexcept { return y_; }

    static bool zero(double z) noexcept { return fabs(z)<tol; }

    friend class lineseg;

    /** Tolerance for equality */
    static double tol;
    /** Tolerance for equality, squared */
    static double tol2;
};

std::ostream &operator<<(std::ostream &, point const &);

#endif //VEC2POLY_POINT_H
