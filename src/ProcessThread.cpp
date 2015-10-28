#include "ProcessThread.h"
#include "Blobber.h"
#include "ImageProcessor.h"
#include "DebugRenderer.h"
#include "Canvas.h"
#include "BaseCamera.h"
#include "Util.h"
#include "Config.h"

#include <iostream>

ProcessThread::ProcessThread(BaseCamera* camera, Blobber* blobber, Vision* vision) : Thread(), dir(dir), camera(camera), blobber(blobber), vision(vision), visionResult(NULL), debug(false), gotFrame(false), faulty(false), done(true) {
	frame = NULL;
	width = blobber->getWidth();
	height = blobber->getHeight();
	dataY = new unsigned char[width * height];
    dataU = new unsigned char[(width / 2) * (height / 2)];
    dataV = new unsigned char[(width / 2) * (height / 2)];
	dataYUYV = new unsigned char[width * height * 3];
	classification = new unsigned char[width * height * 3];
	argb = new unsigned char[width * height * 4];
	rgb = new unsigned char[width * height * 3];

	visionResult = new Vision::Result();
	visionResult->vision = vision;
}

ProcessThread::~ProcessThread() {
	if (visionResult != NULL) {
		delete visionResult;
		visionResult = NULL;
	}

	delete dataY;
	delete dataU;
	delete dataV;
	delete dataYUYV;
	delete classification;
	delete argb;
	delete rgb;
}

void* ProcessThread::run() {
	gotFrame = fetchFrame();

	if (!gotFrame || frame == NULL) {
		if (faulty) {
			if (visionResult != NULL) {
				delete visionResult;
				visionResult = NULL;
			}

			// fetching frame failed, create empty result set
			visionResult = new Vision::Result();
			visionResult->vision = vision;

			std::cout << "- Getting frame failed and faulty camera detected, creating blank results" << std::endl;
		} else {
			//std::cout << "! Getting frame failed, using previous data" << std::endl;
		}

		return NULL;
	}

	if (visionResult != NULL) {
		delete visionResult;
		visionResult = NULL;
	}

	done = false;
	
	//Util::timerStart();
	ImageProcessor::bayerRGGBToI420(
		frame,
		dataY, dataU, dataV,
		width, height
	);
	//std::cout << "  - RGGB > I420: " << Util::timerEnd() << std::endl;

	//Util::timerStart();
	ImageProcessor::I420ToYUYV(
		dataY, dataU, dataV,
		dataYUYV,
		width, height
	);
	//std::cout << "  - I420 > YUYV: " << Util::timerEnd() << std::endl;

	//Util::timerStart();
	blobber->processFrame((Blobber::Pixel*)dataYUYV);
	//std::cout << "  - Process:     " << Util::timerEnd() << " (" << blobber->getBlobCount("ball") << " ball blobs)" << std::endl;

	if (debug) {
		//Util::timerStart();
		blobber->classify((Blobber::Rgb*)classification, (Blobber::Pixel*)dataYUYV);
		//std::cout << "  - Blobber classify: " << Util::timerEnd() << std::endl;

		//Util::timerStart();
		ImageProcessor::YUYVToARGB(dataYUYV, argb, width, height);
		//std::cout << "  - YUYV > ARGB: " << Util::timerEnd() << std::endl;

		//Util::timerStart();
		//ImageProcessor::ARGBToBGR(
		ImageProcessor::ARGBToRGB(
			argb,
			rgb,
			width, height
		);
		//std::cout << "  - ARGB > RGB: " << Util::timerEnd() << std::endl;

		vision->setDebugImage(rgb, width, height);
	} else {
		vision->setDebugImage(NULL, 0, 0);
	}

	if (debug) {
		//DebugRenderer::renderMapping(rgb, vision);
		DebugRenderer::renderGrid(rgb, vision);
	}

	visionResult = vision->process();

	if (debug) {
		DebugRenderer::renderBlobs(classification, blobber);
		DebugRenderer::renderBalls(rgb, vision, visionResult->balls);
		DebugRenderer::renderRobots(rgb, vision, visionResult->robots);
		DebugRenderer::renderGoals(rgb, visionResult->goals);
		//DebugRenderer::renderObstructions(rgb, visionResult->goalPathObstruction);
		
		// TODO Show whether a ball is in the way
	}

	done = true;

	return NULL;
}

bool ProcessThread::fetchFrame() {
	if (camera->isAcquisitioning()) {
		double startTime = Util::millitime();
		
		const BaseCamera::Frame* cameraFrame = camera->getFrame();
		
		double timeTaken = Util::duration(startTime);

		if (timeTaken > 0.03) {
			std::cout << "- Fetching camera #" << camera->getSerial() << " frame took: " << timeTaken << std::endl;

			faulty = true;

			//frame = NULL;

			// don't use this invalid frame
			return false;

			// camera has failed
			/*if (timeTaken > 0.031) {
				int serial = camera->getSerial();

				std::cout << "! Attempting to revive camera #" << serial << std::endl;

				camera->close();
				camera->open(serial);
				camera->startAcquisition();

				return false;
			}*/
		}

		if (cameraFrame != NULL) {
			if (cameraFrame->fresh) {
				frame = cameraFrame->data;

				return true;
			}
		}
	}

	return false;
}