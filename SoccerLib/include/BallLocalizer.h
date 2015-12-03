#ifndef BALL_LOCALIZER_H
#define BALL_LOCALIZER_H

#include "LineSegment.h"
#include "BallManager.h"
#include "Ray.h"
#include "Localizer.h"

class BallLocalizer{
public:
    BallLocalizer(BallManager * pBallManager, Localizer * pRobotLocalizer): pBallManager(pBallManager), pRobotLocalizer(pRobotLocalizer) {
        float fieldWidth = 4.5f;
        float fieldHeight = 3.0f;
        float goalWidth = 0.7f;

        Math::Vector yellowCorner1(0.0f - 0.05f, fieldHeight / 2.0f + goalWidth / 2.0f);
        Math::Vector yellowCorner2(0.0f - 0.05f, fieldHeight / 2.0f - goalWidth / 2.0f);
        Math::Vector blueCorner1(fieldWidth + 0.05f, fieldHeight / 2.0f - goalWidth / 2.0f);
        Math::Vector blueCorner2(fieldWidth + 0.05f, fieldHeight / 2.0f + goalWidth / 2.0f);

        mYellowGoalLine = Geometry::LineSegment(yellowCorner1, yellowCorner2);
        mBlueGoalLine = Geometry::LineSegment(blueCorner1, blueCorner2);
        
    }
    
    void getBallsGoingToBlueGoal(BallManager::BallList & balls) { getBallsGoingToGoal(balls, mBlueGoalLine); }
    void getBallsGoingToYellowGoal(BallManager::BallList & balls) { getBallsGoingToGoal(balls, mYellowGoalLine); }

private:
    void getBallsGoingToGoal(BallManager::BallList & balls, Geometry::LineSegment & goalLine) const
    {
        auto predicate = [this, &goalLine](BallManager::Ball* ball) {
            auto robotPosition = this->pRobotLocalizer->getPosition();
            Math::Vector ballWorldLocation = ball->location.getRotated(robotPosition.orientation) + robotPosition.location;
            Math::Vector ballWorldVelocity = ball->velocity.getRotated(robotPosition.orientation);
            Geometry::Ray ray(ballWorldLocation, ballWorldVelocity);
            Math::Vector result;
            bool intersects = ray.intersection(result, goalLine);
            return intersects;
        };
        pBallManager->getfilteredBalls(predicate);
    }

    Geometry::LineSegment mYellowGoalLine;
    Geometry::LineSegment mBlueGoalLine;
    BallManager * pBallManager;
    Localizer * pRobotLocalizer;
    
};
#endif