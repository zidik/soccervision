#ifndef CANVAS_H
#define CANVAS_H

#include <cstddef>
#include <string>

class Canvas {

public:
    Canvas() : data(NULL), width(-1), height(-1) { maxIndex = width * height * 3 - 2; }
    Canvas(unsigned char* data, int width, int height) : data(data), width(width), height(height) {}

    void setPixelAt(int x, int y, int red = 255, int green = 0, int blue = 0);
    void drawBox(int x, int y, int width, int height, int red = 255, int green = 0, int blue = 0);
    void fillBox(int x, int y, int width, int height, int red = 255, int green = 0, int blue = 0);
    void drawBoxCentered(int x, int y, int width, int height, int red = 255, int green = 0, int blue = 0) {
        drawBox(x - width / 2, y - height / 2, width, height, red, green, blue);
    }
	void fillBoxCentered(int x, int y, int width, int height, int red = 255, int green = 0, int blue = 0) {
        fillBox(x - width / 2, y - height / 2, width, height, red, green, blue);
    }
	void drawCircle(int centerX, int centerY, int radius, int red = 255, int green = 0, int blue = 0);
	void fillCircle(int centerX, int centerY, int radius, int red = 255, int green = 0, int blue = 0);
    void drawChar(int imageX, int imageY, int index, int red = 255, int green = 0, int blue = 0);
    void drawText(int imageX, int imageY, std::string text, int red = 255, int green = 0, int blue = 0, bool clearBackground = true);
    void drawLine(int x1, int y1, int x2, int y2, int red = 255, int green = 0, int blue = 0);
    void drawMarker(int x, int y, int red = 255, int green = 0, int blue = 0, bool tiny = false);

    unsigned char* data;
    int width;
    int height;

private:
    static const unsigned char font[256][8];
	int maxIndex;

};

#endif // CANVAS_H
