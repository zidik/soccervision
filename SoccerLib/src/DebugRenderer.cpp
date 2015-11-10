#include "DebugRenderer.h"
#include "CameraTranslator.h"
#include "Canvas.h"
#include "Maths.h"
#include "Vision.h"
#include "Util.h"
#include "Pixel.h"

void DebugRenderer::renderFPS(unsigned char* image, int fps, int width, int height) {
	Canvas canvas = Canvas();

	canvas.data = image;
	canvas.width = width;
	canvas.height = height;

	canvas.drawText(20, 20, "FPS: " + Util::toString(fps));
}

void DebugRenderer::renderBlobs(unsigned char* image, Blobber* blobber, int width, int height) {
	Canvas canvas = Canvas();

	canvas.data = image;
	canvas.width = width;
	canvas.height = height;

	for (int i = 0; i < blobber->getColorCount(); i++) {
		Blobber::Color* color = blobber->getColor(i);

		if (color == NULL) {
			continue;
		}

		if (
			strcmp(color->name, "ball") != 0
			&& strcmp(color->name, "yellow-goal") != 0
			&& strcmp(color->name, "blue-goal") != 0
		) {
			continue;
		}

		Blobber::Blob* blob = blobber->getBlobs(color->name);

		while (blob != NULL) {
			canvas.drawBoxCentered(
				(int)blob->centerX, (int)blob->centerY,
				blob->x2 - blob->x1, blob->y2 - blob->y1,
				color->color.red, color->color.green, color->color.blue
			);

			blob = blob->next;
		}
	}
}

void DebugRenderer::renderBalls(unsigned char* image, Vision* vision, const ObjectList& balls, int width, int height) {
	Canvas canvas = Canvas();

	canvas.data = image;
	canvas.width = width;
	canvas.height = height;

	Object* ball = NULL;
	float ballFutureX, ballFutureY;
	Pixel futureBallPos, realBallPos;
	size_t ballNumber;
    char buf[256];
	//int correctedX, correctedY;

    for (ObjectListItc it = balls.begin(); it != balls.end(); it++) {
        ball = *it;

		ballNumber = it - balls.begin();

		//calculate estimated future ball position based on movement vector
		ballFutureX = ball->distanceX + 6 * ball->relativeMovement.dX;
		ballFutureY = ball->distanceY + 6 * ball->relativeMovement.dY;

		//get pixel on screen where ball should be
		futureBallPos = vision->getCameraTranslator()->getCameraPosition(Math::Vector(ballFutureX, ballFutureY));
		realBallPos = vision->getCameraTranslator()->getCameraPosition(Math::Vector(ball->distanceX, ball->distanceY));

        canvas.drawBoxCentered(ball->x, ball->y, ball->width, ball->height);
		//canvas.drawLine(ball->x - ball->width / 2, ball->y - ball->height / 2, ball->x + ball->width / 2, ball->y + ball->height / 2);
        //canvas.drawLine(ball->x - ball->width / 2, ball->y + ball->height / 2, ball->x + ball->width / 2, ball->y - ball->height / 2);

		//sprintf(buf, "%.2fm x %.2fm  %.1f deg", ball->distanceX, ball->distanceY, Math::radToDeg(ball->angle));
		sprintf(buf, "%d : %.2fm %.2fm %.2fm/s  %.1f deg, future %.2fm %.2fm", ballNumber, ball->distanceX, ball->distanceY, ball->absoluteMovement.speed, Math::radToDeg(ball->angle), ballFutureX, ballFutureY);

		if (ball->y + ball->height / 2 < Config::cameraHeight - 50) {
			canvas.drawText(ball->x - ball->width / 2 + 2, ball->y + ball->height / 2 + 4, buf);
		} else {
			canvas.drawText(ball->x - ball->width / 2 + 2, ball->y - ball->height / 2 - 10, buf);
		}

		//draw future ball position on screen as orange circle and a line leading to it
		canvas.fillCircle(futureBallPos.x, futureBallPos.y, 3, 255, 155, 0);
		canvas.drawLine(realBallPos.x, realBallPos.y, futureBallPos.x, futureBallPos.y, 255, 155, 0);

		//add marker to mark balls that are not visible
		if (ball->notSeenFrames > 0) canvas.fillBox(ball->x, ball->y, 10, 10, 255, 155, 0);

		//correctedX = ball->x;
		//correctedY = ball->y + ball->height / 2;

		Pixel undistortedPos = vision->getCameraTranslator()->undistort(Pixel(ball->x, ball->y + ball->height / 2));

		//Util::correctCameraPoint(correctedX, correctedY);

		//sprintf(buf, "%d x %d - %d x %d", ball->x, ball->y + ball->height / 2, undistortedPos.x, undistortedPos.y);
        //canvas.drawText(ball->x - ball->width / 2 + 2, ball->y + ball->height / 2 + 14, buf);

        //int boxArea = ball->width * ball->height;

		/*if (boxArea == 0) {
			continue;
		}

        int density = ball->area * 100 / boxArea;

        sprintf(buf, "%d - %d%%", ball->area, density);
        canvas.drawText(ball->x - ball->width / 2 + 2, ball->y - ball->height / 2 - 9, buf);*/
    }

	// TEMP - draw centerline
	//canvas.drawLine(canvas.width / 2, 0, canvas.width / 2, canvas.height);
	//canvas.fillCircleCentered(Config::cameraWidth / 2, Config::cameraHeight / 2, 100, 0, 0, 255);

    /*Blobber::Blob* blob = blobber->getBlobs("ball");

    while (blob != NULL) {
        image->drawBoxCentered(blob->centerX, blob->centerY, blob->x2 - blob->x1, blob->y2 - blob->y1);

        blob = blob->next;
    }*/
}

