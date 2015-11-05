
#include <CameraTranslator.h>
#include <ParticleFilterLocalizer.h>
#include <boost/test/unit_test.hpp>


//____________________________________________________________________________//
struct F {
	F() {
		BOOST_TEST_MESSAGE("setup fixture");
		cameraTranslator.setConstants(
			A, B, C,
			horizon,
			Config::cameraWidth, Config::cameraHeight
			);
	}
	~F() { BOOST_TEST_MESSAGE("teardown fixture"); }

	void ensureDistortionMapLoaded()
	{
		if (!distortionMapLoaded) {
			loadDistortionMap();
		}
	}

	void loadDistortionMap()
	{
		BOOST_TEST_MESSAGE("Loading distortion map...");
		throw std::runtime_error("not completely implemented");
		//cameraTranslator.loadDistortionMapping(
			//Config::distortMappingFilenameFrontX,
			//Config::distortMappingFilenameFrontY
		//);
		BOOST_TEST_MESSAGE("Done!");

		BOOST_TEST_MESSAGE("generating undistortion mappings..");
		CameraTranslator::CameraMapSet mapSet = cameraTranslator.generateInverseMap(cameraTranslator.distortMapX, cameraTranslator.distortMapY);
		cameraTranslator.undistortMapX = mapSet.x;
		cameraTranslator.undistortMapY = mapSet.y;
		BOOST_TEST_MESSAGE("Done!");

		distortionMapLoaded = true;
	}

	float A = 120.11218157847301f;
	float B = -0.037205566171594123f;
	float C = 0.2124259596292023f;
	float horizon = 119.40878f;
	CameraTranslator cameraTranslator;
	bool distortionMapLoaded = false;
};
//____________________________________________________________________________//

BOOST_FIXTURE_TEST_SUITE(TranslationSuite, F)
//____________________________________________________________________________//

typedef Pixel CameraPosition;

void testTranslationLoopback(const CameraTranslator &cameraTranslator, const CameraPosition &initial, bool distortion=true)
{
	Math::Vector worldpos = cameraTranslator.getWorldPosition(initial, distortion);
	CameraPosition camerapos = cameraTranslator.getCameraPosition(worldpos, distortion);
	BOOST_TEST(initial.x - 2 < camerapos.x < initial.x + 2);
	BOOST_TEST(initial.y - 2 < camerapos.y < initial.y + 2);
}

void testMultipleTranslationLoopback(const CameraTranslator &cameraTranslator, bool distortion=true)
{
	testTranslationLoopback(cameraTranslator, CameraPosition(423, 543), distortion);
	testTranslationLoopback(cameraTranslator, CameraPosition(1073, 1000), distortion);
}

BOOST_AUTO_TEST_CASE(cameraTranslationLoopbackTest)
{
	testMultipleTranslationLoopback(cameraTranslator, false);
}

/*
BOOST_AUTO_TEST_CASE(cameraTranslationDistortedLoopbackTest)
{
	ensureDistortionMapLoaded();
	testMultipleTranslationLoopback(cameraTranslator, true);
}
*/

void testGetCameraPositionSanity(const CameraTranslator &cameraTranslator, const Math::Vector &centerPosition, bool distortion = true)
{
	Math::Vector leftPostition = centerPosition + Math::Vector(0.0f, 0.2f);
	Math::Vector rightPostition = centerPosition + Math::Vector(0.0f, -0.2f);
	Math::Vector forwardPostition = centerPosition + Math::Vector(0.2f, 0.0f);
	Math::Vector backPostition = centerPosition + Math::Vector(-0.2f, 0.0f);

	CameraPosition centerPixel = cameraTranslator.getCameraPosition(centerPosition, distortion);
	CameraPosition leftPixel = cameraTranslator.getCameraPosition(leftPostition, distortion);
	CameraPosition rightPixel = cameraTranslator.getCameraPosition(rightPostition, distortion);
	CameraPosition forwardPixel = cameraTranslator.getCameraPosition(forwardPostition, distortion);
	CameraPosition backPixel = cameraTranslator.getCameraPosition(backPostition, distortion);

	BOOST_TEST(leftPixel.x < centerPixel.x);
	BOOST_TEST(rightPixel.x > centerPixel.x);
	BOOST_TEST(forwardPixel.y < centerPixel.y);
	BOOST_TEST(backPixel.y > centerPixel.y);
}

void testMultipleGetCameraPositionSanity(const CameraTranslator &cameraTranslator, bool distortion = true)
{
	testGetCameraPositionSanity(cameraTranslator, Math::Vector(2.0f, 0.5f), distortion);
	testGetCameraPositionSanity(cameraTranslator, Math::Vector(1.0f, -0.5f), distortion);
	testGetCameraPositionSanity(cameraTranslator, Math::Vector(5.0f, -2.0f), distortion);
}

BOOST_AUTO_TEST_CASE(cameraTranslationSanityTest)
{
	testMultipleGetCameraPositionSanity(cameraTranslator, false);
}

/*
BOOST_AUTO_TEST_CASE(cameraTranslationDistortedSanityTest)
{
	ensureDistortionMapLoaded();
	testMultipleGetCameraPositionSanity(cameraTranslator, true);
}
*/

BOOST_AUTO_TEST_CASE(cameraTranslation_OLD_vs_NEW)
{
	ensureDistortionMapLoaded();

	//Old implementation uses nonstandard coordinate system, so x and y are switched and then direction of y flipped.
	CameraPosition camerapos(423, 543);
	Math::Vector newImplem = cameraTranslator.getWorldPosition(camerapos);
	CameraTranslator::DEPRECATEDWorldPosition oldImplem = cameraTranslator.DEPRECATEDgetWorldPosition(camerapos.x, camerapos.y);

	BOOST_TEST(newImplem.x == oldImplem.dy);
	BOOST_TEST(newImplem.y == -oldImplem.dx);
	BOOST_TEST(newImplem.getLength() == oldImplem.distance);

	Math::Vector worldPos(2.0f, 0.5f);
	CameraPosition newImplemCampos = cameraTranslator.getCameraPosition(worldPos);
	CameraPosition oldImplemCampos = cameraTranslator.DEPRECATEDgetCameraPosition(-worldPos.y, worldPos.x); //NB - here is the diff
	BOOST_TEST(newImplemCampos.x == oldImplemCampos.x);
	BOOST_TEST(newImplemCampos.y == oldImplemCampos.y);
}
//____________________________________________________________________________//
BOOST_AUTO_TEST_SUITE_END()
//____________________________________________________________________________//