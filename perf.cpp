//
// Created by jens on 06/02/24.
//

#include "perf.h"
#include <bit>
#include <limits>


using bigint_t = long;


// Separators are at 0 and even powers of two (see docs/perf.fig)
bool is_separator(bigint_t);

world make_big_world(unsigned int size)
{
    world w(1.0);
    constexpr auto props = std::numeric_limits<bigint_t>();
    // Ensure (2 << size) is OK by checking available binary digits
    if(size > props.digits-1)
        throw std::out_of_range("world size too large");
    bigint_t limit = static_cast<bigint_t>(1) << size;
    bigint_t lo = static_cast<bigint_t>(1), hi = static_cast<bigint_t>(2);
    for( bigint_t k = 0; k <= limit; ++k ) {
        if(is_separator(k) ) {
            w.add_path({{k,0},{k,limit}});
            w.add_path({{0,k},{limit,k}});
	    if(k) [[likely]] {
                lo <<= 1;
                hi <<= 1;
	    }
        } else {
            w.add_path({{0,k},{lo,k}});
            w.add_path({{k,0},{k,lo}});
            if(hi < limit) {
                w.add_path({{k,hi},{k,limit}});
                w.add_path({{hi,k},{limit,k}});
            }
        }
    }
    return w;
}


bool is_separator(bigint_t value)
{
    if(value == 0)
        return true;
    if(value <= 1)
        return false;
    return std::popcount(static_cast<unsigned long>(value)) == 1;
}
