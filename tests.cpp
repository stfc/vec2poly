//
// Created by jens on 09/09/23.
//

#include <iostream>
#include <array>
#include <map>
#include <functional>
#include <ranges>
#include <algorithm>
#include "lineseg.h"
#include "world.h"
#include "pntalloc.h"
#include "graph-path.h"
#include "polygon.h"

bool expect(pntalloc &, int i, lineseg const &, lineseg const &, std::optional<point>);

// Tests are sorted by increasing integratedness

/** Test basic pathpoint allocator */
[[nodiscard]] bool test_pntalloc();
/** Test line segments and their intersections */
[[nodiscard]] bool test_lineseg();
/** Test splitting line segment into two */
[[nodiscard]] bool test_split_seg();
/** Test inserter into path */
[[nodiscard]] bool test_poly1();
/** Test splitting paths at intersections */
[[nodiscard]] bool test_poly2();
/** Test path iterator - which iterates over segments */
[[nodiscard]] static bool test_path_iter();
/** Test finding branch points (nodes of degree > 2) */
[[nodiscard]] static bool test_branch_points();
/** Test reorganising the path into "proper" paths starting and ending in branch points */
[[nodiscard]] static bool test_path_split();
/** Testing the polygon edge iterator */
[[nodiscard]] static bool test_make_poly1();
/** Test finding a polygon in the world */
[[nodiscard]] static bool test_make_poly2();
/** Test interor points algorithm */
[[nodiscard]] static bool test_interior();
/** Test cleaning a polygon */
[[nodiscard]] static bool test_tidy_poly();
/** Test tidying algorithm */


static world make_world(int);

/** Utility function for test code to access the World's paths */
decltype(world::map_) &test_paths(world &w)
{
    return w.map_;
}


/** Utility function for test code to access the World's allocator */
pntalloc &test_allocator(world &w)
{
    return w.alloc_;
}


int tests()
{
    bool ret = true;
    unsigned num{0};
    // Tests are to be run in this order
    std::array<std::function<bool()>,12> all{test_pntalloc, test_lineseg, test_split_seg, test_poly1,
                                            test_poly2, test_path_iter, test_branch_points, test_path_split,
                                            test_make_poly1, test_make_poly2, test_interior, test_tidy_poly };
    for( auto testfunc : all ) {
        ++num;
        try {
            bool result = testfunc();
            if(!result) {
                std::cerr << "Test " << num << " failed with no uncaught exception raised\n";
                return 1;
            }
        }
        catch (BadLineSegment &bad) {
            std::cerr << "Test " << num << ": Bad Line Segment exception " << bad.what() << std::endl;
            return 1;
        }
        catch (BadPath &bad) {
            std::cerr << "Test " << num << ": BadPath exception " << bad.what() << std::endl;
            return 1;
        }
    }
    return ret ? 0 : 1;
}


bool test_pntalloc()
{
    pntalloc z(0.01);
    pathpoint u1 = z.make_point(1,2);
    pathpoint u2 = z.make_point(3,4);
    pathpoint u3 = z.make_point(1,2);
    return u1 == u3 && u1->use_count() == 2 \
        && u2->use_count() == 1 && u2 != u1;
}


bool
test_lineseg()
{
    pntalloc u(0.01);
    // a b define a line y = x/3
    // c-f are collinear, on the line y = -2x+14
    point   a = point(0, 0),
            b = point(900,300),
            c = point(400,600),
            d = point(500,400),
            e = point(600,200),
            f = point(700,0);
    const auto ab = u.make_lineseg(a, b);
    bool ret = true;
    ret &= expect(u, 1, ab, u.make_lineseg(c, f), e);
    ret &= expect(u, 2, ab, u.make_lineseg(c, d), std::nullopt);
    ret &= expect(u, 3, ab, u.make_lineseg(c, e), e);
    ret &= expect(u, 4, u.make_lineseg(a, e), u.make_lineseg(c, e), e);
    const auto gh = u.make_lineseg(u.make_point(1, 3), point{500, -100});
    ret &= expect(u, 5, ab, gh, point{300,100});
    ret &= expect(u, 6, gh, ab, point{300,100});
    return ret;
}


