#include <iostream>
#include "except.h"

std::ostream &operator<<(std::ostream &os, Vec2PolyException const &e)
{
    os << e.msg_ << std::endl;
    return os;
}
