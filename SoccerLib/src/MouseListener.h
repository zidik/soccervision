#ifndef MOUSELISTENER_H
#define MOUSELISTENER_H

class DisplayWindow;

class MouseListener {

public:
	enum MouseBtn {
		LEFT = 0,
		MIDDLE = 1,
		RIGHT = 2
	};

	virtual void onMouseMove(int x, int y, DisplayWindow* win) {}
	virtual void onMouseDown(int x, int y, MouseBtn btn, DisplayWindow* win) {}
	virtual void onMouseUp(int x, int y, MouseBtn btn, DisplayWindow* win) {}
	virtual void onMouseWheel(int delta, DisplayWindow* win) {}

};

#endif // MOUSELISTENER_H