#ifndef BALLMANAGER_H
#define BALLMANAGER_H

#include "Maths.h"
#include "Object.h"
#include "LocalizerObjectManager.h"
#include <functional>

class BallManager : public LocalizerObjectManager {

public:

	//typedef std::vector<Ball*> BallList;
	//typedef std::vector<Ball*>::iterator BallListIt;
    
	static LocalizerObjectList extractBalls(const ObjectList& sourceObjects) { return extractObjects(sourceObjects); }

	const LocalizerObjectList& getBalls() const { return getObjects(); }

	const LocalizerObject* getClosestBall() const { return getClosestObject(); }

};

#endif
