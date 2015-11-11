#include "Object.h"
#include "Util.h"
#include "Maths.h"
#include "Config.h"

Object::Object(int x, int y, int width, int height, int area, float distance, float distanceX, float distanceY, float angle, int type, bool behind) : x(x), y(y), width(width), height(height), area(area), distance(distance), distanceX(distanceX), distanceY(distanceY), angle(angle), type(type), behind(behind), processed(false) {
	lastSeenTime = Util::millitime();
}

void Object::copyFrom(const Object* other) {
	x = other->x;
	y = other->y;
	width = other->width;
	height = other->height;
	area = other->area;
	distance = other->distance;
	distanceX = other->distanceX;
	distanceY = other->distanceY;
	angle = other->angle;
	type = other->type;
	lastSeenTime = other->lastSeenTime;
	behind = other->behind;
	processed = other->processed;
}

bool Object::intersects(Object* other, int margin) const {
	int ax1 = x - width / 2 - margin;
	int ax2 = x + width / 2 + margin;
	int ay1 = y - height / 2 - margin;
	int ay2 = y + height / 2 + margin;

	int bx1 = other->x - other->width / 2 - margin;
	int bx2 = other->x + other->width / 2 + margin;
	int by1 = other->y - other->height / 2 - margin;
	int by2 = other->y + other->height / 2 + margin;

	return !(ax2 < bx1 || ax1 > bx2 || ay2 < by1 || ay1 > by2);
}

bool Object::contains(Object* other) const {
	int ax1 = x - width / 2 ;
	int ax2 = x + width / 2;
	int ay1 = y - height / 2;
	int ay2 = y + height / 2;

	int bx1 = other->x - other->width / 2;
	int bx2 = other->x + other->width / 2;
	int by1 = other->y - other->height / 2;
	int by2 = other->y + other->height / 2;

	return bx1 >= ax1 && bx2 <= ax2 && by1 >= ay1 && by2 <= ay2;
}

Object* Object::mergeWith(Object* other) const {
	Object* merged = new Object();

	merged->copyFrom(this);

	float minX = Math::max(Math::min((float)(x - width / 2), (float)(other->x - other->width / 2)), 0.0f);
	float minY = Math::max(Math::min((float)(y - height / 2), (float)(other->y - other->height / 2)), 0.0f);
	float maxX = Math::min(Math::max((float)(x + width / 2), (float)(other->x + other->width / 2)), (float)(Config::cameraWidth - 1));
	float maxY = Math::min(Math::max((float)(y + height / 2), (float)(other->y + other->height / 2)), (float)(Config::cameraWidth - 1));
	float width = maxX - minX;
	float height = maxY - minY;

	merged->x = (int)Math::round(minX + width / 2);
	merged->y = (int)Math::round(minY + height / 2);
	merged->width = (int)Math::round(width);
	merged->height = (int)Math::round(height);
	merged->area = area + other->area;

	return merged;
}

std::vector<Object*> Object::mergeOverlapping(const std::vector<Object*>& set, int margin, bool requireSameType) {
	ObjectList stack(set);
	ObjectList individuals;
	ObjectList garbage;

	while (stack.size() > 0) {
		Object* object1 = stack.back();
		Object* mergedObject = NULL;
		stack.pop_back();

		if (object1->processed) {
			continue;
		}

		bool merged = false;

		for (ObjectListItc it = stack.begin(); it != stack.end(); it++) {
			Object* object2 = *it;

			if (object2 == object1 || object1->processed || object2->processed) {
				continue;
			}

			if (requireSameType && object1->type != object2->type) {
				continue;
			}

			if (!object1->intersects(object2, margin)) {
				continue;
			}

			mergedObject = object1->mergeWith(object2);

			if (mergedObject != NULL) {
				object1->processed = true;
				object2->processed = true;
				mergedObject->processed = false;
				merged = true;

				stack.push_back(mergedObject);
				garbage.push_back(object1);
				garbage.push_back(object2);

				break;
			}
		}

		if (!merged && !object1->processed) {
			individuals.push_back(object1);
		}
	}

	for (ObjectListItc it = garbage.begin(); it != garbage.end(); it++) {
		delete *it;
	}

	garbage.clear();

	return individuals;
}