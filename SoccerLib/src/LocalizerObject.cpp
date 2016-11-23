#include "stdafx.h"
#include "LocalizerObject.h"
#include "Util.h"


int LocalizerObject::instances = 0;

LocalizerObject::LocalizerObject(const Math::Vector & location, int type) : location(location), type(type), velocity(0.0f, 0.0f), pastVelocities(15) {
	id = instances++;
	createdTime = Util::millitime();
	updatedTime = createdTime;
	removeTime = -1.0;
	visible = true;
	inFOV = true;
}

void LocalizerObject::updateVisible(const Math::Vector & new_location, float dt) {
	double currentTime = Util::millitime();
	double timeSinceLastUpdate = currentTime - updatedTime;

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
		std::copy_if(pastVelocities.begin(), pastVelocities.end(), back_inserter(filteredVelocities), [&, meanPastVelocities, stdErrX, stdErrY](Math::Vector pastVelocity)
		{
			Math::Vector diff = pastVelocity - meanPastVelocities;
			bool decision =
				-stdErrX < diff.x && diff.x < stdErrX &&
				-stdErrY < diff.y && diff.y < stdErrY;
			return decision;
		});

		//Mean of filtered velocities
		if (filteredVelocities.size() > 0) {
			velocity = std::accumulate(filteredVelocities.begin(), filteredVelocities.end(), Math::Vector()) / (float)filteredVelocities.size();
		}
	}


	location = new_location;
	updatedTime = currentTime;

	visible = true;

	removeTime = -1;
}

void LocalizerObject::updateInvisible(float dt) {
	location += velocity * dt;
	visible = false;
}

void LocalizerObject::markForRemoval(double afterSeconds) {
	removeTime = Util::millitime() + afterSeconds;
}

bool LocalizerObject::shouldBeRemoved() const {
	return removeTime != -1 && removeTime < Util::millitime();
}
