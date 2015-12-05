#ifndef GEOMETRY_H
#define GEOMETRY_H

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

    class Ray {
    public:
        Ray(Math::Vector & origin, Math::Vector & direction) : origin(origin), direction(direction.getNormalized()) {}
        bool intersection(Math::Vector & result, LineSegment & segment) const;

        // A factor suitable to be passed to ray \arg a as argument to calculate 
        // the intersection point.
        // \NOTE A value in the range [0, 1] indicates a point between
        // a.origin and a.origin + a.direction.
        // \NOTE The result is std::numeric_limits<double>::quiet_NaN() if the
        // rays do not intersect. 
        // \SEE  intersection_point
        float intersection_factor(const Ray& other) const {
            const float Precision = std::sqrt(std::numeric_limits<float>::epsilon());
            float d = direction.x * other.direction.y - direction.y * other.direction.x;
            if (std::abs(d) < Precision) return std::numeric_limits<float>::quiet_NaN();
            else {
                float n = (other.origin.x - origin.x) * other.direction.y
                    - (other.origin.y - origin.y) * other.direction.x;
                return n / d;
            }
        }
        
        // The intersection of two rays.
        // \NOTE The result is a Vector having the coordinates
        //       std::numeric_limits<double>::quiet_NaN() if the rays do not
        //       intersect. 
        Math::Vector intersection_point(const Ray& other, bool bothways = false) const {
            float factor = intersection_factor(other);
            if(!bothways && factor < 0){
                return Math::Vector(NAN, NAN);
            }
            return origin + direction * factor;
        }

    private:
        Math::Vector origin;
        Math::Vector direction;

    };
}

#endif