//
// Created by jens on 03/11/23.
//

#ifndef VEC2POLY_GRAPH_PATH_H
#define VEC2POLY_GRAPH_PATH_H

#include <vector>
#include <ranges>
#include <iosfwd>
#include "lineseg.h"
#include "except.h"
#include "polygon.h"


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


    /** Return the node number of a given world point */
    node_t vertex(pathpoint);

    void add_path(path const &, edge_t);

    /** Find a polygon based on an unused path.
     * @returns polygon object
     * @returns Throws AllDone when no further polygons found 
     * @returns Throws BadGraph when a path cannot be used in a polygon
     */
    polygon find_polygon();

    friend std::ostream &operator<<(std::ostream &, graph const &);
};

#endif //VEC2POLY_GRAPH_PATH_H
