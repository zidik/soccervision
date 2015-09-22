#include "ParticleFilterLocalizer.h"
#include "Maths.h"
#include "Config.h"

#include <iostream>
#include <string>
#include <sstream>


ParticleFilterLocalizer::ParticleFilterLocalizer(
	CameraTranslator* frontCameraTranslator,
	CameraTranslator* rearCameraTranslator,
	int particleCount,
	float forwardNoise,
	float turnNoise
	) :
	frontCameraTranslator(frontCameraTranslator),
	rearCameraTranslator(rearCameraTranslator),
	particleCount(particleCount),
	forwardNoise(forwardNoise), 
	turnNoise(turnNoise) {
	generateRandomParticles(particleCount);
	json = "null";
}

ParticleFilterLocalizer::~ParticleFilterLocalizer() {
    for (ParticleList::const_iterator it = particles.begin(); it != particles.end(); it++) {
        delete *it;
    }

    particles.clear();

    for (LandmarkMap::const_iterator it = landmarks.begin(); it != landmarks.end(); it++) {
        delete it->second;
    }

    landmarks.clear();
}

void ParticleFilterLocalizer::generateRandomParticles(int particleCount)
{
	for (int i = 0; i < particleCount; i++) {
		particles.push_back(new Particle(
			Math::randomFloat(0.0f, Config::fieldWidth),
			Math::randomFloat(0.0f, Config::fieldHeight),
			Math::randomFloat(0.0f, Math::TWO_PI),
			0.0f
			));
	}
}

void ParticleFilterLocalizer::addLandmark(Landmark* landmark) {
    landmarks[landmark->name] = landmark;
}

void ParticleFilterLocalizer::addLandmark(std::string name, float x, float y) {
	addLandmark(new Landmark(name, x, y));
}

void ParticleFilterLocalizer::setPosition(float x, float y, float orientation) {
	for (unsigned int i = 0; i < particles.size(); i++) {
		particles[i]->orientation = orientation;
        particles[i]->location.x = x;
        particles[i]->location.y = y;
		particles[i]->probability = 1;
    }
}

void ParticleFilterLocalizer::move(float velocityX, float velocityY, float velocityOmega, float dt, bool exact) {
	float particleVelocityX, particleVelocityY, particleVelocityOmega;

	for (unsigned int i = 0; i < particles.size(); i++) {
		if (exact) {
			particleVelocityX = velocityX;
			particleVelocityY = velocityY;
			particleVelocityOmega = velocityOmega;
		} else {
			// TODO Add noise in FORWARD direction
			particleVelocityX = velocityX + Math::randomGaussian(forwardNoise);
			particleVelocityY = velocityY + Math::randomGaussian(forwardNoise);
			particleVelocityOmega = velocityOmega + Math::randomGaussian(turnNoise);
		}

		float particleVelocityXLocal = particleVelocityX * Math::cos(particles[i]->orientation) - particleVelocityY * Math::sin(particles[i]->orientation);
		float particleVelocityYLocal = particleVelocityX * Math::sin(particles[i]->orientation) + particleVelocityY * Math::cos(particles[i]->orientation);
		particles[i]->orientation += particleVelocityOmega * dt;
		particles[i]->location.x += particleVelocityXLocal * dt;
		particles[i]->location.y += particleVelocityYLocal * dt;
	}
}

void ParticleFilterLocalizer::update(const Measurements& measurements) {
    Particle* particle;
    float maxProbability = -1;

    for (unsigned int i = 0; i < particles.size(); i++) {
        particle = particles[i];

        particle->probability = getMeasurementProbability(particle, measurements);

        if (maxProbability == -1 || particle->probability > maxProbability) {
            maxProbability = particle->probability;
        }
    }

	if (maxProbability == 0) {
		return;
	}

    for (unsigned int i = 0; i < particles.size(); i++) {
		particles[i]->probability /= maxProbability;
    }

    resample();
}

float ParticleFilterLocalizer::getMeasurementProbability(Particle* particle, const Measurements& measurements) {
    float probability = 1.0f;

    for (Measurements::const_iterator it = measurements.begin(); it != measurements.end(); it++) {
		std::string landmarkName = it->first;
		Measurement measurement = it->second;

		LandmarkMap::iterator landmarkSearch = landmarks.find(landmarkName);
        if (landmarkSearch == landmarks.end()) {
            std::cout << "- Didnt find landmark '" << landmarkName << "', this should not happen" << std::endl;
            continue;
        }

		Landmark* landmark = landmarkSearch->second;


		Math::Vector landmarkPositionFromParticle = landmark->location - particle->location;
		landmarkPositionFromParticle.getRotated(-particle->orientation);

		CameraTranslator* translator = (measurement.cameraDirection == Dir::FRONT ? frontCameraTranslator : rearCameraTranslator);
		CameraTranslator::CameraPosition excpectedCamPos = translator->getCameraPosition(landmarkPositionFromParticle.x, landmarkPositionFromParticle.y);
		Math::Vector expectation(excpectedCamPos.x, excpectedCamPos.y);
		float error = measurement.bottomPixel.distanceTo(expectation);
		probability *= Math::getGaussian(0, 10.0, error);
    }

    return probability;
}

void ParticleFilterLocalizer::resample() {
    ParticleList resampledParticles;
	int particleCount = (int)particles.size();
    int index = Math::randomInt(0, particleCount - 1);
    float beta = 0.0f;
    float maxProbability = 1.0f;

    for (int i = 0; i < particleCount; i++) {
        beta += Math::randomFloat() * 2.0f * maxProbability;

        while (beta > particles[index]->probability) {
            beta -= particles[index]->probability;
            index = (index + 1) % particleCount;
        }

        resampledParticles.push_back(new Particle(
            particles[index]->location.x,
            particles[index]->location.y,
            particles[index]->orientation,
            particles[index]->probability
        ));
    }

    for (ParticleList::const_iterator it = particles.begin(); it != particles.end(); it++) {
        delete *it;
    }

    particles.clear();

    particles = resampledParticles;
}

Math::Position ParticleFilterLocalizer::getPosition() {
    float xSum = 0.0f;
    float ySum = 0.0f;
    float orientationSum = 0.0f;
    unsigned int particleCount = particles.size();
    Particle* particle;

	if (particleCount == 0) {
		std::cout << "@ NO PARTICLES FOR POSITION" << std::endl;

		return Math::Position();
	}

    for (unsigned int i = 0; i < particleCount; i++) {
        particle = particles[i];

        xSum += particle->location.x;
        ySum += particle->location.y;
        orientationSum += particle->orientation;
    }

	x = xSum / (float)particleCount;
	y = ySum / (float)particleCount;
	orientation = Math::floatModulus(orientationSum / (float)particleCount, Math::TWO_PI);

	//Util::confineField(x, y);

	// generate the state JSON
	std::stringstream stream;

    stream << "{";
	stream << "\"x\": " << x << ",";
	stream << "\"y\": " << y << ",";
	stream << "\"orientation\": " << orientation << ",";
	stream << "\"particles\": [";

	 for (unsigned int i = 0; i < particleCount / 10; i++) {
        particle = particles[i];

		if (i > 0) {
			stream << ",";
		}

		stream << "[" << particle->location.x << ", " << particle->location.y << "]";
	 }

	stream << "]}";

    json = stream.str();

    return Math::Position(
        x,
        y,
        orientation
    );
}
