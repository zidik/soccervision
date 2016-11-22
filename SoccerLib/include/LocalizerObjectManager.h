#pragma once

#include "Maths.h"
#include "Object.h"
#include "LocalizerObject.h"

class LocalizerObjectManager
{
public:

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

