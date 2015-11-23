#ifndef BALLLOCALIZER_H
#define BALLLOCALIZER_H

#include "Maths.h"
#include "Object.h"

class BallLocalizer {

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

    static BallList extractBalls(const ObjectList& sourceBalls);
    void update(const BallList& visibleBalls, const Math::Polygon& cameraFOV, float dt);

    BallList balls; //TODO: Should be private

private:
    Ball* getBallAround(Math::Vector & location);
    void purge(const BallList& visibleBalls, const Math::Polygon& cameraFOV);
    //bool isValid(Ball* ball, const BallList& visibleBalls, const Math::Polygon& cameraFOV);

	 

};

#endif // BALLLOCALIZER_H
