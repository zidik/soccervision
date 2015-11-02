#include "FpsCounter.h"
#include "Util.h"

FpsCounter::FpsCounter(int interval) : interval(interval), firstInterval(true) {
    startTime = -1;
    frames = 0;
	frameNumber = 0;
    fps = 0;
    changed = false;
}

void FpsCounter::step() {
	frameNumber++;

    if (startTime == -1) {
        startTime = Util::millitime();
        frames = 1;

        return;
    }

    if (firstInterval || frames >= interval) {
        double currentTime = Util::millitime();
        double elapsedTime = currentTime - startTime;

        fps = (int)((double)frames / elapsedTime);
        startTime = currentTime;
        frames = 0;

        changed = true;
		firstInterval = false;
    } else {
        frames++;
    }
}

bool FpsCounter::isChanged() {
    if (changed) {
        changed = false;

        return true;
    } else {
        return false;
    }
}

int FpsCounter::getFps() {
    return fps;
}
