//
// Created by jens on 10/03/24.
//

#ifndef VEC2POLY_IOBASE_H
#define VEC2POLY_IOBASE_H

#include "world.h"
#include "polygon.h"
#include "graph-path.h"
#include "toplevel.h"


/** transform - scale/move/rotate point */
struct transform {
    /** displacement */
    int dx_, dy_;
    // rotation would go here
    /** scale */
    double sx_, sy_;
    // or here
    /** offset */
    int ox_, oy_;

    transform() noexcept : dx_(0), dy_(0), sx_(1.0), sy_(1.0), ox_(0), oy_(0) {}
    transform(int dx, int dy, double sx, double sy, int ox, int oy) noexcept : dx_(dx), dy_(dy), sx_(sx), sy_(sy), ox_(ox), oy_(oy) {}

    [[nodiscard]] point operator()(point) const noexcept;
};


/** iobase - format world or polygon(s) to stream
 *
 * Currently only doing output.  The default is the IO used for debugging.
 */

class iobase {
protected:
    toplevel const &t_;
    world const &w_;
    graph const &g_;
    transform tf_;
    double tol_;
    virtual void writepoint(std::ostream &os, point x) { os << x; };
    virtual void writepoint(std::ostream &os, const pathpoint q) { os << static_cast<point>(*q); }
    virtual void preamble(std::ostream &) {};
    virtual void writepath(std::ostream &os, path const &p) { os << p; };
    virtual void writepolygon(std::ostream &os, polygon const &p) { os << p; }
    virtual void postamble(std::ostream &) {};
public:
    iobase(toplevel const &, world const &, graph const &);
    virtual void writeworld(std::ostream &);
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
    ioxfig(toplevel const &t, world const &w, graph const &g);

    void preamble(std::ostream &) override;
    void writepath(std::ostream &os, const path &p) override;
    void writepolygon(std::ostream &, const polygon &) override;
};


#endif //VEC2POLY_IOBASE_H
