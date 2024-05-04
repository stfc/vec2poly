//
// Created by jens on 31/03/24.
//

#include <iostream>
#include "toplevel.h"
#include "iobase.h"

void toplevel::visit(alien a)
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
    return std::make_unique<ioxfig>(w_, g_);
}
