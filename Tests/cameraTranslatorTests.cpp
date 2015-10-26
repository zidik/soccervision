#include <boost/test/unit_test.hpp>
#include <CameraTranslator.h>
#include <ParticleFilterLocalizer.h>
#include <Hacks.h>

BOOST_AUTO_TEST_CASE(relativePositionCalculationTest)
{
	ParticleFilterLocalizer::Particle particle(0.11459535020566808, 0.0016026943198371264, 0.5781195446530375, 0.0);
	Math::Vector absoluteLocation(4.5, 1.5);
	Math::Vector relativePosition;
	ParticleFilterLocalizer::calculateRelativePosition(relativePosition, particle, absoluteLocation);
	Math::Vector expectation(4.491539f, -1.1415092238416744f);
	BOOST_TEST(relativePosition.x == expectation.x);
	BOOST_TEST(relativePosition.y == expectation.y);

}
/*
BOOST_AUTO_TEST_CASE(cameraTranslationTest1)
{
	typedef CameraTranslator::WorldPosition WorldPosition;
	typedef CameraTranslator::CameraPosition CameraPosition;

	CameraTranslator* cameraTranslator = new CameraTranslator();
	float A = 120.11218157847301f;
	float B = -0.037205566171594123f;
	float C = 0.2124259596292023f;
	float horizon = 119.40878f;
	cameraTranslator->setConstants(
		A, B, C,
		0.0f, 0.0f, 0.0f,
		horizon, 0.0f,
		Config::cameraWidth, Config::cameraHeight
	);
	
}
*/

BOOST_AUTO_TEST_CASE(cameraTranslationTest)
{
	typedef CameraTranslator::WorldPosition WorldPosition;
	typedef CameraTranslator::CameraPosition CameraPosition;

	CameraTranslator* cameraTranslator = new CameraTranslator();
	float A = 120.11218157847301f;
	float B = -0.037205566171594123f;
	float C = 0.2124259596292023f;
	float horizon = 119.40878f;
	cameraTranslator->setConstants(
		A, B, C,
		0.0f, 0.0f, 0.0f,
		horizon, 0.0f,
		Config::cameraWidth, Config::cameraHeight
	);
	

	WorldPosition worldpos;
	CameraPosition camerapos;

	/// WITHOUT DISTORITION
	int x = 423;
	int y = 543;
	worldpos = cameraTranslator->getWorldPosition(x, y, false);
	camerapos = cameraTranslator->getCameraPosition(worldpos.dx, worldpos.dy, false);
	BOOST_TEST(x - 2 < camerapos.x < x + 2);
	BOOST_TEST(y - 2 < camerapos.y < y + 2);

	x = 1073;
	y = 1000;
	worldpos = cameraTranslator->getWorldPosition(x, y, false);
	camerapos = cameraTranslator->getCameraPosition(worldpos.dx, worldpos.dy, false);
	BOOST_TEST(x - 2 < camerapos.x < x + 2);
	BOOST_TEST(y - 2 < camerapos.y < y + 2);

	Math::Vector position(4.491539281804368, -1.1415092238416744);
	Math::Vector expectation(145.93095901658887, 782.5217145619256);
	HACKFLIP(position);
	camerapos = cameraTranslator->getCameraPosition(position,false);
	HACKFLIP(camerapos);
	BOOST_TEST(camerapos.x == expectation.x);
	BOOST_TEST(camerapos.y == expectation.y);


	/*
	std::cout << "  > loading camera distortion mappings.. ";
	cameraTranslator->loadDistortionMapping(
		Config::distortMappingFilenameFrontX,
		Config::distortMappingFilenameFrontY
		);
	std::cout << "done!" << std::endl;
	std::cout << "  > generating front camera undistortion mappings.. ";
	CameraTranslator::CameraMapSet mapSet = cameraTranslator->generateInverseMap(cameraTranslator->distortMapX, cameraTranslator->distortMapY);
	cameraTranslator->undistortMapX = mapSet.x;
	cameraTranslator->undistortMapY = mapSet.y;
	std::cout << "done!" << std::endl;


	/// WITH DISTORITION
	x = 423;
	y = 543;
	worldpos = cameraTranslator->getWorldPosition(x, y);
	camerapos = cameraTranslator->getCameraPosition(worldpos.dx, worldpos.dy);
	BOOST_TEST(x - 2 < camerapos.x < x + 2);
	BOOST_TEST(y - 2 < camerapos.y < y + 2);

	x = 1073;
	y = 1000;
	worldpos = cameraTranslator->getWorldPosition(x, y);
	camerapos = cameraTranslator->getCameraPosition(worldpos.dx, worldpos.dy);
	BOOST_TEST(x - 2 < camerapos.x < x + 2);
	BOOST_TEST(y - 2 < camerapos.y < y + 2);

	position = Math::Vector(-2.226856165449925, 0.025085557251647718);
	expectation = Math::Vector(64.5542832971654, 646.4778128810209);
	camerapos = cameraTranslator->getCameraPosition(position);
	BOOST_TEST(camerapos.x == expectation.x);
	BOOST_TEST(camerapos.y == expectation.y);
	*/
}