bool
expect(pntalloc &u, int i, lineseg const &v, lineseg const &w, std::optional<point> expt)
{
    bool ret = true;
    std::optional<point> z = intersects(v, w);
    if(z.has_value() && expt.has_value()) {
        if(z.value() != expt.value()) {
            std::cerr << "subtest " << i << " expected " << expt.value() << " got " << z.value() << std::endl;
            ret = false;
        }
    } else if(z.has_value() && !expt.has_value()) {
        std::cerr << "subtest " << i << " expected no intersection, got " << z.value() << std::endl;
        ret = false;
    } else if(!z.has_value() && expt.has_value()) {
        std::cerr << "subtest " << i << " expected " << expt.value() << " got no intersection" << std::endl;
        ret = false;
    }
    return ret;
}


bool test_split_seg()
{
    pntalloc u(0.01);
    point a(-1,-1), b(2,1), c(3, 0);
    auto ac = u.make_lineseg(a,c);
    // Splitting at b should turn a->c into a->b while returning b->c
    auto bc{ac.split_at(u,b)};
    auto a1{ac.first()}, b1{ac.last()}, b2{bc.first()}, c2{bc.last()};
    bool ret = true;
    if(*a1 != a || a1->use_count() != 1) {
        std::cerr << "split a expected " << a << "[1], got " << a1 << std::endl;
        ret = false;
    }
    if(*b1 != b || b1->use_count() != 2) {
        std::cerr << "split b expected " << b << "[2], got " << b1 << std::endl;
        ret = false;
    }
    if(b1 != b2) {
        std::cerr << "split end segment 1 doesn't match start of 2: " << b1 << ' ' << b2 << std::endl;
        ret = false;
    }
    if(*c2 != c || c2->use_count() != 1) {
        std::cerr << "split c expected " << c << "[1], got " << c2 << std::endl;
        ret = false;
    }
    return ret;
}


bool test_poly1()
{
    world w(0.01);
    pntalloc &u = test_allocator(w);
    // World is empty, so should not iterate here
    for( auto const &y : w ) {
        std::cerr << "Error: World is not empty!\n";
        return false;
    }
    point   a = {1, 2},
            b = {2,3},
            c = {3,1},
            d = {4,5},
            e = {-1,-2},
            f = {-2,-3},
            o = {0,0};
    // Two line segments: a->b and b->c
    w.add_path(path(u, {a, b, c}));
    auto q = w.begin(); // point to first segment
    ++q;                // point to second and last segment
    // Add c->d
    q.insert_after(u.make_lineseg(c,d));
    auto &map = test_paths(w);
    if( map[0] != path(u, {a, b, c, d})) {
        std::cerr << "Insertion failed: " << map[0] << '\n';
        return false;
    }
    // Add e->f as a separate path
    w.add_path(path(u, {e, f}));
    // Adding a path may have invalidated the iterator, so we refresh it
    q = w.begin(); ++q; ++q; ++q;
    // q should now reference the single line segment on the second path
    q.insert_after(u.make_lineseg(f,o));
    if(map.size() != 2) {
        std::cerr << "Expected two paths; found " << map.size() << '\n';
        return false;
    }
    if(map[0] != path(u, {a, b, c, d}) || map[1] != path(u, {e, f, o})) {
        std::cerr << "Error with paths " << map[0] << " or " << map[1] << '\n';
        return false;
    }
    // Finally, an insert at not-the-end - q still points to the *first* entry on path 2
    // This is a dummy line segment, not usually useful (or permitted)
    q.insert_after(u.make_lineseg(f,f));
    return true;
}


