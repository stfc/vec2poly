//
// Created by jens on 31/03/24.
//

#include <iostream>
#include "toplevel.h"
#include "iobase.h"


bbox::bbox() noexcept
{
    std::numeric_limits<int> lim;
    botlx = botly = lim.max();
    toprx = topry = lim.min();
    npoints = 0;
}


void bbox::point(struct point p)
{
    if(p.x() < botlx)
        botlx = p.x();
    if(p.y() < botly)
        botly = p.y();
    if(p.x() > toprx)
        toprx = p.x();
    if(p.y() > topry)
        topry = p.y();
    ++npoints;
}


void toplevel::visit(alien &a) const
{
    auto cb = [&a](pathpoint p)
    {
        a.point(*p);
    };
    a.begin_world();
    for(polygon const &m : poly_) {
        a.begin_poly();
        trail_walk w(w_, cb);
        auto [p, q] = w.walk(m.begin(), m.end());
        if(p != q) {
            std::cerr << "Non-closed polygon; can't happen\n";
        }
        a.end_poly();
    }
    const path_lookup lookup(w_);
    auto pp = [&a, &lookup, &cb](edge_t e)
    {
        a.begin_path();
        lookup(e).points(cb);
        a.end_path();
    };
    g_.paths(pp, true);
    a.end_world();
}


std::unique_ptr<iobase> toplevel::make_io(toplevel::io_type_t) {
    return std::make_unique<ioxfig>(*this, w_, g_);
}


debug::debug() { std::cerr << "BB START\n"; }
debug::~debug() { std::cerr << "BB END\n"; }

void debug::begin_world()
{
    std::cerr << "BB BEGWLD\n";
}

void debug::end_world()
{
    std::cerr << "BB ENDWLD\n";
}

void debug::begin_poly()
{
    std::cerr << "BB BEGPOL\n";
}

void debug::end_poly()
{
    std::cerr << "BB ENDPOL\n";
}

void debug::begin_path()
{
    std::cerr << "BB BEGPTH\n";
}

void debug::end_path()
{
    std::cerr << "BB ENDPTH\n";
}

void debug::point(class point p)
{
    std::cerr << "BB " << p << '\n';
}
