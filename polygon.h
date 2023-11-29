//
// Created by jens on 29/11/23.
//

#ifndef VEC2POLY_POLYGON_H
#define VEC2POLY_POLYGON_H

#include <iosfwd>


typedef unsigned long node_t;
typedef unsigned long edge_t;


class polygon {
    /** The ith index is j if node i is first reached from j.
     * (The equivalent of the predecessor array in Dijkstra shortest path) */
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

    friend std::ostream &operator<<(std::ostream &, polygon const &);
};


#endif //VEC2POLY_POLYGON_H