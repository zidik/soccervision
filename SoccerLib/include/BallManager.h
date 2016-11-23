#ifndef BALLMANAGER_H
#define BALLMANAGER_H

#include "Maths.h"
#include "Object.h"
#include "LocalizerObjectManager.h"
#include <functional>

class BallManager : public LocalizerObjectManager {

public:
	class Ball : public LocalizerObject
	{
	public:
		Ball(const Math::Vector& location) : LocalizerObject(location, RobotColor::ORANGE){}

		void updateVisible(const Math::Vector & location, float dt);
		void updateInvisible(float dt);
	private:
		void applyDrag(float dt);
	};

	typedef std::vector<LocalizerObject*> BallList;
	typedef std::vector<LocalizerObject*>::iterator BallListIt;
    
	static LocalizerObjectList extractBalls(const ObjectList& sourceObjects) { return extractObjects(sourceObjects); }

	const LocalizerObjectList& getBalls() const { return getObjects(); }

	const LocalizerObject* getClosestBall() const { return getClosestObject(); }

};

#endif
