#include "LineSegment.h"
#include "Ray.h"

namespace Geometry
{
    bool Ray::intersection(Math::Vector & result, LineSegment & segment) const {
        Math::Vector v1 = origin - segment.p1();
        Math::Vector v2 = segment.p2() - segment.p1();
        Math::Vector v3(-direction.y, direction.x);

        float t1 = v2.cross(v1) / v2.dot(v3);
        float t2 = v1.dot(v3) / v2.dot(v3);

        if (t1 >= 0.0 && (t2 >= 0.0 && t2 <= 1.0)) {
            result = origin + direction * t1;
            return true;
        }
        result = Math::Vector(NAN, NAN);
        return false;
    }
}