bool test_poly2()
{
    world w(0.01);
    pntalloc &u = test_allocator(w);
    w.add_path(path{u, {{-2, 2},{-1,2},{-1,-2},{2,-2},{2,1},{3,2}}});
    w.add_path(path{u, {{-3, 1},{3,1}}});
    w.split_segments();
    // These path comparisons reuse the same points, so only check path structure...
    auto &map = test_paths(w);
    if( map[0] != path(u, {{-2, 2},
                           {-1, 2},
                           {-1, 1},
                           {-1, -2},
                           {2,  -2},
                           {2,  1},
                           {3,  2}}) ) {
        std::cerr << "poly2 first path mismatch\n";
        std::cerr << map[0] << std::endl;
        return false;
    }
    if( map[1] != path{u, {{-3, 1},{-1,1},{2,1},{3,1}}} ) {
        std::cerr << "poly2 second path mismatch\n";
        std::cerr << map[1] << std::endl;
        return false;
    }
    // ... so the final test checks the multiplicites;
    // however, the previous tests, having visited all the points, have precisely
    // doubled all the expected multiplicities because they have the same line segments
    std::list<std::pair<point,unsigned int>> expect{{{-2,2},1},{{-1,2},2},{{-1,-2},2},{{2,-2},2},{{2,1},4},{{3,2},1},{{-3,1},1},{{-1,1},4},{{3,1},1}};
    bool ret = true;
    for( auto const y : u.points() ) {
        point p{ static_cast<point>(*y) };
        auto z = std::ranges::find_if( expect, [&p](std::pair<point,unsigned int> const &q) { return q.first == p; } );
        if(z == expect.end()) {
            std::cerr << "poly2: unexpected point " << y << '\n';
            ret = false;
        } else {
            if( 2 * z->second != y->use_count() ) {
                std::cerr << "poly2: " << y << " expected " << z->second << " count\n";
                ret = false;
            }
            expect.erase(z);
        }
    }
    if( !expect.empty() ) {
        for( const auto &[pt,val] : expect )
            std::cerr << "poly2: expected " << point(pt) << '[' << val << "]\n";
        ret = false;
    }
    return ret;
}


bool test_path_iter()
{
    world w(0.01);
    point   a = {1, 1},
            b = {2,2},
            c = {3,4},
            d = {5,6};
    w.add_path({a, b});
    w.add_path({c, d});
    std::vector<point> items;
    for( auto const &seg : w.segments() ) {
        items.push_back(*seg.first());
        items.push_back(*seg.last());
    }
    std::vector<point> expected{a, b, c, d};
    return items == expected;
}


bool test_branch_points()
{
    world w{make_world(4)};
    // Expecting {-1.0},{0,0},{1,1},{-2,1}
    std::set<point> const expect{{-1,0},{0,0},{1,1},{-2,1}};
    std::set<point> found;
    // though not necessarily in that order
    for( auto p : w.branch_points() )
        found.insert(*p);
    return found == expect;
}


static bool test_path_split1(std::vector<point> const &at, std::initializer_list<std::initializer_list<point>> y)
{
    world w{make_world(1)};
    pntalloc &u = test_allocator(w);

    // This is the actual code we're testing
    w.proper_paths(at);

    std::vector<path> expected;
    for( auto &x : y )
        expected.emplace_back(u, x);
    auto &map{test_paths(w)};
    if(map.empty()) {
        std::cerr << "pathsplit1 no paths returned - not possible!\n";
        return false;
    }

    bool ret = true;
    // Check whether expected paths match reality
    for( path const &p : map ) {
        auto found = std::ranges::find( expected, p );
        if( found == expected.end() ) {
            std::cerr << "pathsplit1 got unexpected path " << p << std::endl;
            ret = false;
        } else {
            expected.erase(found);        
        }
    }
    for( auto const &p : expected ) {
        std::cerr << "pathsplit1 expected path " << p << " not found\n";
        ret = false;
    }
    if(!ret)
        std::cerr << "pathsplit1 (used " << at << " to split)\n";
    return ret;
}


bool test_path_split()
{
    std::vector<point> at;
    at.emplace_back(-2,1); // d
    // Result should be a single path d->a->b->c->d
    if(!test_path_split1(at, {{{-2,1},{-3,2},{-3,0},{-1,0},{-2,1}}}))
        return false;
    at.clear();
    at.emplace_back(1,0);
    at.emplace_back(-3,2); // a
    at.emplace_back(-1,0); // c
    at.emplace_back(1,1);
    // Result should be two paths a->b->c and c->d->a
    if(!test_path_split1(at, {{{-3,2},{-3,0},{-1,0}},{{-1,0},{-2,1},{-3,2}}}))
        return false;
    at.clear();
    at.emplace_back(-1,0); // c
    at.emplace_back(-3,0); // b
    // Result should be two paths b->c and c->d->a->b
    if(!test_path_split1(at,{{{-3,2},{-3,0}},{{-3,0},{-1,0}},{{-1,0},{-2,1},{-3,2}}}))
        return false;
    return true;
}


