#include "BallLocalizer.h"
#include "Config.h"
#include "Util.h"

#include <iostream>
#include <vector>
#include <algorithm>

BallLocalizer::BallLocalizer() {

}

BallLocalizer::~BallLocalizer() {

}

int BallLocalizer::Ball::instances = 0;

BallLocalizer::Ball::Ball(float px, float py) : location(px, py), velocity(0.0f, 0.0f) {
    id = instances++;
    createdTime = Util::millitime();
    updatedTime = createdTime;
	removeTime = -1.0;
	visible = true;
	inFOV = true;
}

void BallLocalizer::Ball::updateVisible(float newX, float newY, float dt) {
    double currentTime = Util::millitime();
    double timeSinceLastUpdate = currentTime - updatedTime;

    if (timeSinceLastUpdate <= Config::velocityUpdateMaxTime) {
		Math::Vector newVelocity = (Math::Vector(newX, newY) - location) / dt;

		if (newVelocity.getLength() <= Config::objectMaxVelocity) {
			velocity = newVelocity;
		} else {
			applyDrag(dt);
		}
    } else {
        applyDrag(dt);
    }

    location.x = newX;
    location.y = newY;
    updatedTime = currentTime;
    
    visible = true;

	removeTime = -1;
}

void BallLocalizer::Ball::updateInvisible(float dt) {
	location += velocity * dt;
    applyDrag(dt);
    visible = false;
}

void BallLocalizer::Ball::markForRemoval(double afterSeconds) {
    /*if (removeTime == -1) {
        return;
    }*/

    removeTime = Util::millitime() + afterSeconds;
}

bool BallLocalizer::Ball::shouldBeRemoved() {
    return removeTime != -1 && removeTime < Util::millitime();
}

void BallLocalizer::Ball::applyDrag(float dt) {
	Math::Vector drag_acceleration = -(velocity.getNormalized()) * Config::rollingDrag;
	if (drag_acceleration > velocity) {
		velocity = Math::Vector(0, 0);
	}
	else{
		velocity += drag_acceleration;
	}
}

BallLocalizer::BallList BallLocalizer::extractBalls(const ObjectList& sourceBalls, float robotX, float robotY, float robotOrientation) {
	BallList balls;
	Object* screenBall;
	Ball* worldBall;

	for (ObjectListItc it = sourceBalls.begin(); it != sourceBalls.end(); it++) {
		screenBall = *it;

		float globalAngle = Math::floatModulus(robotOrientation + screenBall->angle, Math::TWO_PI);
        float ballX = robotX + Math::cos(globalAngle) * screenBall->distance;
        float ballY = robotY + Math::sin(globalAngle) * screenBall->distance;

		worldBall = new Ball(ballX, ballY);

		balls.push_back(worldBall);
	}

	return balls;
}

void BallLocalizer::update(const BallList& visibleBalls, const Math::Polygon& cameraFOV, float dt) {
    Ball* closestBall;
    std::vector<int> handledBalls;
    //float globalAngle;

    for (unsigned int i = 0; i < visibleBalls.size(); i++) {
        /*globalAngle = Math::floatModulus(robotPosition.orientation + visibleBalls[i]->angle, Math::TWO_PI);

        visibleBalls[i]->x = robotPosition.x + Math::cos(globalAngle) * visibleBalls[i]->distance;
        visibleBalls[i]->y = robotPosition.y + Math::sin(globalAngle) * visibleBalls[i]->distance;*/

        closestBall = getBallAround(visibleBalls[i]->location.x, visibleBalls[i]->location.y);

        if (closestBall != NULL) {
            closestBall->updateVisible(visibleBalls[i]->location.x, visibleBalls[i]->location.y, dt);

            handledBalls.push_back(closestBall->id);
        } else {
            Ball* newBall = new Ball(visibleBalls[i]->location.x, visibleBalls[i]->location.y);

            balls.push_back(newBall);

            handledBalls.push_back(newBall->id);
        }
    }

    for (unsigned int i = 0; i < balls.size(); i++) {
        if (std::find(handledBalls.begin(), handledBalls.end(), balls[i]->id) != handledBalls.end()) {
            continue;
        }

        balls[i]->updateInvisible(dt);
    }

    purge(visibleBalls, cameraFOV);
}

