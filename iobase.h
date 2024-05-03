//
// Created by jens on 10/03/24.
//

#ifndef VEC2POLY_IOBASE_H
#define VEC2POLY_IOBASE_H

#include "world.h"
#include "polygon.h"
#include "graph-path.h"
#include "toplevel.h"

/** iobase - format world or polygon(s) to stream
 *
 * Currently only doing output.  The default is the IO used for debugging.
 */

class iobase {
protected:
    world const &w_;
    // FIXME consider making graph const (again)
    graph &g_;
    double tol_;
    virtual void writepoint(std::ostream &os, point x) { os << x; };
    virtual void writepoint(std::ostream &os, const pathpoint q) { os << static_cast<point>(*q); }
    virtual void preamble(std::ostream &) {};
    virtual void writepath(std::ostream &os, path const &p) { os << p; };
    virtual void writepolygon(std::ostream &os, polygon const &p) { os << p; }
    virtual void postamble(std::ostream &) {};
public:
    // FIXME consider making graph const (again)
    iobase(world const &, graph &);
    virtual void writeworld(std::ostream &, bool);
    virtual ~iobase() = default;
};


class ioxfig : public iobase {
private:
    /* points printed per line, used by writepoint */
    size_t pppl;
    const size_t pppl_max = 6;
    /* For giving different colours to different polygons */
    unsigned int colour;
    /* Assign the next colour */
    unsigned int next_colour() noexcept;
protected:
    void writepoint(std::ostream &, point) override;
public:
    ioxfig(world const &w, graph &g) : iobase(w, g) {}

    void preamble(std::ostream &) override;
    void writepath(std::ostream &os, const path &p) override;
    void writepolygon(std::ostream &, const polygon &) override;
};


#endif //VEC2POLY_IOBASE_H
