//
// Created by jens on 09/09/23.
//

// TODO migrate to gtest?
// #include <gtest/gtest.h>
#include <iostream>
#include <array>
#include <functional>
#include "lineseg.h"
#include "world.h"
#include "pntalloc.h"

bool expect(pntalloc &, int i, lineseg const &, lineseg const &, std::optional<point>);

// Tests are sorted by increasing integratedness

[[nodiscard]] bool test_pntalloc();
[[nodiscard]] bool test_lineseg();
[[nodiscard]] bool test_poly1();
[[nodiscard]] bool test_poly2();
[[nodiscard]] static bool test_path_iter();
[[nodiscard]] static bool test_branch_points();
[[nodiscard]] static bool test_path_split();
[[nodiscard]] static bool test_make_poly1();
[[nodiscard]] static bool test_make_poly2();

static world make_world(int);



int tests()
{
    bool ret = true;
    unsigned num{0};
    std::array<std::function<bool()>,7> all{test_pntalloc, test_lineseg, test_poly1, test_poly2,
                                            test_path_iter, test_branch_points, test_path_split};
    for( auto testfunc : all ) {
        ++num;
        try {
            bool result = testfunc();
            if(!result)
                std::cerr << "Test " << num << " failed with no exception raised\n";
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
    auto u1 = z.make_point(1,2);
    auto u2 = z.make_point(3,4);
    auto u3 = z.make_point(1,2);
    for( auto const &p : z.points() )
        std::cout << p << ' ' << p.use_count() << '\n';
    return true;
}


bool
test_lineseg()
{
    pntalloc u;
    // a b define a line y = x/3
    // c-f are collinear, on the line y = -2x+14
    point   a = u.make_point(0,0),
            b = u.make_point(9,3),
            c = u.make_point(4,6),
            d = u.make_point(5,4),
            e = u.make_point(6,2),
            f = u.make_point(7,0);
    lineseg ab(a,b);
    bool ret = true;
    ret &= expect(u, 1, ab, lineseg(c,f), e);
    ret &= expect(u, 2, ab, lineseg(c,d), std::nullopt);
    ret &= expect(u, 3, ab, lineseg(c,e), e);
    ret &= expect(u, 4, lineseg(a,e), lineseg(c,e), e);
    lineseg gh(u.make_point(1,3),u.make_point(5,-1));
    ret &= expect(u, 5, ab, gh, u.make_point(3,1));
    ret &= expect(u, 6, gh, ab, u.make_point(3,1));
    return ret;
}


bool
expect(pntalloc &u, int i, lineseg const &v, lineseg const &w, std::optional<point> expt)
{
    bool ret = true;
    std::optional<point> z = intersects(u, v, w);
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


bool test_poly1()
{
    world w;
    pntalloc &u = w.alloc_;
    // World is empty, so should not iterate here
    for( auto const &y : w ) {
        std::cerr << "Error: World is not empty!\n";
        return false;
    }
    point   a = u.make_point(1,2),
            b = u.make_point(2,3),
            c = u.make_point(3,1),
            d = u.make_point(4,5),
            e = u.make_point(-1,-2),
            f = u.make_point(-2,-3),
            o = u.make_point(0,0);
    // Two line segments: a->b and b->c
    w.add_path(path({a,b,c}));
    auto q = w.begin(); // point to first segment
    ++q;		// point to second and last segment
    // Add c->d
    q.insert_after(lineseg({c,d}));
    if( w.map_[0] != path({a,b,c,d}) ) {
        std::cerr << "Insertion failed: " << w.map_[0] << '\n';
        return false;
    }
    // Add e->f as a separate path
    w.add_path(path({e,f}));
    // Adding a path may have invalidated the iterator, so we refresh it
    q = w.begin(); ++q; ++q; ++q;
    // q should now reference the single line segment on the second path
    q.insert_after(lineseg({f,o}));
    if(w.map_.size() != 2) {
        std::cerr << "Expected two paths; found " << w.map_.size() << '\n';
        return false;
    }
    if(w.map_[0] != path({a,b,c,d}) || w.map_[1] != path({e,f,o})) {
        std::cerr << "Error with paths " << w.map_[0] << " or " << w.map_[1] << '\n';
        return false;
    }
    // Finally, an insert at not-the-end - q still points to the *first* entry on path 2
    // This is a dummy line segment, not usually useful (or permitted)
    q.insert_after(lineseg({f,f}));
    return true;
}


bool test_poly2()
{
    world w;
    pntalloc &u = w.alloc_;
    w.add_path(path{u, {{-2, 2},{-1,2},{-1,-2},{2,-2},{2,1},{3,2}}});
    w.add_path(path{u, {{-3, 1},{3,1}}});
    w.split_segments();
    return w.map_[0] == path(u, {{-2, 2},
                                 {-1, 2},
                                 {-1, 1},
                                 {-1, -2},
                                 {2,  -2},
                                 {2,  1},
                                 {3,  2}})
           && w.map_[1] == path{u, {{-3, 1},{-1,1},{2,1},{3,1}}};
}


bool test_path_iter()
{
    world w;
    point a = w.make_point(1,1),
            b = w.make_point(2,2),
            c = w.make_point(3,4),
            d = w.make_point(5,6);
    w.add_path(path({a,b}));
    w.add_path(path({c,d}));
    std::vector<point> items;
    for( auto const &seg : w.segments() ) {
        items.push_back(seg.first());
        items.push_back(seg.last());
    }
    std::vector<point> expected{a,b,c,d};
    return items == expected;
}


bool test_branch_points()
{
    world w{make_world(4)};
    // Expecting {-1,0},{0,0},{1,1},{-2,1}
    // though not necessarily in that order
    for( auto &p : w.branch_points() ) {
        std::cout << p << '(' << p.use_count() << ')' << std::endl;
    }
    return true;
}


bool test_path_split()
{
    world w{make_world(1)};
    std::set<point> at;
    at.insert(w.make_point(-2,1)); // d
    w.proper_paths({});
    return true;
}


bool test_make_poly1();
bool test_make_poly2();


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
    point c = w.make_point(-1,0), b = w.make_point(-3,0), a = w.make_point(-3, 2);
    // Triangle two
    point h = w.make_point(0,0), i = w.make_point(1,0), g = w.make_point(1,1);
    // Connecting line
    point d = w.make_point(-2,1), e = w.make_point(-1,2), f = w.make_point(0,2);
    w.add_path(path{{a,b,c,d,a}});
    if(k>=2)
        w.add_path(path{{d,e,f,g}});
    if(k>=3)
        w.add_path(path{{i,g,h,i}});
    if(k == 4) {
        w.add_path(path{{c,h}});
    }
    return w;
}

