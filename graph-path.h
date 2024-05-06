//
// Created by jens on 03/11/23.
//

#ifndef VEC2POLY_GRAPH_PATH_H
#define VEC2POLY_GRAPH_PATH_H

#include <set>
#include <ranges>
#include <iosfwd>
#include "lineseg.h"
#include "except.h"
#include "polygon.h"

using test_t = std::function<bool(node_t)>;

struct BadGraph : public Vec2PolyException
{
    BadGraph(std::string &&msg) : Vec2PolyException(std::forward<std::string>(msg)) {}
    BadGraph(char const *msg) : Vec2PolyException(msg) {}
    BadGraph(std::string_view msg) : Vec2PolyException(msg) {}
};


struct graphimpl;

/** The graph class bridges the world class' view with boost's
 *
 * In the world class, vertices are branch points and edges are proper paths.
 * Branch points are of type pathpoint; edges are paths represented by integer
 * indices into world's interior array.
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
    /** pimpl hides boost layer */
    std::unique_ptr<graphimpl> impl_;
public:
    graph(world &w);
    graph(const graph &) = delete;
    graph(graph &&);
    graph &operator=(graph const &) = delete;
    graph &operator=(graph &&);
    ~graph();

    /** Dummy exception for find_unused to signal nothing else to do */
    struct AllDone
    {
    };

    using edgelist = std::set<edge_t>;

    /** Return the node number of a given world point */
    node_t vertex(pathpoint);

    void add_path(path const &, edge_t);

    /** Find a polygon based on an unused path.
     * @returns polygon object
     * @returns Throws AllDone when no further polygons found 
     * @returns Throws BadGraph when a path cannot be used in a polygon
     */
    polygon find_polygon();

    /** Find a path to a particular node or set of nodes
     *
     * @param start node index of start point
     * @param goal called as each node is added to the subtree, returning bool if it's a target
     * @param avoid edge numbers of paths to exclude
     * @return list of edge numbers of edges in path
     */
    polygon pathfinder(node_t start, test_t goal, const graph::edgelist &avoid);

    /** Call a callback for each path/edge number.
     * The second parameter says to call only for unused paths
     */
    void paths(std::function<void(edge_t)>, bool) const;

    friend std::ostream &operator<<(std::ostream &, graph const &);
};

#endif //VEC2POLY_GRAPH_PATH_H
