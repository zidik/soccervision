#include <boost/test/unit_test.hpp>
#include <CameraTranslator.h>
#include <ParticleFilterLocalizer.h>
#include <Hacks.h>

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
typedef CameraTranslator::CameraPosition CameraPosition;
void testTranslationLoopback(const CameraTranslator &cameraTranslator, const CameraPosition &initial, bool distortion=true)
{
	Math::Vector worldpos = cameraTranslator.getWorldPosition(initial, distortion);
	CameraPosition camerapos = cameraTranslator.getCameraPosition(worldpos, distortion);
	BOOST_TEST(initial.x - 2 < camerapos.x < initial.x + 2);
	BOOST_TEST(initial.y - 2 < camerapos.y < initial.y + 2);
}

void setupCameraTranslator(CameraTranslator &cameraTranslator)
{
	float A = 120.11218157847301f;
	float B = -0.037205566171594123f;
	float C = 0.2124259596292023f;
	float horizon = 119.40878f;
	cameraTranslator.setConstants(
		A, B, C,
		0.0f, 0.0f, 0.0f,
		horizon, 0.0f,
		Config::cameraWidth, Config::cameraHeight
	);
}
void loadDistortionMaps(CameraTranslator &cameraTranslator)
{
	std::cout << "  > loading camera distortion mappings.. ";
	cameraTranslator.loadDistortionMapping(
		Config::distortMappingFilenameFrontX,
		Config::distortMappingFilenameFrontY
		);
	std::cout << "done!" << std::endl;
	std::cout << "  > generating front camera undistortion mappings.. ";
	CameraTranslator::CameraMapSet mapSet = cameraTranslator.generateInverseMap(cameraTranslator.distortMapX, cameraTranslator.distortMapY);
	cameraTranslator.undistortMapX = mapSet.x;
	cameraTranslator.undistortMapY = mapSet.y;
	std::cout << "done!" << std::endl;
}

BOOST_AUTO_TEST_CASE(cameraTranslationLoopbackTest)
{
	CameraTranslator cameraTranslator;
	setupCameraTranslator(cameraTranslator);

	CameraPosition initial;

	/// WITHOUT DISTORITION
	initial= CameraPosition(423, 543);
	testTranslationLoopback(cameraTranslator, initial, false);
	initial = CameraPosition(1073, 1000);
	testTranslationLoopback(cameraTranslator, initial, false);

	loadDistortionMaps(cameraTranslator);

	/// WITH DISTORITION
	initial = CameraPosition(423, 543);
	testTranslationLoopback(cameraTranslator, initial, true);
	initial = CameraPosition(1073, 1000);
	testTranslationLoopback(cameraTranslator, initial, true);
}