BallLocalizer::Ball* BallLocalizer::getBallAround(float x, float y) {
	Math::Vector target(x, y);
    float distance;
    float minDistance = -1;
    Ball* ball;
    Ball* closestBall = NULL;

    for (unsigned int i = 0; i < balls.size(); i++) {
        ball = balls[i];

		distance = ball->location.distanceTo(target);

        if (
            distance <= Config::objectIdentityDistanceThreshold
            && (
                minDistance == -1
                || distance < minDistance
            )
        ) {
            minDistance = distance;
            closestBall = ball;
        }
    }

    return closestBall;
}

void BallLocalizer::purge(const BallList& visibleBalls, const Math::Polygon& cameraFOV) {
	double currentTime = Util::millitime();
    BallList remainingBalls;
    Ball* ball;
	bool keep;

    for (unsigned int i = 0; i < balls.size(); i++) {
        ball = balls[i];
		keep = true;

		if (currentTime - ball->updatedTime > Config::objectPurgeLifetime) {
			//std::cout << "@ LIFETIME" << std::endl;

			keep = false;
		}


		if (ball->velocity.getLength() > Config::objectMaxVelocity) {
			std::cout << "@ VELOCITY" << std::endl;

			keep = false;
		}

		if (cameraFOV.containsPoint(ball->location.x, ball->location.y)) {
			ball->inFOV = true;

			bool ballNear = false;

			for (unsigned int i = 0; i < visibleBalls.size(); i++) {
				float distance = ball->location.distanceTo(visibleBalls[i]->location);

				if (distance <= Config::objectFovCloseEnough) {
					ballNear = true;

					break;
				}
			}

			if (!ballNear)  {
				//std::cout << "@ NO BALL NEAR" << std::endl;

				keep = false;
			}
		} else {
			ball->inFOV = false;	
		}

		if (keep) {
			remainingBalls.push_back(ball);
		} else {
			delete ball;
            ball = NULL;
		}

        /*if (!ball->shouldBeRemoved()) {
            remainingBalls.push_back(ball);

            if (!isValid(ball, visibleBalls, cameraFOV)) {
                ball->markForRemoval(Config::objectMarkForRemovalThreshold);
            }
        } else {
            delete ball;
            ball = NULL;
        }*/
    }

    balls = remainingBalls;
}

/*bool BallLocalizer::isValid(Ball* ball, const BallList& visibleBalls, const Math::Polygon& cameraFOV) {
    double currentTime = Util::millitime();

    if (currentTime - ball->updatedTime > Config::objectPurgeLifetime) {
		std::cout << "@ LIFETIME" << std::endl;

		ball->resurrectable = false;

        return false;
    }

    Math::Vector velocity(ball->velocityX, ball->velocityY);

    if (velocity.getLength() > Config::objectMaxVelocity) {
		std::cout << "@ VELOCITY" << std::endl;

		ball->resurrectable = false;

        return false;
    }

    // @TODO Remove if in either goal or out of bounds..

    if (cameraFOV.containsPoint(ball->x, ball->y)) {
		ball->inFOV = true;

        bool ballNear = false;
        float distance;

        for (unsigned int i = 0; i < visibleBalls.size(); i++) {
            distance = Math::distanceBetween(ball->x, ball->y, visibleBalls[i]->x, visibleBalls[i]->y);

            if (distance <= Config::objectFovCloseEnough) {
                ballNear = true;

                break;
            }
        }

        if (!ballNear)  {
			std::cout << "@ NO BALL NEAR" << std::endl;

            return false;
        }
    } else {
		ball->inFOV = false;	
	}

    return true;
}*/
