#ifndef GEOMETRY_RAY_H
#define GEOMETRY_RAY_H

#include "Maths.h"
class LineSegment;

namespace Geometry {
    class Ray {
    public:
        Ray(Math::Vector & origin, Math::Vector & direction): origin(origin), direction(direction.getNormalized()){}
        bool intersection(Math::Vector & result, LineSegment & segment) const;

    private:
        Math::Vector origin;
        Math::Vector direction;

    };
}

#endif
