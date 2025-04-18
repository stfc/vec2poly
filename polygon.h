//
// Created by jens on 29/11/23.
//

#ifndef VEC2POLY_POLYGON_H
#define VEC2POLY_POLYGON_H

#include <iosfwd>
#include "point.h"
#include "except.h"

// world is defined in world.h
class world;

/** node_t is the node (vertex) index in the graph as assigned by
 * the graphimpl object (and ultimately by boost).
 *
 * Vertices are normally the branch points, numbered 0 up to N-1.
 * In addition to (reverse) lookup in graphimpl.vertex_, they will
 * also be the end points of the paths (edges).
 */
typedef unsigned long node_t;

/** edge_t is the edge index as maintained by the world object.
 * After proper_paths() is run (deriving paths between branch points),
 * the edges remain fixed in world.map_
 */
typedef unsigned long edge_t;


/** Polygon diagnostics returned by is_valid */

class poly_valid_t {
public:
    enum class poly_errno_t {
        /** Polygon is good */
        POLY_GOOD,
        /** Polygon is good but last point of last path is not the same as first point of first path */
        POLY_NOTCLOSED,
        /** Polygon has a path disconnected from its neighbour */
        POLY_BROKENPATH,
        /** Polygon is a single point (no paths) */
        POLY_EMPTY,
        /** Polygon intersects itself somewhere a la figure 8 */
        POLY_SELFINTERSECT
    };

    poly_valid_t(poly_errno_t t) : code_(t) {}
    /** Explanation string */
    char const *what() const noexcept;

    poly_errno_t code() const noexcept { return code_; }

    /** As boolean - is it valid? */
    operator bool() const noexcept { return code_ == poly_errno_t::POLY_GOOD; }

private:
    poly_errno_t code_;
};


class BadPolygon : public Vec2PolyException {
    poly_valid_t err_;
public:
    BadPolygon(poly_valid_t::poly_errno_t err): err_(err) {}
    [[nodiscard]] char const *what() const noexcept override { return err_.what(); }
    [[nodiscard]] poly_valid_t::poly_errno_t get_errno() const noexcept { return err_.code(); }
};

// This is needed to inline polygon::interior_paths which in turn is needed to deduce its return type
#include "world.h"

// defined in graph-path.h, used to tidy/shrink polynomials
class graph;

/** A polygon class for connecting paths.
 *
 * It records a spanning subtree of a graph, eventually hitting the target node.
 * While originally designed to hold polygons (closed sequences of paths),
 * it has been refactored to support pathfinding in general.
 */
class polygon {
    /** The ith index is j if node i is first reached from j.
     * (The equivalent of the predecessor array in Dijkstra shortest path) */
    std::vector<node_t> come_from_;
    /** Look up edge number from vertex number
     * The index is the node number of the destination node
     */
    std::vector<edge_t> edges_;
    /** Start and end node of this polygon (if closed) */
    node_t start_;
    /** Invalid node number
     * This is used for uninitialised nodes (as opposed to using -1, say, which is not a valid node_t)
     */
    node_t invalid_;

    /** Replace a sequence of paths with another in the polygon.
     *
     * Prerequisites:
     * 1. The given sequences of paths (trails) connect to each other, meaning the start or end point
     *    of one equals the start or end point of the other trail (albeit possibly in reverse)
     * 2. The unconnected points on the first and last path, ie the endpoints of the trail
     *    are nodes on the polygon.
     *
     * Paths are specified by their edge number ie index into the world's list of all paths.
     * The latter is not modified.
     *
     * The polygon is altered as follows: the two points where the given path sequence
     * intersect the polygon are used to split the polygon into two sequences of paths.
     * The given path will replace one of these two path sequences in the polygon,
     * thus shrinking the polygon if the argument paths are internal.
     *
     * The choice of which of the two possible polygons to keep is based on the keep path
     * */
    void replace_paths(world const &w, std::vector<edge_t> &paths, edge_t keep);

public:
    /** Create a polygon of N vertices.
     * @param N number of vertices or equivalently number of edges
     * @param start node number of start vertex
     */
    polygon(std::size_t N, node_t start) : come_from_(N, N), edges_(N, N), start_(start), invalid_(N) {}

