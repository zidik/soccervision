#include <boost/test/unit_test.hpp>

#include "Geometry.h"
using namespace Geometry;
using namespace Math;
BOOST_AUTO_TEST_CASE(RayIntersection) {
    BOOST_CHECK(Ray(Vector(0, 0), Vector(0, 1)).intersection_point(Ray(Vector(1, 1), Vector(-1, 0))) == Vector(0, 1));
    BOOST_CHECK(Ray(Vector(-1, 3), Vector(0.1f, -0.2f)).intersection_point(Ray(Vector(0, 0), Vector(0, 1))) == Vector(0, 1));

    Math::Vector result = Ray(Vector(2, 2), Vector(3, 3)).intersection_point(Ray(Vector(0, 1), Vector(1, 1)));
    BOOST_TEST(isnan(result.x));
    BOOST_TEST(isnan(result.y));



    Ray ball(Vector(1, 1), Vector(1, 0));
    Ray robot(Vector(0, 0), Vector(0, 1));

    // Ray ball(ball.location, ball.velocity)
    // Ray robot(robot.location, ball.velocity.getRotated(PI/2))
    Math::Vector result2 = ball.intersection_point(robot);
    BOOST_CHECK(isnan(result2.x));
    BOOST_CHECK(isnan(result2.y));

    Math::Vector result3 = robot.intersection_point(ball);
    BOOST_CHECK(result3 == Vector(0, 1));



}

