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

BallLocalizer::Ball::Ball(Math::Vector & location) : location(location), velocity(0.0f, 0.0f) {
    id = instances++;
    createdTime = Util::millitime();
    updatedTime = createdTime;
	removeTime = -1.0;
	visible = true;
	inFOV = true;
}

void BallLocalizer::Ball::updateVisible(Math::Vector & new_location, float dt) {
    double currentTime = Util::millitime();
    double timeSinceLastUpdate = currentTime - updatedTime;

    if (timeSinceLastUpdate <= Config::velocityUpdateMaxTime) {
		Math::Vector newVelocity = (new_location - location) / dt;
        float alpha=0.6f;
        Math::Vector smoothedVelocity = newVelocity*alpha + velocity*(1.0f - alpha);

		if (smoothedVelocity.getLength() <= Config::objectMaxVelocity) {
			velocity = smoothedVelocity;
		} else {
			applyDrag(dt);
		}
    } else {
        applyDrag(dt);
    }

    location = new_location;
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
    removeTime = Util::millitime() + afterSeconds;
}

bool BallLocalizer::Ball::shouldBeRemoved() const {
    return removeTime != -1 && removeTime < Util::millitime();
}


void BallLocalizer::Ball::applyDrag(float dt) {
	Math::Vector drag_acceleration = -velocity.getScaledTo(Config::rollingDrag);
	if (drag_acceleration > velocity) {
		velocity = Math::Vector(0, 0);
	}
	else{
		velocity += drag_acceleration;
	}
}

BallLocalizer::BallList BallLocalizer::extractBalls(const ObjectList& sourceBalls, float robotX, float robotY, float robotOrientation) const {
	BallList balls;

	for (Object* ball: sourceBalls){
		float globalAngle = Math::floatModulus(robotOrientation + ball->angle, Math::TWO_PI);
        float ballX = robotX + Math::cos(globalAngle) * ball->distance;
        float ballY = robotY + Math::sin(globalAngle) * ball->distance;
		
		Math::Vector location(ballX, ballY);
		Ball* worldBall = new Ball(location);

		balls.push_back(worldBall);
	}

	return balls;
}

void BallLocalizer::update(const BallList& visibleBalls, const Math::Polygon& cameraFOV, float dt) {
    Ball* closestBall;
    std::vector<int> handledBalls;
    //float globalAngle;

    for (auto visibleBall: visibleBalls) {
        /*globalAngle = Math::floatModulus(robotPosition.orientation + visibleBall->angle, Math::TWO_PI);

        visibleBalls[i]->x = robotPosition.x + Math::cos(globalAngle) * visibleBall->distance;
        visibleBalls[i]->y = robotPosition.y + Math::sin(globalAngle) * visibleBall->distance;*/

        closestBall = getBallAround(visibleBall->location);

        if (closestBall != nullptr) {
            closestBall->updateVisible(visibleBall->location, dt);

            handledBalls.push_back(closestBall->id);
        } else {
            Ball* newBall = new Ball(visibleBall->location);

            balls.push_back(newBall);

            handledBalls.push_back(newBall->id);
        }
    }

    for (auto ball : balls) {
        if (std::find(handledBalls.begin(), handledBalls.end(), ball->id) != handledBalls.end()) {
            continue;
        }

        ball->updateInvisible(dt);
    }

    purge(visibleBalls, cameraFOV);
}

BallLocalizer::Ball* BallLocalizer::getBallAround(Math::Vector & target) {
    float distance;
    float minDistance = -1;
    Ball* closestBall = nullptr;

    for (auto ball : balls) {
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
	bool keep;

    for(auto ball : balls){
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

            for (Ball* visibleBall : visibleBalls){
				float distance = ball->location.distanceTo(visibleBall->location);

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
            ball = nullptr;
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
