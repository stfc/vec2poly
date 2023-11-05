#include <iostream>
#include <boost/utility.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <map>
#include "graph-path.h"
#include "world.h"


using Graph = boost::adjacency_list< boost::vecS, boost::vecS, boost::undirectedS >;

using Edge = std::pair<int, int>;

using Vertex = boost::graph_traits<Graph>::vertex_descriptor;


struct graphimpl
{
    /** Boost implementation of graph */
    Graph g_;
    /** Nodes are non-negative integers */
    std::map<pathpoint,node_t> vertex_;
    /** Track whether a path has been used */
    std::vector<bool> used_;
    /** Value of next new node to be added
     * or, equivalently, the size (number of nodes) */
    node_t n_;
    graphimpl(node_t size) : g_(size), vertex_(), used_(size, false), n_(size) {}
};


/** Helper function for constructing graphimpl
 *
 * @param w World reference not modified
 * @return Unique pointer to graphimpl
 */

static std::unique_ptr<graphimpl> make_graphimpl(world &w)
{
    node_t init(0);
    std::map<pathpoint,node_t> vertices;
    // First we get the size of the world by creating all the nodes
    // - note all branch points should be unique
    for( auto p : w.branch_points() )
        vertices[p] = init++;
    auto ret = std::make_unique<graphimpl>(init);
    std::swap(ret->vertex_, vertices);
    return ret;
}


graph::graph(world &w) : impl_(make_graphimpl(w))
{
    /* At this point in construction, the graph has been set up but has no edges */

}

// Why are these here and not in the header?  See Meyers' Modern C++ item 22
graph::graph(graph &&) = default;

graph &graph::operator=(graph &&) = default;

graph::~graph() = default;


void graph::add_path(const path &p)
{

}


node_t graph::vertex(pathpoint p)
{
    auto q = impl_->vertex_.find(p);
    if(q == impl_->vertex_.end())
        throw BadGraph("Unknown vertex");
    return q->second;
}


class visitor : public boost::default_bfs_visitor
{
    Vertex tgt_;
    // This has to be a reference because boost will copy the visitor object and
    // update the copy
    polygon &p_;
public:
    visitor(Vertex target, polygon &p) : tgt_(target), p_(p) {}

    class found {};

    /** tree edge is called by boost whenever an edge is added to the spanning subtree */
    void tree_edge( auto e, Graph const &g )
    {
        auto src = static_cast<node_t>(boost::source(e,g)), dst = static_cast<node_t>(boost::target(e, g));
        // insert if missing but do not overwrite existing entry
        p_.add_edge(src, dst, edge);
        /* Shortcut spanning subtree generation after we find the target */
        if( boost::target(e, g) == tgt_ ) throw found();
    }

};



polygon graph::find_polygon()
{
    Vertex start{0}; // FIXME
    polygon result(impl_->n_, start);
    visitor vis(5, result);
    try {
        boost::breadth_first_search( impl_->g_, start, boost::visitor(vis) );
    }
    catch(visitor::found) {
        return result;
    }

    std::cerr << "Not found\n";

}