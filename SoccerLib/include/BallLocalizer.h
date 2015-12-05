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
    void getBallsGoingToGoal(BallManager::BallList & result, Geometry::LineSegment & goalLine) const
    {
        auto fastEnough = [this, &goalLine](BallManager::Ball* ball) -> bool {
            return ball->velocity.getLength() > 0.5f;
        };
        
        auto hasVelocityVectorTowardsGoal = [this, &goalLine](BallManager::Ball* ball) -> bool {
            auto robotPosition = this->pRobotLocalizer->getPosition();
            Math::Vector ballWorldLocation = ball->location.getRotated(robotPosition.orientation) + robotPosition.location;
            Math::Vector ballWorldVelocity = ball->velocity.getRotated(robotPosition.orientation);
            Geometry::Ray ray(ballWorldLocation, ballWorldVelocity);
            Math::Vector result;
            bool intersects = ray.intersection(result, goalLine);
            return intersects;
        };

        BallManager::BallList ballsFastEnough;
        filterBalls(ballsFastEnough, pBallManager->getBalls(), fastEnough);
        filterBalls(result, ballsFastEnough, hasVelocityVectorTowardsGoal);

    }

    static void filterBalls(BallManager::BallList & filteredBalls, const BallManager::BallList & balls, std::function<bool(BallManager::Ball * ball)> predicate) {
        std::copy_if(balls.begin(), balls.end(), std::back_inserter(filteredBalls), predicate);
    }

    Geometry::LineSegment mYellowGoalLine;
    Geometry::LineSegment mBlueGoalLine;
    BallManager * pBallManager;
    Localizer * pRobotLocalizer;
    
};
#endif