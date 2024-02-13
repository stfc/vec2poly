//
// Created by jens on 02/09/23.
//

#include "point.h"
#include <iostream>


std::ostream &
operator<<(std::ostream &os, point const &p)
{
    os << '(' << p.x() << ',' << p.y() << ')';
    return os;
}


std::ostream &
operator<<(std::ostream &os, pathpoint p)
{
    os << '(' << p->x() << ',' << p->y() << ")[" << p->use_count_ << "]";
    return os;
}


std::ostream &
operator<<(std::ostream &os, std::vector<point> const &v)
{
    os << '[';
    auto flip = v.cbegin();
    auto const flap = v.cend();
    if( flip != flap ) {
       os << *flip;
       while( ++flip != flap )
           os << ',' << *flip;
    }
    os << ']';
    return os;
}
