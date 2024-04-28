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

// Defined in iobase.h
class iobase;

class toplevel {
    world &w_;
    graph g_;
public:
    toplevel(world &w) : w_(w), g_(w) {}
    void visit(alien);

    enum class io_type_t { IO_W_XFIG };
    iobase *make_io(io_type_t);
};


#endif //VEC2POLY_TOPLEVEL_H