void DebugRenderer::renderRobots(unsigned char* image, Vision* vision, const ObjectList& robots, int width, int height) {
	Canvas canvas = Canvas();

	canvas.data = image;
	canvas.width = width;
	canvas.height = height;

	Object* robot = NULL;
	float robotFutureX, robotFutureY;
	Pixel futureRobotPos, realRobotPos;
	size_t robotNumber;
	char buf[256];
	int r, g, b;

	for (ObjectListItc it = robots.begin(); it != robots.end(); it++) {
		robot = *it;

		if (robot->type == RobotColor::YELLOWHIGH) {
			r = 50;
			g = 200;
			b = 0;
			sprintf(buf, "yellowHigh");
		}
		else if (robot->type == RobotColor::BLUEHIGH) {
			r = 100;
			g = 0;
			b = 200;
			sprintf(buf, "blueHigh");
		}
		else {
			r = 255;
			g = 0;
			b = 0;
			sprintf(buf, "UNKNOWN");
		}

		robotNumber = it - robots.begin();

		//calculate estimated future robot position based on movement vector
		robotFutureX = robot->distanceX + 6 * robot->relativeMovement.dX;
		robotFutureY = robot->distanceY + 6 * robot->relativeMovement.dY;

		//get pixel on screen where robot should be
		futureRobotPos = vision->getCameraTranslator()->getCameraPosition(Math::Vector(robotFutureX, robotFutureY));
		realRobotPos = vision->getCameraTranslator()->getCameraPosition(Math::Vector(robot->distanceX, robot->distanceY));

		canvas.drawBoxCentered(robot->x, robot->y, robot->width, robot->height, r, g, b);
		canvas.drawCircle(robot->x, robot->y, std::min(robot->width / 2, robot->height / 2), r, g, b);
		canvas.drawLine(robot->x - robot->width / 2, robot->y - robot->height / 2, robot->x + robot->width / 2, robot->y + robot->height / 2, r, g, b);
		canvas.drawLine(robot->x - robot->width / 2, robot->y + robot->height / 2, robot->x + robot->width / 2, robot->y - robot->height / 2, r, g, b);

		canvas.drawText(robot->x + robot->width / 2, robot->y - 8, buf, r, g, b);

		sprintf(buf, "%d : %.2fm %.2fm %.2fm/s %.1f deg, future %.2fm %.2fm", robotNumber, robot->distanceX, robot->distanceY, robot->absoluteMovement.speed, Math::radToDeg(robot->angle), robotFutureX, robotFutureY);
		canvas.drawText(robot->x + robot->width / 2, robot->y + 2, buf, r, g, b);

		sprintf(buf, "%d x %d, %d", robot->x, robot->y + robot->height / 2, robot->area);
		canvas.drawText(robot->x + robot->width / 2, robot->y + 12, buf, r, g, b);

		//draw future robot position on screen as a circle and a line leading to it
		canvas.fillCircle(futureRobotPos.x, futureRobotPos.y, 3, r, g, b);
		canvas.drawLine(realRobotPos.x, realRobotPos.y, futureRobotPos.x, futureRobotPos.y, r, g, b);

		//add marker to mark robots that are not visible
		if (robot->notSeenFrames > 0) canvas.fillBox(robot->x, robot->y, 10, 10, r, g, b);

		/*int boxArea = robot->width * robot->height;

		if (boxArea == 0) {
		continue;
		}

		int density = robot->area * 100 / boxArea;

		sprintf(buf, "%d - %d%%", robot->area, density);
		canvas.drawText(robot->x + robot->width / 2, robot->y + 22, buf);*/
	}
}

