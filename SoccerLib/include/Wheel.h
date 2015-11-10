#ifndef WHEEL_H
#define WHEEL_H

#include "Command.h"

#include <string>

class Wheel : public Command::Listener {

public:
    Wheel(int id);

    virtual void setTargetOmega(float omega);
    virtual void setTargetSpeed(int speed);
	virtual float getTargetOmega() const;
	virtual float getFilteredTargetOmega() const;
    virtual float getTargetSpeed() const;
    virtual float getRealOmega() const;
	virtual bool isStalled();
    virtual void step(float dt);
	virtual bool handleCommand(const Command& cmd);

    static float omegaToSpeed(float omega);
    static float speedToOmega(float speed);

    //std::string getStateJSON() const;

protected:
    int id;
    float targetOmega;
	float filteredTargetOmega;
    float realOmega;
	int stallCounter;

    static const float pidFrequency;
    static const float ticksPerRevolution;

};

#endif // WHEEL_H
