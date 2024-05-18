//
// Created by jens on 10/03/24.
//

#include <iostream>
#include <numeric>
#include "iobase.h"
#include "graph-path.h"



point transform::operator()(point p) const noexcept
{
    double px = p.x(), py = p.y();
    px += dx_; py += dy_;
    px *= sx_; py *= sy_;
    return point(px+ox_, py+oy_);
}


iobase::iobase(toplevel const &t, world const &w, graph const &g) : t_(t), w_(w), g_(g), tf_(), tol_(w.alloc_.tol())
{
}


void iobase::writeworld(std::ostream &os)
{
    preamble(os);

    // TODO: get polygons from toplevel
    // TODO: filter for paths unused in polygons
    for( path const &p : w_.paths() )
        writepath(os, p);
    postamble(os);
}


ioxfig::ioxfig(const toplevel &t, const world &w, const graph &g) : iobase(t, w, g), pppl(0), colour(0)
{
    bbox bb;
    t.visit(bb);
    switch(bb.npoints) {
    case 0:
        // Nothing to print, use default transform
        return;
    case 1:
        throw BadIO("cannot happen single point");
    }
    // shift to the centre before scaling - coordinates are origin bottom right initially
    int dx = std::midpoint(bb.botlx, bb.toprx);
    int dy = std::midpoint(bb.botly, bb.topry);
    // bb cannot be a point or uninitialised
    double scale = 1000.0 / std::max(bb.toprx-bb.botlx,bb.topry-bb.botly);
    // scaling Y by -1 to put origin into top left.  Hence the factor 1.5; y maps to (H-y)+H/2 where H is height
    tf_ = transform(dx, dy, scale, -scale, (bb.toprx-bb.botlx)*scale, 1.5*(bb.topry-bb.botly)*scale);
}


void ioxfig::writepoint(std::ostream &os, point xy)
{
    if(pppl == pppl_max) {
        os << "\n\t";
        pppl = 0;
    }
    auto writecoord = [&os](decltype(xy.x()) val)
    {
        os << ' ' << (val < 0 ? '-' : ' ') << (val < 0 ? -val : val);
    };
    auto q = tf_(xy);
    writecoord(q.x());
    writecoord(q.y());
}


void ioxfig::writepath(std::ostream &os, const path &p)
{
    auto s = p.size();
    os << "2 1 0 1 " << next_colour() << " 7 50 -1 -1 0.000 0 0 -1 0 0 " << s+1 << '\n';
    for( auto const &ls : p ) {
        writepoint(os, *ls.first());
        if( 0 == --s )
            writepoint(os, *ls.second());
    }
    os << '\n';
}


void ioxfig::writepolygon(std::ostream &os, const polygon &p)
{
    auto s = p.size(w_);
    // Subtype (second entry) is 3 for polygon
    // TODO: should polygons have different colours from paths
    os << "2 3 0 1 " << next_colour() << " 7 50 -1 -1 0.000 0 0 -1 0 0 " << s << '\n';
    auto cb = [this, &os](const pathpoint q)
    {
        this->writepoint(os, static_cast<point>(*q));
    };
    pppl = 0;
    trail_walk walker(w_, cb);
    auto [a,b] = walker.walk(p.begin(), p.end());
    os << '\n';
    if(a != b) {
        std::cerr << "Polygon not closed (cannot happen)\n";
    }
}


void ioxfig::preamble(std::ostream &os) {
    os << "#FIG 3.2  Produced by vec2poly\n"
          "Landscape\n"
          "Center\n"
          "Metric\n"
          "A4\n"
          "100.0\n"
          "Single\n"
          "-2\n"
          "1200 2\n";
}


unsigned int ioxfig::next_colour() noexcept
{
    switch(colour) {
    case 6:
        colour = 7;    // skip white
        break;
    case 31:
        colour = 0;    // skip black
        break;
    // more stuff here
    }
    // default
    return ++colour;
}
