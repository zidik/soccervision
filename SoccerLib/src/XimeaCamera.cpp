#include "XimeaCamera.h"

#include <iostream>
#include <vector>

XimeaCamera::XimeaCamera(int serial) : opened(false), acquisitioning(false), serialNumber(serial){
    image.size = sizeof(XI_IMG);
    image.bp = NULL;
    image.bp_size = 0;
    device = NULL;
    frame.data = NULL;
}

XimeaCamera::~XimeaCamera() {
    close();
}

unsigned long XimeaCamera::getNumberDevices()
{
	DWORD deviceCount = 0;
	xiGetNumberDevices(&deviceCount);
	return deviceCount;
}

void XimeaCamera::open() {
	//std::cout << "! Searching for a camera with serial: " << serial << std::endl;

	unsigned long deviceCount = getNumberDevices();
	//std::cout << "  > found " << deviceCount << " available devices" << std::endl;

    int sn = 0;
    bool found = false;

    for (unsigned int i = 0; i < deviceCount; i++) {
		//std::cout << "  > opening camera #" << i << ".. ";
        xiOpenDevice(i, &device);
		//std::cout << "done!" << std::endl;

        xiGetParamInt(device, XI_PRM_DEVICE_SN, &sn);
        std::cout << "  > found camera with serial number: " << sn << ".. ";

        if (serialNumber == sn) {
            found = true;
			//std::cout << "match found!" << std::endl;
			break;
        }
		
		//std::cout << "not the right one, closing it" << std::endl;
        xiCloseDevice(device);
    }

	if (found)
	{
		opened = true;
		//xiSetParamInt(device, XI_PRM_EXPOSURE, 16000);
		//xiSetParamInt(device, XI_PRM_IMAGE_DATA_FORMAT, XI_MONO8);
		//xiSetParamInt(device, XI_PRM_IMAGE_DATA_FORMAT, XI_RGB24);
		//xiSetParamInt(device, XI_PRM_BUFFER_POLICY, XI_BP_UNSAFE);
		//xiSetParamInt(device, XI_PRM_FRAMERATE, 60);
		//xiSetParamInt(device, XI_PRM_DOWNSAMPLING, 2); // @TEMP
		//xiSetParamInt(device, XI_PRM_DOWNSAMPLING_TYPE, XI_BINNING);
		//xiSetParamFloat(device, XI_PRM_GAIN, 5.0f);
		//xiSetParamInt(device, XI_PRM_ACQ_BUFFER_SIZE, 70*1000*1000);
		//xiSetParamInt(device, XI_PRM_BUFFERS_QUEUE_SIZE, 1);
		//xiSetParamInt(device, XI_PRM_RECENT_FRAME, 1);
		//xiSetParamInt(device, XI_PRM_AUTO_WB, 0);
		//xiSetParamFloat(device, XI_PRM_WB_KR, 1.0f);
		//xiSetParamFloat(device, XI_PRM_WB_KG, 1.0f);
		//xiSetParamFloat(device, XI_PRM_WB_KB, 1.0f);
		//xiSetParamFloat(device, XI_PRM_GAMMAY, 1.0f);
		//xiSetParamFloat(device, XI_PRM_GAMMAC, 1.0f);
		//xiSetParamFloat(device, XI_PRM_SHARPNESS, 0.0f);
		//xiSetParamInt(device, XI_PRM_AEAG, 0);
		//xiSetParamInt(device, XI_PRM_BPC, 1); // fixes bad pixel
		//xiSetParamInt(device, XI_PRM_HDR, 1);
		
	}
}

std::vector<int> XimeaCamera::getAvailableSerials() {
	std::vector<int> serials;

	unsigned long deviceCount = getNumberDevices();

	for (unsigned int i = 0; i < deviceCount; i++) {
		int sn;
		xiOpenDevice(i, &device);
		xiGetParamInt(device, XI_PRM_DEVICE_SN, &sn);
		serials.push_back(sn);
		xiCloseDevice(device);
	}

	return serials;
}



XimeaCamera::Frame* XimeaCamera::getFrame() {
	if (!opened) {
		return NULL;
	}

    xiGetImage(device, 1000, &image);
    //xiGetImage(device, 64, &image);

    if (image.bp == NULL) {
        return NULL;
    }

    frame.data = (unsigned char*)image.bp;
    frame.size = image.bp_size;
    frame.number = image.nframe;
    frame.width = image.width;
    frame.height = image.height;
    frame.timestamp = (double)image.tsSec + (double)image.tsUSec / 1000000.0;
    frame.fresh = frame.number != lastFrameNumber;

    lastFrameNumber = frame.number;

    return &frame;
}

void XimeaCamera::startAcquisition() {
    if (!opened) {
		std::cout << "- Unable to start acquisition, open camera first" << std::endl;

        return;
    }

    xiStartAcquisition(device);

	acquisitioning = true;
}

void XimeaCamera::stopAcquisition() {
    if (!opened) {
		std::cout << "- Unable to stop acquisition, open camera first" << std::endl;

        return;
    }

	if (!acquisitioning) {
		std::cout << "- Unable to stop acquisition, not started" << std::endl;

        return;
    }

    xiStopAcquisition(device);

	acquisitioning = false;
}

void XimeaCamera::close() {
    if (!opened) {
		return;
	}

	if (acquisitioning) {
		stopAcquisition();
	}

    xiCloseDevice(device);

    device = NULL;

	opened = false;
}


std::string XimeaCamera::getStringParam(const char* name) {
	if (!opened) {
		return "n/a";
	}

    char stringParam[254];

    xiGetParamString(device, name, stringParam, sizeof(stringParam));

    return std::string(stringParam);
}

int XimeaCamera::getIntParam(const char* name) {
	if (!opened) {
		return -1;
	}

    int intParam = 0;

    xiGetParamInt(device, name, &intParam);

    return intParam;
}

float XimeaCamera::getFloatParam(const char* name) {
	if (!opened) {
		return -1.0f;
	}

    float floatParam = 0;

    xiGetParamFloat(device, name, &floatParam);

    return floatParam;
}

void XimeaCamera::setStringParam(const char* name, std::string value) {
	if (!opened) {
		return;
	}

    xiSetParamString(device, name, (void*)value.c_str(), value.length());
}

void XimeaCamera::setIntParam(const char* name, int value) {
	if (!opened) {
		return;
	}

    xiSetParamInt(device, name, value);
}

void XimeaCamera::setFloatParam(const char* name, float value) {
	if (!opened) {
		return;
	}

    xiSetParamFloat(device, name, value);
}
