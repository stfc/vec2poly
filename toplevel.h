//
// Created by jens on 31/03/24.
//

/** toplevel holds the world and its graph.
 * Running out of good names but something needs to hold the world object and its associated graph
 */
#ifndef VEC2POLY_TOPLEVEL_H
#define VEC2POLY_TOPLEVEL_H


#include "world.h"
#include "graph-path.h"
#include "polygon.h"


/** aliens visit the world - a world visitor */
struct alien {
    virtual void begin_world() {};
    virtual void end_world() {};
    virtual void begin_poly() {};
    virtual void point(point) {};
    virtual void end_poly() {};
    virtual void begin_path() {};
    virtual void end_path() {};
};


/** special utility alien measuring all points */
struct bbox : public alien {
    int botlx, botly, toprx, topry;
    unsigned int npoints;
    bbox() noexcept;
    void point(class point) override;
};

/** special utility alien for debugging */
struct debug : public alien {
    debug();
    ~debug();

    void begin_world() override;
    void end_world() override;
    void begin_poly() override;
    void end_poly() override;
    void begin_path() override;
    void end_path() override;
    void point(class point) override;
};


// Defined in iobase.h
class iobase;

class toplevel {
    world &w_;
    graph g_;
    std::list<polygon> poly_;
public:
    toplevel(world &w) : w_(w), g_(w), poly_() {}
    void visit(alien &a) const;

    enum class io_type_t { IO_W_XFIG };
    std::unique_ptr<iobase> make_io(io_type_t);
};


#endif //VEC2POLY_TOPLEVEL_H
