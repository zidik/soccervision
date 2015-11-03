#ifndef IMAGEPROCESSOR_H
#define IMAGEPROCESSOR_H

#include <libyuv.h>

#include <string>

class ImageProcessor {

public:
	struct YUYV {
		YUYV() : y1(0), u(0), y2(0), v(0) {}

        int y1, u, y2, v;
    };

	struct YUYVRange {
		int minY, maxY;
		int minU, maxU;
		int minV, maxV;
	};

	static void bayerRGGBToI420(unsigned char* input, unsigned char* outputY, unsigned char* outputU, unsigned char* outputV, int width, int height);
	static void I420ToYUYV(unsigned char* inputY, unsigned char* inputU, unsigned char* inputV, unsigned char* output, int width, int height);
	static void YUYVToARGB(unsigned char* input, unsigned char* output, int width, int height);
	static void ARGBToBGR(unsigned char* input, unsigned char* output, int width, int height);
	static void ARGBToRGB(unsigned char* input, unsigned char* output, int width, int height);
	static bool rgbToJpeg(unsigned char* input, unsigned char* output, int& bufferSize, int width, int height, int channels = 3);
	static YUYV* getYuyvPixelAt(unsigned char* dataY, unsigned char* dataU, unsigned char* dataV, int width, int height, int x, int y);
	static YUYVRange extractColorRange(unsigned char* dataY, unsigned char* dataU, unsigned char* dataV, int imageWidth, int imageHeight, int centerX, int centerY, int brushRadius, float stdDev);
	static bool saveBitmap(unsigned char* data, std::string filename, int size);
	static bool loadBitmap(std::string filename, unsigned char* buffer, int size);
	static bool saveJPEG(unsigned char* data, std::string filename, int width, int height, int channels = 3);

};

#endif // IMAGEPROCESSOR_H