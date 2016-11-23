#include "BallManager.h"
#include "Config.h"
#include "Util.h"

#include <iostream>
#include <vector>
#include <functional>


void BallManager::Ball::applyDrag(float dt) {
	Math::Vector drag_acceleration = -velocity.getScaledTo(Config::rollingDrag);
	if (drag_acceleration > velocity) {
		velocity = Math::Vector(0, 0);
	}
	else {
		velocity += drag_acceleration;
	}
}

void BallManager::Ball::updateVisible(const Math::Vector& location, float dt)
{
	applyDrag(dt);
	LocalizerObject::updateVisible(location, dt);
}


void BallManager::Ball::updateInvisible(float dt)
{
	LocalizerObject::updateInvisible(dt);
	applyDrag(dt);
}
