//
// Created by jens on 10/03/24.
//

#include <iostream>
#include "iobase.h"
#include "graph-path.h"

iobase::iobase(world const &w, graph &g) : w_(w), g_(g), tol_(w.alloc_.tol())
{
}


void iobase::writeworld(std::ostream &os, bool calc)
{
    preamble(os);
    if(calc) {
	try {
	    for(;;) {
		auto p = g_.find_polygon();
		writepolygon(os, p);
	    }
	}
	catch(graph::AllDone) {
	}
    }
    // TODO: filter for paths unused in polygons
    for( path const &p : w_.paths() )
	writepath(os, p);

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
    writecoord(xy.x());
    writecoord(xy.y());
}


void ioxfig::writepath(std::ostream &os, const path &p)
{
    auto s = p.size();
    // pen colour (fifth entry) set to 1 to make paths not-black
    os << "2 1 0 1 1 7 50 -1 -1 0.000 0 0 -1 0 0 " << s+1 << '\n';
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
    // Colour (fifth entry) is 0 for black
    os << "2 3 0 1 0 7 50 -1 -1 0.000 0 0 -1 0 0 " << s << '\n';
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
    os << "#FIG3.2  Produced by vec2poly\n"
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
