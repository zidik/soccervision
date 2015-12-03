#ifndef BALL_LOCALIZER_H
#define BALL_LOCALIZER_H

#include "LineSegment.h"
#include "BallManager.h"
#include "Ray.h"

class BallLocalizer{
public:
    BallLocalizer(BallManager * pBallManager): pBallManager(pBallManager) {
        float fieldWidth = 4.5f;
        float fieldHeight = 3.0f;
        float goalWidth = 0.7f;

        Math::Vector yellowCorner1(0.0f - 0.05f, fieldHeight / 2.0f + goalWidth / 2.0f);
        Math::Vector yellowCorner2(0.0f - 0.05f, fieldHeight / 2.0f - goalWidth / 2.0f);
        Math::Vector blueCorner1(fieldWidth + 0.05f, fieldHeight / 2.0f - goalWidth / 2.0f);
        Math::Vector blueCorner2(fieldWidth + 0.05f, fieldHeight / 2.0f + goalWidth / 2.0f);

        yellowGoalLine = Geometry::LineSegment(yellowCorner1, yellowCorner2);
        blueGoalLine = Geometry::LineSegment(blueCorner1, blueCorner2);
        
    }
    
    void getBallsGoingToBlueGoal(BallManager::BallList & balls) { getBallsGoingToGoal(balls, blueGoalLine); }
    void getBallsGoingToYellowGoal(BallManager::BallList & balls) { getBallsGoingToGoal(balls, yellowGoalLine); }

private:
    void getBallsGoingToGoal(BallManager::BallList & balls, Geometry::LineSegment & goalLine) const
    {
        auto predicate = [&goalLine](BallManager::Ball* ball) {
            Geometry::Ray ray(ball->location, ball->velocity);
            Math::Vector result;
            bool intersects = ray.intersection(result, goalLine);
            return intersects;
        };
        pBallManager->getfilteredBalls(predicate);
    }

    Geometry::LineSegment yellowGoalLine;
    Geometry::LineSegment blueGoalLine;
    BallManager * pBallManager;
    
};
#endif