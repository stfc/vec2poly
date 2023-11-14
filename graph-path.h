//
// Created by jens on 03/11/23.
//

#ifndef VEC2POLY_GRAPH_PATH_H
#define VEC2POLY_GRAPH_PATH_H

#include <vector>
#include <ranges>
#include "lineseg.h"

typedef unsigned long node_t;
typedef unsigned long edge_t;


class BadGraph : public std::exception
{
    char const *msg_;
public:
    BadGraph(char const *msg) noexcept : msg_(msg) {}
    char const *what() const noexcept override { return msg_; }
};

class polygon {
    /** The ith index is j if node i is first reached from j */
    std::vector<node_t> come_from_;
    /** Look up edge number from vertex number
     * The index is the node number of the destination node
     */
    std::vector<edge_t> edges_;
    /** Start and end node of this polygon */
    node_t start_;
    /** Invalid node number
     * This is used for uninitialised nodes (as opposed to using -1, say, which is not a valid node_t)
     */
    node_t invalid_;

public:
    /** Create a polygon of N vertices
     * @param N number of vertices or equivalently number of edges
     * @param start node number of start vertex
     */
    polygon(std::size_t N, node_t start) : come_from_(N, N), edges_(N, N), start_(start), invalid_(N) {}

    /** Add a new edge to dst from src with edge number e
     * If dst already has an edge to it, do nothing, as the first one is best
     */
    void add_edge(node_t src, node_t dst, edge_t e)
    {
        if(come_from_[dst] == invalid_) {
            come_from_[dst] = src;
            edges_[dst] = e;
        }
    }

    /** Iterator over the paths (indices) that make up the polygon */
    class poly_iterator {
        polygon const *poly_;
        node_t cur_;
    public:
        poly_iterator(polygon const *p) : poly_(p), cur_(p->start_) {}
        poly_iterator(polygon const *p, node_t init) : poly_(p), cur_(init) {}
        edge_t operator*() const { return poly_->edges_[cur_]; }
        poly_iterator &operator++()
        {
            cur_ = poly_->come_from_[cur_];
            if(cur_ == poly_->start_)
                cur_ = poly_->invalid_;
            return *this;
        }
        poly_iterator operator++(int)
        {
            auto i{*this};
            ++(*this);
            return i;
        }
        bool operator==(poly_iterator const &o)
        {
            return cur_ == o.cur_;
        }
    };

    /** Begin iterator */
    [[nodiscard]] poly_iterator begin() const { return {this}; }
    /** End iterator */
    [[nodiscard]] poly_iterator end() const { return {this, invalid_}; }

};


struct graphimpl;

/** The graph class bridges the world class' view with boost's
 *
 * In the world class, vertices are branch points and edges are proper paths.
 * Branch points are of type pathpoint; edges are paths represented by integer
 * indices into world's internal array.
 * In the boost view, all are unsigned ints.
 *
 * The main job of the class is to find candidate polygons.  This is done by:
 * 1. Finding a path not yet used in a polygon
 * 2. Finding an alternative route to this path's start and end
 * 3. Combining the results of 1 and 2 into a polygon
 *
 * Boost is used for step 2
 */
class graph {
    std::unique_ptr<graphimpl> impl_;
public:
    graph(world &w);
    graph(const graph &) = delete;
    graph(graph &&);
    graph &operator=(graph const &) = delete;
    graph &operator=(graph &&);
    ~graph();

    /** Return the node number of a given world point */
    node_t vertex(pathpoint);

    void add_path(path const &, edge_t);

    polygon find_polygon();
};

#endif //VEC2POLY_GRAPH_PATH_H
