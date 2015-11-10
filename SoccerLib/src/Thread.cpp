#include "Thread.h"

static void* runThread(void* arg) {
    return ((Thread*)arg)->run();
}

Thread::Thread() : running(0), detached(0) {}

Thread::~Thread() {
    if (running) {
		if (!detached) {
			pthread_detach(handle);
		}

        pthread_cancel(handle);
    }
}

int Thread::start() {
    int result = pthread_create(&handle, NULL, runThread, this);

    if (result == 0) {
        running = true;
    }

    return result;
}

int Thread::join() {
    int result = -1;

    if (running) {
        result = pthread_join(handle, NULL);

        if (result == 0) {
            detached = false;
        }
    }

    return result;
}

int Thread::detach() {
    int result = -1;

    if (running && !detached) {
        result = pthread_detach(handle);

        if (result == 0) {
            detached = 1;
        }
    }

    return result;
}

pthread_t Thread::self() {
    return handle;
}
