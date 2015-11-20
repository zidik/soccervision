#ifndef BALLLOCALIZER_H
#define BALLLOCALIZER_H

#include "Maths.h"
#include "Object.h"

class BallLocalizer {

public:
	class Ball {

    public:
        Ball(float x, float y);
        void updateVisible(float x, float y, float dt);
        void updateInvisible(float dt);
        void markForRemoval(double afterSeconds);
        bool shouldBeRemoved();

        int id;
        double createdTime;
        double updatedTime;
        double removeTime;
		Math::Vector location;
		Math::Vector velocity;
        bool visible;
		bool inFOV;

    private:
        static int instances;

        void applyDrag(float dt);

	};

	typedef std::vector<Ball*> BallList;
	typedef std::vector<Ball*>::iterator BallListIt;

    BallLocalizer();
    ~BallLocalizer();

	BallList extractBalls(const ObjectList& sourceBalls, float robotX, float robotY, float robotOrientation);
    void update(const BallList& visibleBalls, const Math::Polygon& cameraFOV, float dt);
    Ball* getBallAround(float x, float y);
    void purge(const BallList& visibleBalls, const Math::Polygon& cameraFOV);
    //bool isValid(Ball* ball, const BallList& visibleBalls, const Math::Polygon& cameraFOV);

	BallList balls; 

};

#endif // BALLLOCALIZER_H
