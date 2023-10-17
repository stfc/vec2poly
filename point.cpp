//
// Created by jens on 02/09/23.
//

#include "point.h"
#include <iostream>

double point::tol = 0.01;
double point::tol2 = point::tol * point::tol;


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
