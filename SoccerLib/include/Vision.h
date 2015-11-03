#ifndef VISION_H
#define VISION_H

#include "Blobber.h"
#include "Canvas.h"
#include "Object.h"
#include "LookupTable.h"
#include "Config.h"
#include "Maths.h"
#include "CameraTranslator.h"

#include <string>
#include <vector>

class Vision {

public:
	typedef std::vector<std::string> ColorList;

	struct PathMetric {
		PathMetric(float percentage, int invalidSpree, bool validColorFound, bool out, int invalidColorCount) : percentage(percentage), invalidSpree(invalidSpree), validColorFound(validColorFound), out(out), invalidColorCount(invalidColorCount) {}

		float percentage;
		int invalidSpree;
		bool validColorFound;
		bool out;
		int invalidColorCount;
	};

	struct ColorDistance {
		ColorDistance() : left(0.0f), leftMiddle(0.0f), center(0.0f), rightMiddle(0.0f), right(0.0f), min(0.0f), max(0.0f) {}
		ColorDistance(float left, float leftMiddle, float center, float rightMiddle, float right) : left(left), leftMiddle(leftMiddle), center(center), rightMiddle(rightMiddle), right(right) {
			min = -1.0f;
			max = -1.0f;

			if (left != -1.0f && (min == -1.0f || left < min)) min = left;
			if (leftMiddle != -1.0f && (min == -1.0f || leftMiddle < min)) min = leftMiddle;
			if (center != -1.0f && (min == -1.0f || center < min)) min = center;
			if (rightMiddle != -1.0f && (min == -1.0f || rightMiddle < min)) min = rightMiddle;
			if (right != -1.0f && (min == -1.0f || right < min)) min = right;

			if (left != -1.0f && (max == -1.0f || left > max)) max = left;
			if (leftMiddle != -1.0f && (max == -1.0f || leftMiddle > max)) max = leftMiddle;
			if (center != -1.0f && (max == -1.0f || center > max)) max = center;
			if (rightMiddle != -1.0f && (max == -1.0f || rightMiddle > max)) max = rightMiddle;
			if (right != -1.0f && (max == -1.0f || right > max)) max = right;
		}

		float left;
		float leftMiddle;
		float center;
		float rightMiddle;
		float right;
		float min;
		float max;
	};

	struct BallInWayMetric {
		BallInWayMetric() : isBallInWay(false), ballInWayCount(0), closestBallInWayDistance(0.0f), furthestBallInWayDistance(0.0f) {}
		BallInWayMetric(bool isBallInWay, int ballInWayCount, float closestBallInWayDistance, float furthestBallInWayDistance) : isBallInWay(isBallInWay), ballInWayCount(ballInWayCount), closestBallInWayDistance(closestBallInWayDistance), furthestBallInWayDistance(furthestBallInWayDistance) {}

		bool isBallInWay;
		int ballInWayCount;
		float closestBallInWayDistance;
		float furthestBallInWayDistance;
	};

	struct Obstruction {
		Obstruction() : left(false), right(false), invalidCountLeft(0), invalidCountRight(0) {}
		Obstruction(bool left, bool right, int invalidCountLeft, int invalidCountRight) : left(left), right(right), invalidCountLeft(invalidCountLeft), invalidCountRight(invalidCountRight) {}

		bool left;
		bool right;
		int invalidCountLeft;
		int invalidCountRight;
	};

	struct Result {
		Result() : vision(NULL) {}

		ObjectList balls;
		ObjectList robots;
		ObjectList goals;
		ColorList colorOrder;
		ColorDistance whiteDistance;
		ColorDistance blackDistance;
		Vision* vision;
	};

	class Results {
		public:
			Results() : front(NULL), rear(NULL), goalPathObstruction() {}
			Object* getClosestBall(Dir dir = Dir::ANY, bool nextClosest = false, bool preferLeft = false, bool preferRear = false, bool preferFront = false);
			Object* getFurthestBall(Dir dir = Dir::ANY);
			Object* getNextClosestBall(Dir dir = Dir::ANY);
			Object* getLargestGoal(Side side, Dir dir = Dir::ANY);
			Object* getFurthestGoal(Dir dir = Dir::ANY);
			bool isBallInGoal(Object* ball);
			bool isBallInGoal(Object* ball, Object* blueGoal, Object* yellowGoal);
			BallInWayMetric getBallInWayMetric(ObjectList balls, int goalY, Object* ignoreBall = NULL);
			//bool isBallInWay(ObjectList balls, int goalY);
			bool isRobotOut(Dir dir = Dir::ANY);
			int getVisibleBallCount();
			Obstruction goalPathObstruction;

			Vision::Result* front;
			Vision::Result* rear;
	};

