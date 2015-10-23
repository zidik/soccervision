#include "Odometer.h"
#include "Maths.h"
#include "Util.h"

#include <string>
#include <sstream>

Odometer::Odometer(float angle1, float angle2, float angle3, float angle4, float wheelOffset, float wheelRadius) : wheelOffset(wheelOffset), wheelRadius(wheelRadius) {
	wheelAngles[0] = Math::degToRad(angle1);
    wheelAngles[1] = Math::degToRad(angle2);
    wheelAngles[2] = Math::degToRad(angle3);
    wheelAngles[3] = Math::degToRad(angle4);

    wheelRadiusInv = 1.0f / wheelRadius;

	omegaMatrix = Math::Matrix4x3(
        -Math::sin(wheelAngles[0]), Math::cos(wheelAngles[0]), wheelOffset,
		-Math::sin(wheelAngles[1]), Math::cos(wheelAngles[1]), wheelOffset,
		-Math::sin(wheelAngles[2]), Math::cos(wheelAngles[2]), wheelOffset,
		-Math::sin(wheelAngles[3]), Math::cos(wheelAngles[3]), wheelOffset
    );
    omegaMatrixInvA = Math::Matrix3x3(
        -Math::sin(wheelAngles[0]), Math::cos(wheelAngles[0]), wheelOffset,
		-Math::sin(wheelAngles[1]), Math::cos(wheelAngles[1]), wheelOffset,
		-Math::sin(wheelAngles[2]), Math::cos(wheelAngles[2]), wheelOffset
    ).getInversed();
    omegaMatrixInvB = Math::Matrix3x3(
        -Math::sin(wheelAngles[0]), Math::cos(wheelAngles[0]), wheelOffset,
		-Math::sin(wheelAngles[1]), Math::cos(wheelAngles[1]), wheelOffset,
		-Math::sin(wheelAngles[3]), Math::cos(wheelAngles[3]), wheelOffset
    ).getInversed();
    omegaMatrixInvC = Math::Matrix3x3(
        -Math::sin(wheelAngles[0]), Math::cos(wheelAngles[0]), wheelOffset,
		-Math::sin(wheelAngles[2]), Math::cos(wheelAngles[2]), wheelOffset,
		-Math::sin(wheelAngles[3]), Math::cos(wheelAngles[3]), wheelOffset
    ).getInversed();
    omegaMatrixInvD = Math::Matrix3x3(
		-Math::sin(wheelAngles[1]), Math::cos(wheelAngles[1]), wheelOffset,
		-Math::sin(wheelAngles[2]), Math::cos(wheelAngles[2]), wheelOffset,
		-Math::sin(wheelAngles[3]), Math::cos(wheelAngles[3]), wheelOffset
    ).getInversed();
}

Odometer::WheelSpeeds Odometer::calculateWheelSpeeds(float targetDirX, float targetDirY, float targetOmega) {
	Math::Matrix3x1 targetMatrix(
        targetDirX,
        targetDirY,
        targetOmega
    );

    Math::Matrix4x1 resultMatrix = omegaMatrix
        .getMultiplied(wheelRadiusInv)
        .getMultiplied(targetMatrix);

	return WheelSpeeds(
		resultMatrix.a21,
		resultMatrix.a31,
		resultMatrix.a11,
		resultMatrix.a41
	);
}