void DebugRenderer::renderGoals(unsigned char* image, const ObjectList& goals, int width, int height) {
	Canvas canvas = Canvas();

	canvas.data = image;
	canvas.width = width;
	canvas.height = height;

	Object* goal = NULL;
    char buf[256];
	int r, g, b;

    for (ObjectListItc it = goals.begin(); it != goals.end(); it++) {
        goal = *it;

		if (goal->type == Side::YELLOW) {
			r = 200;
			g = 200;
			b = 0;
		} else {
			r = 0;
			g = 0;
			b = 200;
		}

        canvas.drawBoxCentered(goal->x, goal->y, goal->width, goal->height, r, g, b);
		//canvas.drawLine(goal->x - goal->width / 2, goal->y - goal->height / 2, goal->x + goal->width / 2, goal->y + goal->height / 2, r, g, b);
        //canvas.drawLine(goal->x - goal->width / 2, goal->y + goal->height / 2, goal->x + goal->width / 2, goal->y - goal->height / 2, r, g, b);

        sprintf(buf, "%.2fm %.1f deg", goal->distance, Math::radToDeg(goal->angle));
        canvas.drawText(goal->x - goal->width / 2 + 2, goal->y + goal->height / 2 + 2, buf, r, g, b);

		sprintf(buf, "%d x %d, %d", goal->x, goal->y + goal->height / 2, goal->area);
        canvas.drawText(goal->x - goal->width / 2 + 2, goal->y + goal->height / 2 + 12, buf, r, g, b);

        /*int boxArea = goal->width * goal->height;

		if (boxArea == 0) {
			continue;
		}

        int density = goal->area * 100 / boxArea;

        sprintf(buf, "%d - %d%%", goal->area, density);
        canvas.drawText(goal->x - goal->width / 2 + 2, goal->y - goal->height / 2 - 9, buf);*/
    }
}

void DebugRenderer::renderBrush(unsigned char* image, int x, int y, int radius, bool active, int width, int height) {
	Canvas canvas = Canvas();

	canvas.data = image;
	canvas.width = width;
	canvas.height = height;

	canvas.drawCircle(x, y, radius, active ? 255 : 0, 0, active ? 0 : 255);
}

/*void DebugRenderer::renderObstructions(unsigned char* image, Obstruction obstruction, int width, int height) {
	Canvas canvas = Canvas();

	canvas.data = image;
	canvas.width = width;
	canvas.height = height;

	if (obstruction == Obstruction::BOTH || obstruction == Obstruction::LEFT) {
		canvas.fillBox(width / 2 - 20, height - 80, 20, 40, 200, 0, 0);
	} else {
		canvas.fillBox(width / 2 - 20, height - 80, 20, 40, 0, 200, 0);
	}

	if (obstruction == Obstruction::BOTH || obstruction == Obstruction::RIGHT) {
		canvas.fillBox(width / 2, height - 80, 20, 40, 200, 0, 0);
	} else {
		canvas.fillBox(width / 2, height - 80, 20, 40, 0, 200, 0);
	}
}*/

void DebugRenderer::renderObjectHighlight(unsigned char* image, Object* object, int red, int green, int blue, int width, int height) {
	Canvas canvas = Canvas();

	canvas.data = image;
	canvas.width = width;
	canvas.height = height;

	canvas.drawBoxCentered(object->x, object->y, object->width + 4, object->height + 4, red, green, blue);
	canvas.drawLine(object->x - object->width / 2, object->y - object->height / 2, object->x + object->width / 2, object->y + object->height / 2, red, green, blue);
    canvas.drawLine(object->x - object->width / 2, object->y + object->height / 2, object->x + object->width / 2, object->y - object->height / 2, red, green, blue);
}

