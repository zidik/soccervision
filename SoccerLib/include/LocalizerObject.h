#pragma once

#include "Maths.h"

class LocalizerObject
{
public:
	LocalizerObject(const Math::Vector & location, int type);
	void updateVisible(const Math::Vector & location, float dt);
	void updateInvisible(float dt);
	void markForRemoval(double afterSeconds);
	bool shouldBeRemoved() const;
	float distanceTo(const LocalizerObject & other) const { return location.distanceTo(other.location); }
	float distanceTo(const LocalizerObject * const other) const { return distanceTo(*other); }
	void transformLocation(Math::Vector & dtLocation, float dtOrientation) { location = (location - dtLocation).getRotated(-dtOrientation); }

	int id;
	double createdTime;
	double updatedTime;
	double removeTime;
	Math::Vector location;
	Math::Vector velocity;
	bool visible;
	bool inFOV;
	int type;

protected:
	static int instances;
	std::vector<Math::Vector> pastVelocities;
	int next_pastVelocity = 0;
};

