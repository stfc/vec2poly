//
// Created by jens on 10/03/24.
//

#ifndef VEC2POLY_IOBASE_H
#define VEC2POLY_IOBASE_H

#include "world.h"
#include "polygon.h"
#include "graph-path.h"

/** iobase - format world or polygon(s) to stream
 *
 * Currently only doing output.  The default is the IO used for debugging.
 */

class iobase {
    world const &w_;
    graph const &g_;
protected:
    double tol_;
    virtual void preamble(std::ostream &) {};
    virtual void writepath(std::ostream &os, path const &p) { os << p; };
    virtual void writepolygon(std::ostream &os, polygon const &p) { os << p; }
    virtual void postamble(std::ostream &) {};
public:
    iobase(world const &, graph const &);
    virtual void writeworld(std::ostream &);
    virtual ~iobase();
};


class ioxfig : public iobase {
public:
    ioxfig(world const &w, graph const &g) : iobase(w, g) {}

    void preamble(std::ostream &);
    void writepath(std::ostream &os, const path &p);
};


#endif //VEC2POLY_IOBASE_H
