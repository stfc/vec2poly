#include <iostream>
#include <sstream>
#include <boost/utility.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <map>
#include <vector>
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

// (out)edge storage is the first customisation: we use a list so we can
// add and remove edges without invalidating iterators
using Graph = boost::adjacency_list< boost::listS, boost::vecS, boost::undirectedS, VertexProp, EdgeProp >;

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
    // (these are not necessarily branch points as a path may meet only one other path)
    auto may_add_point = [&init,&vertices](pathpoint p)
    {
	if(!vertices.contains(p))
	    vertices[p] = init++;
    };
    for( auto const &p : w.paths() ) {
	auto [a, b] = p.endpoints();
	may_add_point(a);
	may_add_point(b);
    }
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


graph::graph() : impl_(std::make_unique<graphimpl>(0))
{
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
    // Failure happens if the edge already exists?
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
    // This has to be a reference because boost will copy the visitor object and
    // update the copy
    polygon &p_;
public:
    visitor(test_t testfunc, polygon &p) : p_(p), test_(testfunc) {}

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
        if( test_(boost::target(e, g)) ) throw found();
    }
private:
    test_t test_;
};


polygon graph::find_polygon() {
    // find_unused throws an exception if no path is found
    auto e = find_unused(impl_->g_);
    std::cerr << "Picked unused path " << e << std::endl;
    node_t start = boost::source(e, impl_->g_), target = boost::target(e, impl_->g_);
    EdgeProp prop = impl_->g_[e];

    auto test = [start](Vertex v) { return v == start; };
    edgelist avoid = {prop.index};
    polygon result = pathfinder(target, test, avoid);
    // The current edge will form the first/last edge of the polygon
    result.add_edge(start, target, prop.index);
    return result;
}


struct EdgeData {
    Vertex src, dst;
    EdgeProp data;
    using collect = std::vector<EdgeData>;
};

static auto save_edges(Graph &g, graph::edgelist const &edges)
{
    EdgeData::collect data;
    data.reserve(edges.size());
    // Check that iterators are not invalidated by remove_edge
    auto [p, q] = boost::edges(g);
    while(p != q) {
        auto p1 = p;
        ++p1;
        if(edges.contains(g[*p].index)) {
            Vertex src = boost::source(*p, g);
            Vertex dst = boost::target(*p, g);
            data.push_back({src, dst, g[*p]});
            g.remove_edge(*p);
        }
        p = p1;
    }
    return data;
}


static auto restore_edges(Graph &g, EdgeData::collect const &data)
{
    std::ranges::for_each(data, [&g](EdgeData const &e) { boost::add_edge(e.src, e.dst, e.data, g); });
}


polygon graph::pathfinder(node_t start, test_t goal, const graph::edgelist &avoid)
{
    polygon result(impl_->n_, start);

    // Temporarily remove the avoid graph edges (paths)
    // (the alternatives would be to either copy the graph minus the
    // to-be-avoided edges, or to weight the edges changing the weights
    // to prohibit using the to-be-avoided edges)
    auto saved = save_edges(impl_->g_, avoid);

    // For the search we need to connect start to a target point, with the
    // polygon object collecting the spanning subtree starting at start
    visitor vis(goal, result);
    try {
        boost::breadth_first_search( impl_->g_, start, boost::visitor(vis) );
    }
    catch(visitor::found) {
        // Restore the removed edges
        restore_edges(impl_->g_, saved);
        // The visitor has completed the polygon
        return result;
    }

    // Restore the removed edges
    restore_edges(impl_->g_, saved);
    throw BadGraph("no path found");

}


void graph::paths(std::function<void(edge_t)> cb, bool unused) const
{
    // See also find_unused above
    using iterator = boost::graph_traits<Graph>::edge_iterator;
    auto [p,q] = boost::edges(impl_->g_);
    while(p != q) {
        if(!unused || !impl_->g_[*p].used)
            cb(impl_->g_[*p].index);
        ++p;
    }
}


std::ostream &operator<<(std::ostream &os, graph const &g)
{
    // Invert the map of points
    std::vector<pathpoint> pts(g.impl_->n_);
    for( auto [k,v] : g.impl_->vertex_ )
        pts[v] = k;
    using iter = boost::graph_traits<decltype(g.impl_->g_)>::edge_iterator;
    auto [cur,end] = boost::edges(g.impl_->g_);
    while(cur != end) {
        auto s = boost::source(*cur, g.impl_->g_);
        auto t = boost::target(*cur, g.impl_->g_);
        os << '{' << s << '}' << pts[s] << ' ' << '{' << t << '}' << pts[t] << '\n';
        ++cur;
    }
    return os;
}
