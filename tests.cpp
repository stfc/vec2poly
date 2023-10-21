//
// Created by jens on 09/09/23.
//

// TODO migrate to gtest?
// #include <gtest/gtest.h>
#include <iostream>
#include <array>
#include <map>
#include <functional>
#include <ranges>
#include "lineseg.h"
#include "world.h"
#include "pntalloc.h"

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
[[nodiscard]] static bool test_make_poly1();
[[nodiscard]] static bool test_make_poly2();

static world make_world(int);



int tests()
{
    bool ret = true;
    unsigned num{0};
    // Tests are to be run in this order
    std::array<std::function<bool()>,8> all{test_pntalloc, test_lineseg, test_split_seg, test_poly1,
                                            test_poly2, test_path_iter, test_branch_points, test_path_split};
    for( auto testfunc : all ) {
        ++num;
        try {
            bool result = testfunc();
            if(!result) {
                std::cerr << "Test " << num << " failed with no exception raised\n";
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
    pntalloc z;
    pathpoint u1 = z.make_point(1,2);
    pathpoint u2 = z.make_point(3,4);
    pathpoint u3 = z.make_point(1,2);
    return u1 == u3 && u1->use_count() == 2 \
        && u2->use_count() == 1 && u2 != u1;
}


bool
test_lineseg()
{
    pntalloc u;
    // a b define a line y = x/3
    // c-f are collinear, on the line y = -2x+14
    point   a = point(0, 0),
            b = point(9,3),
            c = point(4,6),
            d = point(5,4),
            e = point(6,2),
            f = point(7,0);
    lineseg ab(u, a, b);
    bool ret = true;
    ret &= expect(u, 1, ab, lineseg(u, c, f), e);
    ret &= expect(u, 2, ab, lineseg(u, c, d), std::nullopt);
    ret &= expect(u, 3, ab, lineseg(u, c, e), e);
    ret &= expect(u, 4, lineseg(u, a, e), lineseg(u, c, e), e);
    lineseg gh(u, point(1, 3), point(5, -1));
    ret &= expect(u, 5, ab, gh, point(3,1));
    ret &= expect(u, 6, gh, ab, point(3,1));
    return ret;
}


bool
expect(pntalloc &u, int i, lineseg const &v, lineseg const &w, std::optional<point> expt)
{
    bool ret = true;
    std::optional<point> z = intersects(v, w);
    if(z.has_value() && expt.has_value()) {
        if(z.value() != expt.value()) {
            std::cerr << "Test " << i << " expected " << expt.value() << " got " << z.value() << std::endl;
            ret = false;
        }
    } else if(z.has_value() && !expt.has_value()) {
        std::cerr << "Test " << i << " expected no intersection, got " << z.value() << std::endl;
        ret = false;
    } else if(!z.has_value() && expt.has_value()) {
        std::cerr << "Test " << i << " expected " << expt.value() << " got no intersection" << std::endl;
        ret = false;
    }
    return ret;
}


bool test_split_seg()
{
    pntalloc u;
    point a(-1,-1), b(2,1), c(3, 0);
    lineseg ac(u,a,c);
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
    world w;
    pntalloc &u = w.alloc_;
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
    ++q;		// point to second and last segment
    // Add c->d
    q.insert_after(lineseg(u,c,d));
    if( w.map_[0] != path(u, {a, b, c, d})) {
        std::cerr << "Insertion failed: " << w.map_[0] << '\n';
        return false;
    }
    // Add e->f as a separate path
    w.add_path(path(u, {e, f}));
    // Adding a path may have invalidated the iterator, so we refresh it
    q = w.begin(); ++q; ++q; ++q;
    // q should now reference the single line segment on the second path
    q.insert_after(lineseg(u,f,o));
    if(w.map_.size() != 2) {
        std::cerr << "Expected two paths; found " << w.map_.size() << '\n';
        return false;
    }
    if(w.map_[0] != path(u, {a, b, c, d}) || w.map_[1] != path(u, {e, f, o})) {
        std::cerr << "Error with paths " << w.map_[0] << " or " << w.map_[1] << '\n';
        return false;
    }
    // Finally, an insert at not-the-end - q still points to the *first* entry on path 2
    // This is a dummy line segment, not usually useful (or permitted)
    q.insert_after(lineseg(u,f,f));
    return true;
}


bool test_poly2()
{
    world w;
    pntalloc &u = w.alloc_;
    w.add_path(path{u, {{-2, 2},{-1,2},{-1,-2},{2,-2},{2,1},{3,2}}});
    w.add_path(path{u, {{-3, 1},{3,1}}});
    w.split_segments();
    // These path comparisons reuse the same points, so only check path structure...
    if( w.map_[0] != path(u, {{-2, 2},
                              {-1, 2},
                              {-1, 1},
                              {-1, -2},
                              {2,  -2},
                              {2,  1},
                              {3,  2}}) ) {
        std::cerr << "poly2 first path mismatch\n";
        std::cerr << w.map_[0] << std::endl;
        return false;
    }
    if( w.map_[1] != path{u, {{-3, 1},{-1,1},{2,1},{3,1}}} ) {
        std::cerr << "poly2 second path mismatch\n";
        std::cerr << w.map_[1] << std::endl;
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
    world w;
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
    // Expecting {-1,0},{0,0},{1,1},{-2,1}
    std::set<point> const expect{{-1,0},{0,0},{1,1},{-2,1}};
    std::set<point> found;
    // though not necessarily in that order
    for( auto p : w.branch_points() )
        found.insert(*p);
    return found == expect;
}


bool test_path_split()
{
    world w{make_world(1)};
    std::set<point> at;
    at.insert(point(-2,1)); // d
    w.proper_paths({});
    return true;
}


bool test_make_poly1()
{
    return true;
}


bool test_make_poly2()
{
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
 */

world make_world(int k)
{
    world w;
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

