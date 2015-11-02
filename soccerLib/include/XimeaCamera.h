#ifndef XIMEACAMERA_H
#define XIMEACAMERA_H

#include "BaseCamera.h"

#include <Windows.h>
#include "xiApi.h"
//#include "xiExt.h"
//#include "m3Ext.h"

#include <string>

class XimeaCamera : public BaseCamera {

public:
    XimeaCamera();
    ~XimeaCamera();

    bool open(int serial = 0);
	int getSerial() { return getSerialNumber(); }
	bool isOpened() { return opened; };
	bool isAcquisitioning() { return acquisitioning; };
	void startAcquisition();
    void stopAcquisition();
    void close();
    Frame* getFrame();
    
    std::string getName() { return getStringParam(XI_PRM_DEVICE_NAME); }
    std::string getDeviceType() { return getStringParam(XI_PRM_DEVICE_TYPE); }
    std::string getApiVersion() { return getStringParam(XI_PRM_API_VERSION XI_PRM_INFO); }
    std::string getDriverVersion() { return getStringParam(XI_PRM_DRV_VERSION XI_PRM_INFO); }
    int getSerialNumber() { return getIntParam(XI_PRM_DEVICE_SN); }
    bool supportsColor() { return getIntParam(XI_PRM_IMAGE_IS_COLOR) == 1; }
    int getAvailableBandwidth() { return getIntParam(XI_PRM_AVAILABLE_BANDWIDTH); }
	int getExposure() { return getIntParam(XI_PRM_EXPOSURE); }
    int getGain() { return getIntParam(XI_PRM_GAIN); }
    int getFramerate() { return getIntParam(XI_PRM_FRAMERATE); }

    void setFormat(int format) { setIntParam(XI_PRM_IMAGE_DATA_FORMAT, format); }
    void setExposure(int microseconds) { setIntParam(XI_PRM_EXPOSURE, microseconds); }
    void setGain(int value) { setIntParam(XI_PRM_GAIN, value); }
    void setDownsampling(int times) { setIntParam(XI_PRM_DOWNSAMPLING, times); }
    void setWhiteBalanceRed(float value) { setFloatParam(XI_PRM_WB_KR, value); }
    void setWhiteBalanceGreen(float value) { setFloatParam(XI_PRM_WB_KG, value); }
    void setWhiteBalanceBlue(float value) { setFloatParam(XI_PRM_WB_KB, value); }
    void setLuminosityGamma(float value) { setFloatParam(XI_PRM_GAMMAY, value); }
    void setChromaticityGamma(float value) { setFloatParam(XI_PRM_GAMMAC, value); }
    void setAutoWhiteBalance(bool enabled) { setIntParam(XI_PRM_AUTO_WB, enabled ? 1 : 0); }
    void setAutoExposureGain(bool enabled) { setIntParam(XI_PRM_AEAG, enabled ? 1 : 0); }
    void setQueueSize(int size) { setIntParam(XI_PRM_BUFFERS_QUEUE_SIZE, size); }
    void setAcquisitionBufferSize(int size) { setIntParam(XI_PRM_ACQ_BUFFER_SIZE, size); }

    std::string getStringParam(const char* name);
    int getIntParam(const char* name);
    float getFloatParam(const char* name);

    void setStringParam(const char* name, std::string value);
    void setIntParam(const char* name, int value);
    void setFloatParam(const char* name, float value);

private:
    XI_IMG image;
	Frame frame;
    HANDLE device;
    bool opened;
	bool acquisitioning;
	int serialNumber;
    int lastFrameNumber;

};

#endif // XIMEACAMERA_H
