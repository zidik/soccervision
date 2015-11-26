#include "BallLocalizer.h"
#include "Config.h"
#include "Util.h"

#include <iostream>
#include <vector>
#include <functional>

BallLocalizer::BallLocalizer() {

}

BallLocalizer::~BallLocalizer() {

}

int BallLocalizer::Ball::instances = 0;

BallLocalizer::Ball::Ball(const Math::Vector & location) : location(location), velocity(0.0f, 0.0f), pastVelocities(15) {
    id = instances++;
    createdTime = Util::millitime();
    updatedTime = createdTime;
	removeTime = -1.0;
	visible = true;
	inFOV = true;
}

void BallLocalizer::Ball::updateVisible(const Math::Vector & new_location, float dt) {
    double currentTime = Util::millitime();
    double timeSinceLastUpdate = currentTime - updatedTime;

    applyDrag(dt);

    if (timeSinceLastUpdate <= Config::velocityUpdateMaxTime) {
        Math::Vector newVelocity = (new_location - location) / dt;
        pastVelocities[next_pastVelocity] = newVelocity;
        next_pastVelocity++;
        next_pastVelocity %= pastVelocities.size();

        std::vector<float> pastVelocitiesX;
        std::vector<float> pastVelocitiesY;
        pastVelocitiesX.resize(pastVelocities.size());
        pastVelocitiesY.resize(pastVelocities.size());
        std::transform(pastVelocities.begin(), pastVelocities.end(), pastVelocitiesX.begin(), [](Math::Vector v) { return v.x; });
        std::transform(pastVelocities.begin(), pastVelocities.end(), pastVelocitiesY.begin(), [](Math::Vector v) { return v.y; });
        
        Math::Vector meanPastVelocities;
        float stdErrX = Math::standardDeviation(pastVelocitiesX, meanPastVelocities.x);
        float stdErrY = Math::standardDeviation(pastVelocitiesY, meanPastVelocities.y);
        
        std::vector<Math::Vector> filteredVelocities;
        std::copy_if(pastVelocities.begin(), pastVelocities.end(), filteredVelocities.begin(), [&, meanPastVelocities, stdErrX, stdErrY](Math::Vector pastVelocity)
        {
            Math::Vector diff = pastVelocity - meanPastVelocities;
            bool decision =
                -stdErrX < diff.x && diff.x < stdErrX &&
                -stdErrY < diff.y && diff.y < stdErrY;
            return decision;
        });

        //Mean of filtered velocities
        if (filteredVelocities.size() > 0) {
            velocity = std::accumulate(filteredVelocities.begin(), filteredVelocities.end(), Math::Vector()) / filteredVelocities.size();
        }
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

BallLocalizer::BallList BallLocalizer::extractBalls(const ObjectList& sourceBalls){
	BallList balls;

	for (Object* ball: sourceBalls){
        Math::Vector location = Math::Vector::fromPolar(ball->angle, ball->distance);
		Ball* worldBall = new Ball(location);

		balls.push_back(worldBall);
	}

	return balls;
}

void BallLocalizer::transformLocations(Math::Vector& dtLocation, float dtOrientation) {
    for (auto ball : balls) {
        ball->transformLocation(dtLocation, dtOrientation);
    }
}


void BallLocalizer::update(const BallList& visibleBalls, const Math::Polygon& cameraFOV, float dt) {
    Ball* closestBall;
    std::vector<int> handledBalls;

    for (auto visibleBall: visibleBalls) {

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

        ball->inFOV = cameraFOV.containsPoint(ball->location.x, ball->location.y);
		if (ball->inFOV) {
			bool ballNear = false;

            for (Ball* visibleBall : visibleBalls){
				if (ball->distanceTo(*visibleBall) <= Config::objectFovCloseEnough) {
					ballNear = true;

					break;
				}
			}

			if (!ballNear)  {
				//std::cout << "@ NO BALL NEAR" << std::endl;

				keep = false;
			}
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
