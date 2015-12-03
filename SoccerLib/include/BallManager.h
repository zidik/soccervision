#ifndef BALLMANAGER_H
#define BALLMANAGER_H

#include "Maths.h"
#include "Object.h"
#include <functional>

class BallManager {

public:
	class Ball {

    public:
        Ball(const Math::Vector & location);
        void updateVisible(const Math::Vector & location, float dt);
        void updateInvisible(float dt);
        void markForRemoval(double afterSeconds);
        bool shouldBeRemoved() const;
        float distanceTo(const Ball & other) const { return location.distanceTo(other.location); }
        float distanceTo(const Ball * const other) const { return distanceTo(*other); }
        void transformLocation(Math::Vector & dtLocation, float dtOrientation) { location = (location-dtLocation).getRotated(-dtOrientation); }

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
        std::vector<Math::Vector> pastVelocities;
        int next_pastVelocity = 0;

        void applyDrag(float dt);

	};

	typedef std::vector<Ball*> BallList;
	typedef std::vector<Ball*>::iterator BallListIt;

    BallManager();
    ~BallManager();
    
    static BallList extractBalls(const ObjectList& sourceBalls);
    void transformLocations(Math::Vector & dtLocation, float dtOrientation);
    void update(const BallList& visibleBalls, const Math::Polygon& cameraFOV, float dt);

    const BallList& getBalls() const{ return balls; }

    const Ball* getClosestBall() const {
        auto closest = min_element(balls.begin(), balls.end(), [](Ball* b1, Ball* b2) { return b1->location < b2->location; });
        return closest == balls.end() ? nullptr : *closest;
    }
    

private:
    BallList balls;
    Ball* getBallAround(Math::Vector & location);
    void purge(const BallList& visibleBalls, const Math::Polygon& cameraFOV);
    //bool isValid(Ball* ball, const BallList& visibleBalls, const Math::Polygon& cameraFOV);
};

#endif
