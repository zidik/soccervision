#ifndef THREAD_H
#define THREAD_H

#include <pthread.h>

class Thread {

public:
	Thread();
	virtual ~Thread();

	int start();
	int join();
	int detach();
	pthread_t self();
    
	virtual void* run() = 0;
    
private:
	pthread_t handle;
	bool running;
	bool detached;

};

#endif // THREAD_H
