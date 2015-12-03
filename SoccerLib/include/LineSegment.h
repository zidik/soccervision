#ifndef GEOMETRY_LINESEGMENT_H
#define GEOMETRY_LINESEGMENT_H

#include "Maths.h"

namespace Geometry {
    class LineSegment {
    public:
        LineSegment() = default;
        LineSegment(Math::Vector & p1, Math::Vector & p2) : _p1(p1), _p2(p2) {}
        Math::Vector& p1() { return _p1; }
        Math::Vector& p2() { return _p2; }
    private:
        Math::Vector _p1;
        Math::Vector _p2;
    };
}

#endif