Odometer::Movement Odometer::calculateMovement(float omegaFL, float omegaFR, float omegaRL, float omegaRR) {
	Math::Matrix3x1 wheelMatrixA = Math::Matrix3x1(
        omegaRL,
        omegaFL,
        omegaFR
    );
    Math::Matrix3x1 wheelMatrixB = Math::Matrix3x1(
        omegaRL,
        omegaFL,
        omegaRR
    );
    Math::Matrix3x1 wheelMatrixC = Math::Matrix3x1(
        omegaRL,
        omegaFR,
        omegaRR
    );
    Math::Matrix3x1 wheelMatrixD = Math::Matrix3x1(
        omegaFL,
        omegaFR,
        omegaRR
    );

    Math::Matrix3x1 movementA = omegaMatrixInvA.getMultiplied(wheelMatrixA).getMultiplied(wheelRadius);
    Math::Matrix3x1 movementB = omegaMatrixInvB.getMultiplied(wheelMatrixB).getMultiplied(wheelRadius);
    Math::Matrix3x1 movementC = omegaMatrixInvC.getMultiplied(wheelMatrixC).getMultiplied(wheelRadius);
    Math::Matrix3x1 movementD = omegaMatrixInvD.getMultiplied(wheelMatrixD).getMultiplied(wheelRadius);

    float avgVelocityX = -(movementA.a11 + movementB.a11 + movementC.a11 + movementD.a11) / 4.0f;
    float avgVelocityY = -(movementA.a21 + movementB.a21 + movementC.a21 + movementD.a21) / 4.0f;
    float avgOmega = -(movementA.a31 + movementB.a31 + movementC.a31 + movementD.a31) / 4.0f;

    float avgDiffA = Math::abs(movementA.a11 - avgVelocityX) + Math::abs(movementA.a21 - avgVelocityY) + Math::abs(movementA.a31 - avgOmega);
    float avgDiffB = Math::abs(movementB.a11 - avgVelocityX) + Math::abs(movementB.a21 - avgVelocityY) + Math::abs(movementB.a31 - avgOmega);
    float avgDiffC = Math::abs(movementC.a11 - avgVelocityX) + Math::abs(movementC.a21 - avgVelocityY) + Math::abs(movementC.a31 - avgOmega);
    float avgDiffD = Math::abs(movementD.a11 - avgVelocityX) + Math::abs(movementD.a21 - avgVelocityY) + Math::abs(movementD.a31 - avgOmega);

    float diffs[] = {avgDiffA, avgDiffB, avgDiffC, avgDiffD};
    float largestDiff = 0;
    int largestDiffIndex = -1;

    for (int i = 0; i < 4; i++) {
        if (diffs[i] > largestDiff) {
            largestDiff = diffs[i];
            largestDiffIndex = i;
        }
    }

    if (largestDiffIndex != -1) {
        switch (largestDiffIndex) {
            case 0:
                avgVelocityX = (movementB.a11 + movementC.a11 + movementD.a11) / 3.0f;
                avgVelocityY = (movementB.a21 + movementC.a21 + movementD.a21) / 3.0f;
                avgOmega = (movementB.a31 + movementC.a31 + movementD.a31) / 3.0f;
            break;

            case 1:
                avgVelocityX = (movementA.a11 + movementC.a11 + movementD.a11) / 3.0f;
                avgVelocityY = (movementA.a21 + movementC.a21 + movementD.a21) / 3.0f;
                avgOmega = (movementA.a31 + movementC.a31 + movementD.a31) / 3.0f;
            break;

            case 2:
                avgVelocityX = (movementA.a11 + movementB.a11 + movementD.a11) / 3.0f;
                avgVelocityY = (movementA.a21 + movementB.a21 + movementD.a21) / 3.0f;
                avgOmega = (movementA.a31 + movementB.a31 + movementD.a31) / 3.0f;
            break;

            case 3:
                avgVelocityX = (movementA.a11 + movementB.a11 + movementC.a11) / 3.0f;
                avgVelocityY = (movementA.a21 + movementB.a21 + movementC.a21) / 3.0f;
                avgOmega = (movementA.a31 + movementB.a31 + movementC.a31) / 3.0f;
            break;
        }
    }

	//Math::Vector velocityVector(movement.velocityX, movement.velocityY);
	//velocity = velocityVector.getLength() / dt;

	return Movement(
		avgVelocityX,
		avgVelocityY,
		avgOmega
	);
}