	struct Distance {
		Distance(float x = 0.0f, float y = 0.0f, float straight = 0.0f, float angle = 0.0f) : x(x), y(y), straight(straight), angle(angle) {}

		float x;
		float y;
		float straight;
		float angle;
	};

	struct EdgeDistance {
		EdgeDistance(int screenX = -1, int screenY = -1, float distance = -1) : screenX(screenX), screenY(screenY), distance(distance) {}

		int screenX;
		int screenY;
		float distance;
	};

	struct EdgeDistanceMetric {
		EdgeDistanceMetric(Vision::EdgeDistance leftTopDistance, Vision::EdgeDistance rightTopDistance, Vision::EdgeDistance centerDistance, int newX = -1, int newWidth = -1) : leftTopDistance(leftTopDistance), rightTopDistance(rightTopDistance), centerDistance(centerDistance), newX(newX), newWidth(newWidth) {}

		Vision::EdgeDistance leftTopDistance;
		Vision::EdgeDistance rightTopDistance;
		Vision::EdgeDistance centerDistance;
		int newX;
		int newWidth;
	};

    Vision(Blobber* blobber, CameraTranslator* cameraTranslator, Dir dir, int width, int height);
    ~Vision();

	void setDebugImage(unsigned char* image, int width, int height);
    Result* process();
    Blobber::Color* getColorAt(int x, int y);
	CameraTranslator* getCameraTranslator() { return cameraTranslator; }
	Dir getDir() { return dir; }
    Distance getDistance(int x, int y);
	//float getHorizontalDistance(Dir dir, int x, int y);
    float getAngle(int x, int y);
	//int getPixelRowAt(float distance);
	Pixel getPixelAt(float distanceX, float distanceY);
	Math::Vector getScreenCoords(float distanceX, float distanceY);
	Obstruction getGoalPathObstruction(float goalDistance);

private:
	std::pair<ObjectList, ObjectList> processGoalsAndRobots(Dir dir);
	ObjectList processBalls(Dir dir, ObjectList& goals);
	float getSurroundMetric(int x, int y, int radius, std::vector<std::string> validColors, std::string requiredColor = "", int side = 0, bool allowNone = false);
    PathMetric getPathMetric(int x1, int y1, int x2, int y2, std::vector<std::string> validColors, std::string requiredColor = "");
	EdgeDistanceMetric getEdgeDistanceMetric(int x, int y, int width, int height, std::string color1, std::string color2);
	float getBlockMetric(int x, int y, int width, int height, std::vector<std::string> validColors, int step = 6);
	float getUndersideMetric(int x, int y, float distance, int width, int height, std::string targetColor, std::string targetColor2, std::vector<std::string> validColors, bool expand = true);
	float getUndersideMetric(int x, int y, float distance, int width, int height, std::string targetColor, std::string targetColor2, std::vector<std::string> validColors, int& minValidX, int& minValidY, int& maxValidX, int& maxValidY, bool expand = true);
	float getColorDistance(std::string colorName, int x1, int y1, int x2, int y2);
	ColorDistance getColorDistance(std::string colorName);
	ColorList getViewColorOrder();
	Object* mergeGoals(Object* goal1, Object* goal2);
	bool findRobotBlobs(Dir dir, ObjectList* blobs, ObjectList* robots);
	ObjectList mergeRobotBlobs(Dir dir, ObjectList blobs);
	float getColorMatchRatio(std::vector<std::pair<int, int>>* scanPoints, std::string colorName);
	Distance getRobotDistance(int x, int y, int iterations, Dir dir);
	bool isValidBall(Object* ball, Dir dir, ObjectList& goals);
    bool isValidGoal(Object* goal, Side side);
	bool isValidRobot(Object* robot);
	bool isBallInGoal(Object* ball, Dir dir, ObjectList& goals);
	int getBallRadius(int width, int height);
	int getBallSenseRadius(int ballRadius, float distance);
	int getPixelsBelow(int x, int y, std::vector<std::string> validColors, int allowedWrongPixels = 3);
	/*int getBallMaxInvalidSpree(int y);
	int getGoalMaxInvalidSpree(int y);*/
	void updateColorDistances();
	void updateColorOrder();

	Dir dir;
	Canvas canvas;
    Blobber* blobber;
	CameraTranslator* cameraTranslator;
    std::vector<std::string> validBallBgColors;
    std::vector<std::string> validBallPathColors;
    std::vector<std::string> validGoalPathColors;
    std::vector<std::string> validColorsBelowBall;
    std::vector<std::string> viewObstructedValidColors;
    std::vector<std::string> goalObstructedValidColors;
    std::vector<std::string> goalColors;
    int width;
    int height;
	ColorList colorOrder;
	ColorDistance whiteDistance;
	ColorDistance blackDistance;

};

#endif // VISION_H
