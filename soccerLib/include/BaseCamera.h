#ifndef CAMERA_H
#define CAMERA_H

class BaseCamera {

public:
	struct Frame {
        unsigned char* data;
        int size;
        int width;
        int height;
        int number;
		bool fresh;
        double timestamp;
    };

	virtual Frame* getFrame() = 0;
	virtual int getSerial() { return -1; }
	virtual bool open(int serial = 0) { return true; }
	virtual bool isOpened() { return true; }
	virtual bool isAcquisitioning() { return true; }
	virtual void startAcquisition() {}
	virtual void stopAcquisition() {}
	virtual void close() {}

};

#endif // CAMERA_H