//
// Created by jens on 02/09/23.
//

#include "point.h"
#include <iostream>

double point::tol = 0.01;
double point::tol2 = point::tol * point::tol;
int point::pid_ctr = 0;

std::ostream &
operator<<(std::ostream &os, point const &p)
{
    os << '(' << p.x() << ',' << p.y() << ')';
    return os;
}