bool test_make_poly1()
{
    polygon p(4,0);
    p.add_edge(0,1,100);
    p.add_edge(1,2,101);
    p.add_edge(2,3,102);
    p.add_edge(3,0,103);
    auto it = p.begin(), end = p.end();
    for(int i = 1; i <= 4; ++i) {
        if(it == end) {
            std::cerr << "poly1 premature end of iterator at iteration " << i << std::endl;
            return false;
        }
        auto v = *it;
        // The iterator works *backwards* through the edges
        if(v != 104-i) {
            std::cerr << "poly1 iteration " << i << " got " << v << std::endl;
            return false;
        }
        ++it;
    }
    if(it != end) {
        std::cerr << "poly1 expected iterator end" << std::endl;
        return false;
    }
    return true;
}


bool test_make_poly2()
{
    world w{make_world(4)};
    w.proper_paths();
    graph g(w);
    try {
        auto poly = g.find_polygon();
        auto statws = poly.is_valid(w);
        if(statws != poly_errno_t::POLY_GOOD) {
            std::cerr << "poly2: find poly invalid polygon: " << poly_errno_string(statws) << std::endl;
            return false;
        }
    }
    catch( graph::AllDone )
    {
        std::cerr << "poly2: unexpected early end of paths\n";
        return false;
    }
    catch( BadGraph const &e )
    {
        std::cerr << "poly2: no path found or other graph exception: " << e.what() << std::endl;
        return false;
    }
    return true;
}


/* Helper function for test_interior testing the helper function for interior... */
bool test_interior1()
{
    pntalloc p(0.01);
    auto w = p.make_lineseg({-1,2},{1,-2});
    // 15 is -1 mod 16
    std::vector<unsigned> expect{0,15,2,2,0,0,0};;
    for(int x=-4, y=-3; y <= 3; ++x, ++y) {
        point p{x,y};
        if(expect.empty()) {
            std::cerr << "test_int1 ran out of expected values?\n";
            return false;
        }
        auto u = intersects(w, p);
        if(u != expect.front()) {
            std::cerr << "test_int1 point " << p << " expected " << expect.front()
                      << " got " << u << '\n';
            return false;
        }
        expect.erase(expect.begin());
    }
    if(!expect.empty()) {
        std::cerr << "test_int1 not all test cases covered\n";
        return false;
    }
    return true;
}


/* Helper function for test_interior testing polynomial */
static bool test_interior2()
{
    world w{make_world(4)};
    w.proper_paths();
    // Graph values refer to those documented in the comment for make_world
    polygon p(4, 0);
    // Add edge 3: c->h or (-1,0) -> (0,0)
    p.add_edge(0, 3, 3);
    // Add edge 2: h->i->g or (0,0) -> (1,0) -> (1,1)
    p.add_edge(3, 2, 2);
    // Add edge 1: d->e->f->g or (-2,1) -> (-1,2) -> (0,2) -> (1,1)
    p.add_edge(2, 1, 1);
    // Add edge 5: c->d or (-1,0) -> (-2,1)
    p.add_edge(1, 0, 5);
    auto status = p.is_valid(w);
    if(status == poly_errno_t::POLY_GOOD)
        return true;
    std::cerr << "int2 Polygon is invalid: " << poly_errno_string(status) << "\n";
    return false;
}


