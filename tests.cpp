//
// Created by jens on 09/09/23.
//

// TODO migrate to gtest?
// #include <gtest/gtest.h>
#include <iostream>
#include "lineseg.h"
#include "world.h"

bool expect(int i, lineseg const &, lineseg const &, std::optional<point>);

static bool test_lineseg();
static bool test_poly1();

int tests()
{
    bool ret = test_lineseg();
    ret |= test_poly1();
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
    std::cout << "No paths\n";
    w.split_paths();
    w.add_path(path{{1,2},{2,3},{3,1}});
    std::cout << "One path\n";
    w.split_paths();
    std::cout << "Two paths\n";
    w.add_path(path{{-1,-2},{-2,-3},{-3,-1}});
    w.split_paths();
    return true;
}
