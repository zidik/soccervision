#ifndef PROCESSTHREAD_H
#define PROCESSTHREAD_H

#include "Thread.h"
#include "Config.h"
#include "Vision.h"

class BaseCamera;
class Blobber;

class ProcessThread : public Thread {

public:
	ProcessThread(BaseCamera* camera, Blobber* blobber, Vision* vision);
	~ProcessThread();

	//void setFrame(unsigned char* data) { frame = data; };
	bool isDone() { return done; };

	int width;
	int height;
	Dir dir;

	bool debug;

	BaseCamera* camera;
	Blobber* blobber;
	Vision* vision;
	Vision::Result* visionResult;

	bool gotFrame;
	bool faulty;
	unsigned char* frame;
	unsigned char* dataYUYV;
	unsigned char* dataY;
    unsigned char* dataU;
    unsigned char* dataV;
	unsigned char* classification;
	unsigned char* argb;
	unsigned char* rgb;

private:
	void* run();
	bool fetchFrame();

	bool done;
};

#endif // PROCESSTHREAD_H