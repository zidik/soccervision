#ifndef DEBUGRENDERER_H
#define DEBUGRENDERER_H

#include "Object.h"
#include "Config.h"
#include "Blobber.h"

class Canvas;
class Vision;

class DebugRenderer {

public:
	static void renderFPS(unsigned char* image, int fps, int width = Config::cameraWidth, int height = Config::cameraHeight);
	static void renderBlobs(unsigned char* image, Blobber* blobber, int width = Config::cameraWidth, int height = Config::cameraHeight);
	static void renderBalls(unsigned char* image, Vision* vision, const ObjectList& balls, int width = Config::cameraWidth, int height = Config::cameraHeight);
	static void renderRobots(unsigned char* image, Vision* vision, const ObjectList& robots, int width = Config::cameraWidth, int height = Config::cameraHeight);
	static void renderGoals(unsigned char* image, const ObjectList& goals, int width = Config::cameraWidth, int height = Config::cameraHeight);
	static void renderBrush(unsigned char* image, int x, int y, int radius, bool active, int width = Config::cameraWidth, int height = Config::cameraHeight);
	//static void renderObstructions(unsigned char* image, Vision::Obstruction obstruction, int width = Config::cameraWidth, int height = Config::cameraHeight);
	static void renderObjectHighlight(unsigned char* image, Object* object, int red = 255, int green = 255, int blue = 255, int width = Config::cameraWidth, int height = Config::cameraHeight);
	static void renderGrid(unsigned char* image, Vision* vision, int width = Config::cameraWidth, int height = Config::cameraHeight);
	static void renderMapping(unsigned char* image, Vision* vision, int width = Config::cameraWidth, int height = Config::cameraHeight);
};

#endif // DEBUGRENDERER_H