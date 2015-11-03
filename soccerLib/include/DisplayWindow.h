#ifndef DISPLAY_H
#define DISPLAY_H

#include "Canvas.h"
#include <Windows.h>
#include <string>

class Canvas;
class Command;
class Gui;

class DisplayWindow {

public:
    DisplayWindow(HINSTANCE instance, int width, int height, std::string name = "Window", Gui* gui = NULL);
    ~DisplayWindow();

    void setImage(unsigned char* image, bool rgb2bgr = true);
    static bool windowsVisible() { return 0; /* TODO! */ }
	LRESULT handleMessage(HWND windowHandle, UINT msg, WPARAM wParam, LPARAM lParam);

private:
	HINSTANCE instance;
	HWND windowHandle;
	BITMAPINFO bitmapInfo;
	HDC windowDeviceHandle;
	HDC bitmapDeviceHandle;
	HBITMAP bitmap;
	Canvas* canvas;
	Gui* gui;
    int width;
    int height;
	std::string name;
	bool firstDraw;

};

#endif // DISPLAY_H
