#include "ParticleFilterLocalizer.h"
#include <boost/test/unit_test.hpp>



/*
BOOST_AUTO_TEST_CASE(Integration)
{
	CameraTranslator* frontCameraTranslator = new CameraTranslator();
	CameraTranslator* rearCameraTranslator = new CameraTranslator();

	float A = 120.11218157847301f;
	float B = -0.037205566171594123f;
	float C = 0.2124259596292023f;
	float horizon = 119.40878f;

	// TODO Add to config or load from file
	frontCameraTranslator->setConstants(
		A, B, C,
		horizon,
		Config::cameraWidth, Config::cameraHeight
		);

	// rear parameters
	A = 116.87509670118826f;
	B = -0.024224799830663904f;
	C = 0.20843106680747164f;
	horizon = 123.73853f;

	rearCameraTranslator->setConstants(
		A, B, C,
		horizon,
		Config::cameraWidth, Config::cameraHeight
		);

	std::cout << "  > loading front camera distortion mappings.. ";
	frontCameraTranslator->loadDistortionMapping(
		Config::distortMappingFilenameFrontX,
		Config::distortMappingFilenameFrontY
		);
	std::cout << "done!" << std::endl;

	std::cout << "  > generating front camera undistortion mappings.. ";
	CameraTranslator::CameraMapSet mapSet = frontCameraTranslator->generateInverseMap(frontCameraTranslator->distortMapX, frontCameraTranslator->distortMapY);
	frontCameraTranslator->undistortMapX = mapSet.x;
	frontCameraTranslator->undistortMapY = mapSet.y;
	std::cout << "done!" << std::endl;


	std::cout << "  > loading rear camera distortion mappings.. ";
	rearCameraTranslator->loadDistortionMapping(
		Config::distortMappingFilenameRearX,
		Config::distortMappingFilenameRearY
		);
	std::cout << "done!" << std::endl;


	std::cout << "  > generating rear camera undistortion mappings.. ";
	mapSet = rearCameraTranslator->generateInverseMap(rearCameraTranslator->distortMapX, rearCameraTranslator->distortMapY);
	rearCameraTranslator->undistortMapX = mapSet.x;
	rearCameraTranslator->undistortMapY = mapSet.y;
	std::cout << "done!" << std::endl;

	ParticleFilterLocalizer* robotLocalizer = new ParticleFilterLocalizer(frontCameraTranslator, rearCameraTranslator);

	robotLocalizer->addLandmark(
		"yellow-center",
		0.0f,
		Config::fieldHeight / 2.0f
	);

	robotLocalizer->addLandmark(
		"blue-center",
		Config::fieldWidth,
		Config::fieldHeight / 2.0f
	);
	
	robotLocalizer->setPosition(2.0f, 1.0f, 0.0f);

	ParticleFilterLocalizer::Measurements measurements;
	measurements.clear();

	CameraTranslator::CameraPosition yellowGoal = rearCameraTranslator->getCameraPosition(Math::Vector(2.0f, 0.5f));
	CameraTranslator::CameraPosition blueGoal = rearCameraTranslator->getCameraPosition(Math::Vector(2.5f, -0.5f));

	measurements["yellow-center"] = ParticleFilterLocalizer::Measurement(
		Math::Vector(yellowGoal.x, yellowGoal.y),
		Dir::REAR);

	measurements["blue-center"] = ParticleFilterLocalizer::Measurement(
		Math::Vector(blueGoal.x, blueGoal.y),
		Dir::FRONT);
	while (true) {
		robotLocalizer->update(measurements);
		robotLocalizer->move(0.0f, 0.0f, 0.0f, 1.0f / 60);
		robotLocalizer->calculatePosition();
		Math::Position localizerPosition = robotLocalizer->getPosition();
	}
}
*/