/** Another helper function testing the polygon::interior function */
static bool test_interior3()
{
    world w(0.01);
    // Unlike any other polygon we work with, this one has a hole
    //w.add_path({{1,0},{0,1},{-1,0},{0,-1},{1,0}});
    w.add_path({{-1,3},{2,2},{5,0},{3,0},{5,-2},
                {2,-1},{-1,-2},{-4,0},{-3,0},{-4,2},{-1,3}});
    // Size is 1 because we have only one "branch point" (the chosen start/end point0
    // and only one path
    polygon poly(1,0);
    poly.add_edge(0,0,0);
    // Tuple of X, Y, Expected.
    // Expected is 0=Exterior, 1=Interior, -1=(Possible) Exception
    auto say = [](int expd) -> char const *
    {
        switch(expd) {
            case -1: return "exception";
            case  0: return "exterior";
            case  1: return "interior";
        }
        return "canthappen";
    };
    // {4,0,-1} removed
    std::vector<std::tuple<int,int,int>> const testdata =
            {{0,0,1},{1,1,1},{-1,2,1},{-2,0,1},{-4,1,0},{-2,-2,0},{-1,-1,1},
             {2,-2,0},{3,-1,1},{4,1,0},{3,2,0}};
    for( auto const &data : testdata ) {
        point p{std::get<0>(data),
                std::get<1>(data)};
        try {
            auto val = poly.interior(w, p);
            if(val != (std::get<2>(data) == 1)) {
                std::cerr << "int3 at " << p << " got " << say(val ? 1 : 0)
                          << " expected " << say(std::get<2>(data)) << '\n';
                return false;
            }
        }
        catch (...) {
            if(std::get<2>(data) != 2) {
                std::cerr << "int3 at " << p << " got exception, expected "
                          << say(std::get<2>(data)) << '\n';
                return false;
            }
        }
    }
    return true;
}


bool test_interior()
{
    return test_interior1() && test_interior2() && test_interior3();
}


bool test_tidy_poly()
{
    world w{make_world(4)};
    w.proper_paths();
    graph g(w);
    // See description of node indices in doc for make_world
    // Four points, start at 0
    polygon poly(4,0);
    // These are a subset of the "proper" paths to form a non-minimal polygon
    poly.add_edge(1, 0, 0);      // 0->1
    poly.add_edge(1, 2, 1);      // 1->2
    poly.add_edge(3, 2, 2);      // 2->3
    poly.add_edge(0, 3, 3);      // 3->0
    //if(!poly.is_valid()) 
    for( auto const &y : poly.interior_paths(w) ) {
        std::cerr << y << std::endl;
    }
    return true;
}



/* make_world returns a test setup with k paths (1..4)
 * Path 1 is a triangle abcda
 * Path 2 is a line defg
 * Path 3 is another triangle ihgi
 * Path 4 is another path between them, making three polygons
 *
 * The branching points for k==1 are {d,g}; for k==2, {c,d,g,h}
 * The "proper paths" (possibly reversed) derived from the four paths
 * for k==2 are:
 * 1a. cbad
 * 1b. cd
 * 2. defg
 * 3a. hg
 * 3b. gih
 * 4. ch
 *
 * Once converted to a graph, it looks like
 *         3
 *    0 -------> 3
 *   / \        / \
 * 5 V ^ 0    2 V ^ 4
 *   \ /        \ /
 *    1 -------> 2
 *         1
 * where node 0 is c=(-1,0); 1 is d=(-2,1); 2 is g=(1,1); 3 is h=(0,0)
 * and the edge numbers denote the initial order assigned by the graph object.
 * Note that edges are undirected, though they are saved as if they were directed as above.
 *
 * After the "proper paths" call is run, the paths between branch points (in the current
 * implementation) will be:
 * Path 0: dabc or 1->0
 * Path 1: defg or 1->2
 * Path 2: hig or 3->2
 * Path 3: ch or 0->3
 * Path 4: gh or 2->3
 * Path 5: cd or 0->1
 */

world make_world(int k)
{
    world w(0.01);
    // Triangle one
    point c = point(-1, 0), b = point(-3, 0), a = point(-3, 2);
    // Triangle two
    point h = point(0, 0), i = point(1, 0), g = point(1, 1);
    // Connecting line
    point d = point(-2, 1), e = point(-1, 2), f = point(0, 2);
    w.add_path({a, b, c, d, a});
    if(k>=2)
        w.add_path({d, e, f, g});
    if(k>=3)
        w.add_path({i, g, h, i});
    if(k == 4) {
        w.add_path({c, h});
    }
    return w;
}

