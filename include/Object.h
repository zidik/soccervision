#ifndef OBJECT_H
#define OBJECT_H

#include "Config.h"
#include "Maths.h"

#include <vector>

struct ObjectLocation {
	float locationX;
	float locationY;
	int age;
	ObjectLocation(float locationX, float locationY) {
		this->locationX = locationX;
		this->locationY = locationY;
		this->age = 0;
	}
};

typedef std::vector<ObjectLocation*> ObjectLocationList;

struct movementVector {
	float dX;
	float dY;
	float speed;	//currently used units are meters per frame i think
	float angle;
	ObjectLocationList locationBuffer;
	movementVector(float dX = 0.0f, float dY = 0.0f, ObjectLocationList locationBuffer = ObjectLocationList());
	bool addLocation(float posX, float posY);
	bool incrementLocationsAge();
	bool removeOldLocations();
	bool updateSpeedAndAngle();
};

class Object {

public:
	Object(int x = 0, int y = 0, int width = 0, int height = 0, int area = 0, float distance = 0.0f, float distanceX = 0.0f, float distanceY = 0.0f, float angle = 0.0f, movementVector relativeMovement = movementVector(0.0f, 0.0f), movementVector absoluteMovement = movementVector(0.0f, 0.0f), int type = -1, bool behind = false);
	void copyFrom(const Object* other);
	void copyWithoutMovement(const Object* other);
	bool intersects(Object* other, int margin = 0) const;
	float getDribblerDistance() { return Math::max(distance - Config::robotDribblerDistance, 0.0f); };
	bool contains(Object* other) const;
	Object* mergeWith(Object* other) const;

	static std::vector<Object*> mergeOverlapping(const std::vector<Object*>& set, int margin = 0, bool requireSameType = false);

    int x;
    int y;
    int width;
    int height;
    int area;
    float distance;
	float distanceX;
	float distanceY;
    float angle;
	movementVector relativeMovement;
	movementVector absoluteMovement;	//not properly implemented currently so don't use it
    int type;
	double lastSeenTime;
	int notSeenFrames;
	bool behind;
	bool processed;
};

enum property {
	ANGLE = 9,
};

struct EntityComp {
	int property;
	EntityComp(int property) { this->property = property; }
	bool operator()(Object* s1, Object* s2) const {
		if (property == ANGLE)
			return s1->angle < s2->angle;
		else
			return s1->area < s2->area;
	}
};

typedef std::vector<Object*> ObjectList;
typedef ObjectList::iterator ObjectListIt;
typedef ObjectList::const_iterator ObjectListItc;

#endif // OBJECT_H
