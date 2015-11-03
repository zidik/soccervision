#ifndef ODOMETER_H
#define ODOMETER_H

#include "Maths.h"

class Odometer {

public:
	struct Movement {
        Movement(float velocityX, float velocityY, float omega) : velocityX(velocityX), velocityY(velocityY), omega(omega) {}
		Movement() : velocityX(0), velocityY(0), omega(0) {}

        float velocityX;
        float velocityY;
        float omega;
    };

	struct WheelSpeeds {
		WheelSpeeds(float FL, float FR, float RL, float RR) : FL(FL), FR(FR), RL(RL), RR(RR) {}
		WheelSpeeds() : FL(0.0f), FR(0.0f), RL(0.0f), RR(0.0f) {} 

		float FL;
		float FR;
		float RL;
		float RR;
	};

    Odometer(float angle1, float angle2, float angle3, float angle4, float wheelOffset, float wheelRadius);

	WheelSpeeds calculateWheelSpeeds(float targetDirX, float targetDirY, float targetOmega);
	Movement calculateMovement(float omegaFL, float omegaFR, float omegaRL, float omegaRR);

	float wheelOffset;
    float wheelRadius;
    float wheelRadiusInv;
    float wheelAngles[4];

private:
	Math::Matrix4x3 omegaMatrix;
    Math::Matrix3x3 omegaMatrixInvA;
    Math::Matrix3x3 omegaMatrixInvB;
    Math::Matrix3x3 omegaMatrixInvC;
    Math::Matrix3x3 omegaMatrixInvD;
};

#endif // ODOMETER_H