void DebugRenderer::renderGrid(unsigned char* image, Vision* vision, int width, int height) {
	Canvas canvas = Canvas();

	canvas.data = image;
	canvas.width = width;
	canvas.height = height;

	float maxDistanceY = 8.0f;
	float stepX = 0.01f;
	float minDistanceX = -4.0f;
	float maxDistanceX = 4.0f;
	float distanceX = 0.0f, distanceY = 0.0f;
	float distanceStartY = 0.125f;
	int counter = 0;
	int lastTextY = -1;
	int xOverflow = 500;
	Math::Vector screenCoords;
	Pixel pos;
	Pixel distorted;
	Pixel undistorted;

	for (distanceY = distanceStartY; distanceY <= maxDistanceY; distanceY *= 2.0f) {
		for (distanceX = minDistanceX; distanceX <= maxDistanceX; distanceX += stepX) {
			pos = vision->getCameraTranslator()->DEPRECATEDgetCameraPosition(distanceX, distanceY);

			canvas.setPixelAt(pos.x, pos.y, 0, 0, 128);

			/*for (int x = -xOverflow; x < Config::cameraWidth + xOverflow; x += 3) {
				distorted = vision->getCameraTranslator()->distort(x, pos.y);
				//undistorted = vision->getCameraTranslator()->undistort(distorted.x, distorted.y);

				canvas.setPixelAt(x, pos.y, 128, 0, 0);
				canvas.setPixelAt(distorted.x, distorted.y, 0, 0, 128);
				//canvas.setPixelAt(undistorted.x, undistorted.y, 128, 0, 0);
			}*/

			/*for (int y = 0; y < Config::cameraHeight; y += 3) {
				distorted = vision->getCameraTranslator()->distort(pos.x, y);
				//undistorted = vision->getCameraTranslator()->undistort(distorted.x, distorted.y);

				//canvas.setPixelAt(x, y, 0, 0, 128);
				canvas.setPixelAt(distorted.x, distorted.y, 0, 0, 128);
				//canvas.setPixelAt(undistorted.x, undistorted.y, 128, 0, 0);
			}*/

			//px = 10 + (counter % 10) * 30;
			

			/*for (distanceX = minDistanceX; distanceX < maxDistanceX; distanceX += stepX) {
				for (int y = 0; y < Config::cameraHeight; y++) {
					screenCoords = vision->getScreenCoords(vision->getDir(), distanceX, distanceY);
				}
			}*/

			counter++;
		}

		int x = Config::cameraWidth / 2 - 15;

		distorted = vision->getCameraTranslator()->DEPRECATEDgetCameraPosition(0, distanceY);

		//if (lastTextY == -1 || lastTextY - distorted.y >= 8) {
			canvas.drawText(distorted.x, distorted.y, Util::toString(distanceY) + "m", 0, 0, 0);

			lastTextY = distorted.y;
		//}
	}

	// draw vertical dots at each 10x increment
	for (distanceX = minDistanceX; distanceX <= maxDistanceX; distanceX += stepX * 10.0f) {
		for (distanceY = 0.0f; distanceY < maxDistanceY; distanceY += stepX) {
			pos = vision->getCameraTranslator()->DEPRECATEDgetCameraPosition(distanceX, distanceY);

			canvas.setPixelAt(pos.x, pos.y, 0, 0, 128);
		}
	}
}

void DebugRenderer::renderMapping(unsigned char* image, Vision* vision, int width, int height) {
	Canvas canvas = Canvas();

	canvas.data = image;
	canvas.width = width;
	canvas.height = height;

	CameraTranslator* translator = vision->getCameraTranslator();

	int step = 5;

	int x, y;
	Pixel pos;

	for (x = 0; x < Config::cameraWidth; x += step) {
		for (y = 0; y < Config::cameraHeight; y += step) {
			pos = translator->undistort(Pixel(x, y));
			//pos = translator->distort(x, y);

			canvas.setPixelAt(pos.x, pos.y, 0, 0, 0);
		}
	}
}