    /** Add a new edge to dst from src with edge number e.
     * If dst already has an edge to it, do nothing, as the first one is best
     * - unless we're returning to the beginning!
     */
    void add_edge(node_t src, node_t dst, edge_t e)
    {
        if(come_from_[dst] == invalid_ || dst == start_) {
            come_from_[dst] = src;
            edges_[dst] = e;
        }
    }

    /** Iterator over the paths (indices) that make up the polygon */
    class poly_iterator {
        polygon const *poly_;
        node_t init_, cur_;
    public:
        poly_iterator(polygon const *p) : poly_(p), init_(p->start_), cur_(p->start_) {}
        poly_iterator(polygon const *p, node_t init) : poly_(p), init_(init), cur_(init) {}
        edge_t operator*() const { return poly_->edges_[cur_]; }
        poly_iterator &operator++()
        {
            cur_ = poly_->come_from_[cur_];
            if(cur_ == init_)
                cur_ = poly_->invalid_;
            return *this;
        }
        poly_iterator operator++(int)
        {
            auto i{*this};
            ++(*this);
            return i;
        }
        bool operator==(poly_iterator const &o) const
        {
            return cur_ == o.cur_;
        }
    };

    /** Begin iterator */
    [[nodiscard]] poly_iterator begin() const { return {this}; }
    /** End iterator */
    [[nodiscard]] poly_iterator end() const { return {this, invalid_}; }

    /** Check polygon is OK (starts at the start, ends at the start).
     * It assumes all paths are proper. */
    poly_valid_t is_valid(const world &) const noexcept;

    /** Use all world paths to tidy a polygon */
    void tidy(world const &, graph const &);

    /** Size of polygon (number of points).
     * Note may be expensive to run (as in O(N))
     */
    size_t size(world const &w) const noexcept;

    /** Is a point interior to the polygon?
     * Note that all edges (in edges_) must be valid at this point
     * ie the polygon building must be fully finished.
     * Also, the point being tested must not lie on the polygon itself. */
    bool interior(const world &w, point p) const;

    /** Identify interior paths
     *
     * Interior paths are candidates from removal from a polygon.
     * This implementation needs to be here so the compiler can pick up the return type
     */
    auto interior_paths(world const &w)
    {
        /** All paths are
         *  1. On the polygon
         *  2. Wholly outside the polygon
         *  3. Wholly inside the polygon
         * We now need to connect paths inside the polygon and use them to
         * reduce the polygon
         */
         edge_t index{0};

         // Without an indexed iterator (it's in C++23) we have to do it ourselves
         // - tracking the index we identify paths on the polygon
         auto is_interior = [this,&w,&index](path const &path) -> bool
         {
             auto p = this->edges_.cbegin(), q = this->edges_.cend();
             // path index is not on the polygon list (meaning path is not on polygon)
             // and a path test point is interior to the polygon
             return std::find(p,q,index++) == q && this->interior(w, path.testpoint());
         };

        return w.paths()
                | std::views::filter(is_interior);
    }

    friend std::ostream &operator<<(std::ostream &, polygon const &);
};


/** Adaptor object to convert a edge number (edge_t) to an actual path */
class path_lookup final {
    world const &w_;
public:
    path_lookup(world const &w): w_(w) {}
    path const &operator()(edge_t e) const { return w_.map_[e]; }
};


/** Utility class to "walk" through a trail - a sequence of paths, calling a callback on each point on the trail.
 *
 * For a trail consisting of N paths, the callback will be called N+1 times.
 * The problem is that some paths need to be traversed backwards as they are bidirectional.
 */
class trail_walk {
public:
    using callback_t = std::function<void(pathpoint)>;
private:
    path_lookup lookup_;
    callback_t cb_;
public:
    trail_walk(world const &w, callback_t cb): lookup_(w), cb_(cb) {}
    [[nodiscard]] std::pair<pathpoint,pathpoint> walk(polygon::poly_iterator begin, polygon::poly_iterator end);
};


#endif //VEC2POLY_POLYGON_H
