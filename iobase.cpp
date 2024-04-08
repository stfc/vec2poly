//
// Created by jens on 10/03/24.
//

#include "iobase.h"
#include "graph-path.h"

iobase::iobase(world const &w, const graph &g) : w_(w), g_(g), tol_(w.alloc_.tol())
{
}


iobase::~iobase()
{
}


void iobase::writeworld(std::ostream &os)
{
    preamble(os);

}


void ioxfig::writepath(std::ostream &os, const path &p)
{
    os << "2 1 0 1 0 7 50 -1 -1 0.000 0 0 -1 0 0 ";
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
