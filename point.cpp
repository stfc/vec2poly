//
// Created by jens on 02/09/23.
//

#include "point.h"
#include <iostream>

double basepoint::tol = 0.01;
double basepoint::tol2 = basepoint::tol * basepoint::tol;

std::ostream &
operator<<(std::ostream &os, point const &p)
{
    os << '(' << p->x() << ',' << p->y() << ')';
    return os;
}
