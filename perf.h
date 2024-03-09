//
// Created by jens on 06/02/24.
// Create large/many test polygons for performance testing
// See docs/perf.fig for size 4 (ie 2^4)
//

#ifndef VEC2POLY_PERF_H
#define VEC2POLY_PERF_H

#include "world.h"

/** Make a big world for performance testing.
 *
 * The size of the world will be 2^k * 2^k
 *
 * @param size size of world
 * @return world with paths added to it
 */
world make_big_world(unsigned int size);

#endif //VEC2POLY_PERF_H
