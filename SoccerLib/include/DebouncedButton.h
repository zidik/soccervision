#ifndef DEBOUNCEDBUTTON_H
#define DEBOUNCEDBUTTON_H

class DebouncedButton {

public:
	DebouncedButton(float period = 0.1f);

	bool toggle();

private:
	double lastChangeTime;
	float period;

};

#endif // DEBOUNCEDBUTTON_H