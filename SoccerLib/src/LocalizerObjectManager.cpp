#include "stdafx.h"
#include "LocalizerObjectManager.h"
#include "Config.h"
#include "Util.h"

#include <iostream>
#include <vector>


LocalizerObjectManager::LocalizerObjectManager()
{
}


LocalizerObjectManager::~LocalizerObjectManager()
{
}

LocalizerObjectManager::LocalizerObjectList LocalizerObjectManager::extractObjects(const ObjectList& sourceObjects) {
	LocalizerObjectList objects;

	for (Object* object : sourceObjects) {
		Math::Vector location = Math::Vector::fromPolar(object->angle, object->distance);
		LocalizerObject* worldObject = new LocalizerObject(location);

		objects.push_back(worldObject);
	}

	return objects;
}

void LocalizerObjectManager::transformLocations(Math::Vector& dtLocation, float dtOrientation) {
	for (auto object : objects) {
		object->transformLocation(dtLocation, dtOrientation);
	}
}


void LocalizerObjectManager::update(const LocalizerObjectList& visibleObjects, const Math::Polygon& cameraFOV, float dt) {
	LocalizerObject* closestObject;
	std::vector<int> handledObjects;

	for (auto visibleObject : visibleObjects) {

		closestObject = getObjectAround(visibleObject->location);

		if (closestObject != nullptr) {
			closestObject->updateVisible(visibleObject->location, dt);

			handledObjects.push_back(closestObject->id);
		}
		else {
			LocalizerObject* newObject = new LocalizerObject(visibleObject->location);

			objects.push_back(newObject);

			handledObjects.push_back(newObject->id);
		}
	}

	for (auto object : objects) {
		if (std::find(handledObjects.begin(), handledObjects.end(), object->id) != handledObjects.end()) {
			continue;
		}

		object->updateInvisible(dt);
	}

	purge(visibleObjects, cameraFOV);
}

LocalizerObject* LocalizerObjectManager::getObjectAround(Math::Vector & target) {
	float distance;
	float minDistance = -1;
	LocalizerObject* closestObject = nullptr;

	for (auto object : objects) {
		distance = object->location.distanceTo(target);

		if (
			distance <= Config::objectIdentityDistanceThreshold
			&& (
				minDistance == -1
				|| distance < minDistance
				)
			) {
			minDistance = distance;
			closestObject = object;
		}
	}

	return closestObject;
}

void LocalizerObjectManager::purge(const LocalizerObjectList& visibleObjects, const Math::Polygon& cameraFOV) {
	double currentTime = Util::millitime();
	LocalizerObjectList remainingObjects;
	bool keep;

	for (auto object : objects) {
		keep = true;

		if (currentTime - object->updatedTime > Config::objectPurgeLifetime) {
			//std::cout << "@ LIFETIME" << std::endl;

			keep = false;
		}


		if (object->velocity.getLength() > Config::objectMaxVelocity) {
			std::cout << "@ VELOCITY" << std::endl;

			keep = false;
		}

		object->inFOV = cameraFOV.containsPoint(object->location.x, object->location.y);
		if (object->inFOV) {
			bool objectNear = false;

			for (LocalizerObject* visibleObject : visibleObjects) {
				if (object->distanceTo(*visibleObject) <= Config::objectFovCloseEnough) {
					objectNear = true;

					break;
				}
			}

			if (!objectNear) {
				//std::cout << "@ NO OBJECT NEAR" << std::endl;

				keep = false;
			}
		}

		if (keep) {
			remainingObjects.push_back(object);
		}
		else {
			delete object;
			object = nullptr;
		}

	}

	objects = remainingObjects;
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

