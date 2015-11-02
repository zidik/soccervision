#include <boost/test/unit_test.hpp>

#include "Maths.h"
using namespace Math;
BOOST_AUTO_TEST_CASE(VectorCreation)
{
	
	Vector a(1.21f, 2.45f);
	Vector b(a);
	BOOST_TEST(a.x == 1.21f);
	BOOST_TEST(a.y == 2.45f);
	BOOST_TEST(b.x == 1.21f);
	BOOST_TEST(b.y == 2.45f);

	Vector c;
	BOOST_CHECK(c.x == 0.0f);
	BOOST_CHECK(c.y == 0.0f);
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

BOOST_AUTO_TEST_CASE(Vectorsubtraction)
{
	Vector a(1.21f, 2.45f);
	Vector b(0.21f, 3.45f);

	Vector c = a - b; 
	BOOST_TEST(c.x == 1.0f);
	BOOST_TEST(c.y == -1.0f);
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