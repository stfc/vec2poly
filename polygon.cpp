//
// Created by jens on 29/11/23.
//

#include <iostream>
#include <vector>
#include "polygon.h"



std::ostream &operator<<(std::ostream &os, const polygon &p)
{
    auto const end{p.come_from_.size()};
    for(size_t i = 0; i < end; ++i) {
        if(p.come_from_[i] == p.invalid_)
            os << '*';
        else
            os << p.come_from_[i];
        os << " -> " << i << '\n';
    }
    return os;
}
