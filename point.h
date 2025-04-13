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
#include <optional>


/** Check between-ness (doesn't seem to be in std?).
 *
 * @tparam NUM Comparable numeric type
 * @param a lower or upper bound
 * @param x thing-to-test for betweenness
 * @param b upper or lower bound
 * @return true if strictly between
 * Does not support types whose comparison is not noexcept
 */

template<typename NUM>
concept noexcept_compare = requires(NUM a, NUM b)
{
    noexcept(a < b);
};

template<typename NUM>
requires std::totally_ordered<NUM> && noexcept_compare<NUM>
inline bool between(NUM a, NUM x, NUM b) noexcept
{
    return a < x && x < b || b < x && x < a;
}


/** node_t is the node (vertex) index in the graph as assigned by
 * the graphimpl object (and ultimately by boost).  This value is
 * used only by the graph but may be cached with the pathpoint.
 *
 * Vertices are normally the branch points, numbered 0 up to N-1.
 * In addition to (reverse) lookup in graphimpl.vertex_, they will
 * also be the end points of the paths (edges).
 */
typedef unsigned long node_t;

/** edge_t is the edge index as maintained by the world object.
 * This value is used by the graph class, but can usefully be cached by the path.
 * After proper_paths() is run (deriving paths between branch points),
 * the edges remain fixed in world.map_
 */
typedef unsigned long edge_t;





class pntalloc;


/** point is a POD class holding the basic version of point */
class point {
private:
    /** x and y coordinates of point */
    signed long x_, y_;
public:
    constexpr point(signed long x, signed long y): x_(x), y_(y) {}

    constexpr bool operator==(point const &other) const noexcept = default;
    constexpr bool operator!=(point const &other) const noexcept = default;
    constexpr bool operator<(point const &other) const noexcept
    {
        if(*this == other) return false;
        if(x_ < other.x_) {
            if(y_ < other.y_)
                return true;
        }
        return x_ == other.x_ && y_ < other.y_;
    }

    [[nodiscard]] constexpr auto x() const noexcept { return x_; }
    [[nodiscard]] constexpr auto y() const noexcept { return y_; }

};


std::ostream &operator<<(std::ostream &, point const &);

std::ostream &operator<<(std::ostream &, std::vector<point> const &);




class xpathpoint;
using pathpoint = xpathpoint *;

/** pathpoint is the point inside of a path */
class xpathpoint : public point
{
    std::optional<node_t> node_;
    unsigned use_count_;
public:
    xpathpoint(double x, double y) : point(x, y), node_(std::nullopt), use_count_(1) {}
    xpathpoint(point bp) : point(bp), node_(std::nullopt), use_count_(1) {}
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

    [[nodiscard]] std::optional<node_t> get_node() const noexcept { return node_; }
    void set_node(node_t n) noexcept { node_ = n; }

    friend std::ostream &operator<<(std::ostream &, pathpoint);
};


#endif //VEC2POLY_POINT_H
