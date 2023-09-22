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

bool expect(int i, lineseg const &, lineseg const &, std::optional<point>);

[[nodiscard]] static bool test_lineseg();
[[nodiscard]] bool test_poly1();
[[nodiscard]] bool test_poly2();
[[nodiscard]] static bool test_path_iter();
[[nodiscard]] static bool test_branch_points();
[[nodiscard]] static bool test_split_path();
[[nodiscard]] static bool test_make_poly1();
[[nodiscard]] static bool test_make_poly2();

static world make_world(int);



int tests()
{
    bool ret = true;
    unsigned num{0};
    std::array<std::function<bool()>,4> all{test_lineseg, test_poly1, test_poly2, test_path_iter};
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

bool
test_lineseg()
{
    // a b define a line y = x/3
    // c-f are collinear, on the line y = -2x+14
    point a(0,0), b(9,3), c(4,6), d(5,4), e(6,2), f(7,0);
    lineseg ab(a,b);
    bool ret = true;
    ret &= expect(1, ab, lineseg(c,f), e);
    ret &= expect(2, ab, lineseg(c,d), std::nullopt);
    ret &= expect(3, ab, lineseg(c,e), e);
    ret &= expect(4, lineseg(a,e), lineseg(c,e), e);
    lineseg gh(point(1,3),point(5,-1));
    ret &= expect( 5, ab, gh, point(3,1));
    ret &= expect(6, gh, ab, point(3,1));
    return ret;
}


bool
expect(int i, lineseg const &v, lineseg const &w, std::optional<point> expt)
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


bool test_poly1()
{
    world w;
    // World is empty, so should not iterate here
    for( auto const &y : w ) {
        std::cerr << "Error: World is not empty!\n";
        return false;
    }
    point a(1,2), b(2,3), c(3,1), d(4,5), e(-1,-2), f(-2,-3), o(0,0);
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
    w.add_path(path{{-2,2},{-1,2},{-1,-2},{2,-2},{2,1},{3,2}});
    w.add_path(path{{-3,1},{3,1}});
    w.split_paths();
    return w.map_[0] == path({{-2,2},{-1,2},{-1,1},{-1,-2},{2,-2},{2,1},{3,2}})
           && w.map_[1] == path{{-3,1},{-1,1},{2,1},{3,1}};
}


bool test_path_iter()
{
    world w;
    point a(1,1), b(2,2), c(3,4), d(5,6);
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


bool test_branch_points();
bool test_split_path();
bool test_make_poly1();
bool test_make_poly2();


world make_world(int k)
{
    world w;
    // Triangle one
    point c(-1,0), b(-3,0), a(-3, 2);
    // Triangle two
    point h(0,0), i(1,0), g(1,1);
    // Connecting line
    point d(-2,1), e(-1,2), f(0,2);
    w.add_path(path{a,b,c,d,a});
    w.add_path(path{d,e,f,g});
    w.add_path(path{i,h,g,i});
    if(k == 2) {
	w.add_path(path{c,h});
    }
    return w;
}

