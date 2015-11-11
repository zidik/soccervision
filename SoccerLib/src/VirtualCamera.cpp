#include "VirtualCamera.h"
#include "ImageProcessor.h"
#include "Config.h"
#include "Util.h"

VirtualCamera::VirtualCamera() : data(NULL), bufferSize(0), frameNr(0) {

}

bool VirtualCamera::loadImage(std::string filename, int size) {
	if (data == NULL) {
		data = new unsigned char[size];
	}

	bufferSize = size;

	return ImageProcessor::loadBitmap(filename, data, size);
}

BaseCamera::Frame* VirtualCamera::getFrame() {
	frame.data = data;
    frame.size = bufferSize;
    frame.number = frameNr++;
	frame.width = Config::cameraWidth;
	frame.height = Config::cameraHeight;
	frame.timestamp = Util::millitime();
    frame.fresh = true;

	return &frame;
}