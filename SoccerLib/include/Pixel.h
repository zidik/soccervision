#ifndef PIXEL_H
#define PIXEL_H


struct Pixel {
	Pixel() : x(0), y(0) {}
	Pixel(int x, int y) : x(x), y(y) {}

	float distanceTo(const Pixel& b) const {
		return sqrt(pow((float)(x - b.x), (float)2) + pow((float)(y - b.y), (float)2));
	}

	int x;
	int y;
};


#endif