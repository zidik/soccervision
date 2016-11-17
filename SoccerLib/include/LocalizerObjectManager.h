#pragma once

#include "Maths.h"
#include "Object.h"

class LocalizerObjectManager
{
public:
	class LocalizerObject {

	public:
		LocalizerObject(const Math::Vector & location);
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

	protected:
		static int instances;
		std::vector<Math::Vector> pastVelocities;
		int next_pastVelocity = 0;

		void applyDrag(float dt);

	};

	typedef std::vector<LocalizerObject*> LocalizerObjectList;
	typedef std::vector<LocalizerObject*>::iterator LocalizerObjectListIt;

	LocalizerObjectManager();
	~LocalizerObjectManager();

	static LocalizerObjectList extractObjects(const ObjectList& sourceObjects);
	void transformLocations(Math::Vector & dtLocation, float dtOrientation);
	void update(const LocalizerObjectList& visibleObjects, const Math::Polygon& cameraFOV, float dt);

	const LocalizerObjectList& getObjects() const { return objects; }

	const LocalizerObject* getClosestObject() const {
		auto closest = min_element(objects.begin(), objects.end(), [](LocalizerObject* b1, LocalizerObject* b2) { return b1->location < b2->location; });
		return closest == objects.end() ? nullptr : *closest;
	}


protected:
	LocalizerObjectList objects;
	LocalizerObject* getObjectAround(Math::Vector & location);
	void purge(const LocalizerObjectList& visibleObjects, const Math::Polygon& cameraFOV);
};

