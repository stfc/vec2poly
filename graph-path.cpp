#include <iostream>
#include <sstream>
#include <boost/utility.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <map>
#include "graph-path.h"
#include "world.h"


struct VertexProp
{
};

struct EdgeProp
{
    edge_t index;
    bool used;
    // Discourage empty construction
    EdgeProp() = delete;
    explicit EdgeProp(edge_t i, bool u = false) : index(i), used(u) {}
};

using Graph = boost::adjacency_list< boost::vecS, boost::vecS, boost::undirectedS, VertexProp, EdgeProp >;

using Edge = std::pair<int, int>;

using Vertex = boost::graph_traits<Graph>::vertex_descriptor;



struct graphimpl
{
    /** Boost implementation of graph */
    Graph g_;
    /** Nodes are non-negative integers */
    std::map<pathpoint,node_t> vertex_;
    /** Value of next new node to be added
     * or, equivalently, the size (number of nodes) */
    node_t n_;
    graphimpl(node_t size) : g_(size), vertex_(), n_(size) {}
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


/** Helper function for graph::find_polygon
 * Find a path not yet used in a polygon
 */
static auto find_unused(Graph &g)
{
    using iterator = boost::graph_traits<Graph>::edge_iterator;
    std::pair<iterator,iterator> range = boost::edges(g);
    iterator e = std::find_if( range.first, range.second,
            [&g](auto r) { return !g[r].used; }
            );
    if(e == range.second)
        throw graph::AllDone();
    return *e;
}


graph::graph(world &w) : impl_(make_graphimpl(w))
{
    /* At this point in construction, the graph has been set up but has no edges
     * Now add the edges - which will also create the vertices - in the graph
     */
    edge_t e{0};  // edge indices start from 0 (like an indexed view)
    for( path const &y : w.paths() )
        add_path( y, e++ );
}

// Why are these here and not in the header?  See Meyers' Modern C++ item 22
graph::graph(graph &&) = default;

graph &graph::operator=(graph &&) = default;

graph::~graph() = default;


void graph::add_path(const path &p, edge_t i)
{
    auto endpoints = p.endpoints();
    Vertex src = vertex(endpoints.first), dst = vertex(endpoints.second);
    EdgeProp data{i, false};
    auto flops = boost::add_edge(src, dst, data, impl_->g_);
    // Failure happens if the edge already exists
    if(!flops.second)
        throw BadGraph("failed to add edge");
}


node_t graph::vertex(pathpoint p)
{
    auto q = impl_->vertex_.find(p);
    if(q == impl_->vertex_.end()) {
	std::ostringstream msg;
	msg << "Unknown vertex (did you \"properise\" the paths?): ";
	msg << p;
        throw BadGraph(msg.str());
    }
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
        // Upstream edge index from the bundled property
        edge_t edge = g[e].index;
        // insert if missing but do not overwrite existing entry
        p_.add_edge(src, dst, edge);
        /* Shortcut spanning subtree generation after we find the target */
        if( boost::target(e, g) == tgt_ ) throw found();
    }

};



polygon graph::find_polygon()
{
    // find_unused throws an exception if no path is found
    auto e = find_unused(impl_->g_);
    std::cerr << "Picked unused path " << e << std::endl;
    node_t start = boost::source(e, impl_->g_), target = boost::target(e, impl_->g_);

    polygon result(impl_->n_, start);
    // The current edge will form the first edge of the polygon
    result.add_edge(start, target, impl_->g_[e].index);

    // For the search we need to connect the target to the starting point
    // because we are making a return path around the unused edge
    visitor vis(start, result);

    // XXX temporarily remove the current edge/path to find alternative path
    try {
        boost::breadth_first_search( impl_->g_, start, boost::visitor(vis) );
    }
    catch(visitor::found) {
        return result;
    }

    throw BadGraph("no polygon found");

}
