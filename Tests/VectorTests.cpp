#include <boost/test/unit_test.hpp>

#include "Maths.h"
using namespace Math;
BOOST_AUTO_TEST_CASE(VectorConstructor)
{
	Vector a;
	BOOST_CHECK(a.x == 0.0f);
	BOOST_CHECK(a.y == 0.0f);

	Vector b(1.21f, 2.45f);
	BOOST_TEST(b.x == 1.21f);
	BOOST_TEST(b.y == 2.45f);

	Vector c(b);
	BOOST_TEST(c.x == 1.21f);
	BOOST_TEST(c.y == 2.45f);
}

BOOST_AUTO_TEST_CASE(VectorModification)
{
	Vector a(1.21f, 2.45f);
	Vector b(a);

	b.x = 100.0f;
	b.y = -200.0f;
	BOOST_TEST(b.x == 100.0f);
	BOOST_TEST(b.y == -200.0f);

	BOOST_TEST(a.x == 1.21f);
	BOOST_TEST(a.y == 2.45f);
}

BOOST_AUTO_TEST_CASE(VectorAdditionSubtraction, *boost::unit_test::tolerance(0.000001f))
{
	Vector a(1.21f, 2.45f);
	Vector b(0.21f, 3.45f);

	Vector c = a + b;
	BOOST_TEST(c.x == 1.42f);
	BOOST_TEST(c.y == 5.9f);

	Vector d = a - b; 
	BOOST_TEST(d.x == 1.0f);
	BOOST_TEST(d.y == -1.0f);
}

BOOST_AUTO_TEST_CASE(VectorScaling)
{
	Vector a(1.0f, -2.0f);
	Vector b = a * 3;
	BOOST_TEST(b.x == 3);
	BOOST_TEST(b.y == -6);

	Vector c = b/-2;
	BOOST_TEST(c.x == -1.5f);
	BOOST_TEST(c.y == 3);
}

BOOST_AUTO_TEST_CASE(VectorDistanceTo)
{
	Vector a(1.0f, 2.0f);
	Vector b(4.0f, 6.0f);
	Vector c(-2.0f, -2.0f);

	BOOST_TEST(a.distanceTo(b) == 5.0f);
	BOOST_TEST(b.distanceTo(a) == 5.0f);
	BOOST_TEST(a.distanceTo(c) == 5.0f);
	BOOST_TEST(b.distanceTo(c) == 10.0f);
}

BOOST_AUTO_TEST_CASE(VectorLength)
{
	Vector a(3.0f, 4.0f);
	Vector b(-3.0f, 4.0f);
	Vector c(-3.0f, -4.0f);

	BOOST_TEST(a.getLength() == 5.0f);
	BOOST_TEST(b.getLength()== 5.0f);
	BOOST_TEST(c.getLength() == 5.0f);
}

BOOST_AUTO_TEST_CASE(VectorRotation)
{
	Vector a(0.0f, 0.0f);
	Vector b = a.getRotated(PI/3);
	BOOST_TEST(b.x == 0);
	BOOST_TEST(b.y == 0);

	Vector c(1.0f, 0.0f);
	Vector d = c.getRotated(PI / 4);
	BOOST_TEST(d.x == 1/Math::sqrt(2.0f));
	BOOST_TEST(d.y == 1/Math::sqrt(2.0f));

	Vector e(0.0f, 2.0f);
	Vector f = e.getRotated(PI / 4);
	BOOST_TEST(f.x == -Math::sqrt(2.0f));
	BOOST_TEST(f.y ==  Math::sqrt(2.0f));

	Vector g(-2.0f, -2.0f);
	Vector h = g.getRotated(PI / 4);
	BOOST_TEST(h.x == 0);
	BOOST_TEST(h.y == -2*Math::sqrt(2.0f));
}