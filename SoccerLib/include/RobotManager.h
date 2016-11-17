#pragma once

#include "LocalizerObjectManager.h"

//TODO
//Remove drag from robotmanager
//Add Robot color to robot objects
//Think about what else has to be changed

class RobotManager : public LocalizerObjectManager
{
public:
	static LocalizerObjectList extractRobots(const ObjectList& sourceObjects) { return extractObjects(sourceObjects); }

	const LocalizerObjectList& getRobots() const { return getObjects(); }

	const LocalizerObject* getClosestRobot() const { return getClosestObject(); }
};

