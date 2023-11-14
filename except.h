//
// Created by jens on 14/11/23.
//

#ifndef VEC2POLY_EXCEPT_H
#define VEC2POLY_EXCEPT_H

#include <exception>
#include <string>
#include <iosfwd>


class Vec2PolyException : public std::exception {
    std::string msg_;
public:
    Vec2PolyException() : msg_("Unknown vec2poly exception") {};

    Vec2PolyException(std::string &&msg) : msg_(std::forward<std::string>(msg)) {}

    Vec2PolyException(std::string_view msg) : msg_(msg) {}

    Vec2PolyException(char const *msg) : msg_(msg) {}

    char const *what() const noexcept override { return msg_.c_str(); }

    friend std::ostream &operator<<(std::ostream &, Vec2PolyException const &);
};


#endif // VEC2POLY_EXCEPT_H
