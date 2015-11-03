#ifndef VIRTUALCAMERA_H
#define VIRTUALCAMERA_H

#include "BaseCamera.h"

#include <string>

class VirtualCamera : public BaseCamera {

public:
	VirtualCamera();

	Frame* getFrame();
	bool isAcquisitioning() { return data != NULL; }
	bool loadImage(std::string filename, int size);

private:
	Frame frame;
	unsigned char* data;
	int bufferSize;
	int frameNr;

};

#endif //VIRTUALCAMERA_H