#include "DebouncedButton.h"
#include "Util.h"

#include <iostream>

DebouncedButton::DebouncedButton(float period) : period(period), lastChangeTime(0) {}

bool DebouncedButton::toggle() {
	if (Util::duration(lastChangeTime) > (double)period) {
		lastChangeTime = Util::millitime();

		return true;
	}

	return false;
}
