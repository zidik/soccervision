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
	generateRandomParticles(particles, particleCount);
	json = "null";
}

ParticleFilterLocalizer::~ParticleFilterLocalizer() {
    for (ParticleList::const_iterator it = particles.begin(); it != particles.end(); ++it) {
        delete *it;
    }

    particles.clear();

    for (LandmarkMap::const_iterator it = landmarks.begin(); it != landmarks.end(); ++it) {
        delete it->second;
    }

    landmarks.clear();
}

void ParticleFilterLocalizer::generateRandomParticles(std::vector<Particle*>& particleVector, int particleCount) {
	for (int i = 0; i < particleCount; i++) {
		particleVector.push_back(new Particle(
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
		particles[i]->probability = 1.0f;
    }
}

void ParticleFilterLocalizer::move(float velocityX, float velocityY, float velocityOmega, float dt, bool exact) {
	for (unsigned int i = 0; i < particles.size(); i++) {
		float particleVelocityX		= velocityX;
		float particleVelocityY		= velocityY;
		float particleVelocityOmega = velocityOmega;
		
		if (!exact)
		{
			particleVelocityX		+= Math::randomGaussian(forwardNoise);
			particleVelocityY		+= Math::randomGaussian(forwardNoise);
			particleVelocityOmega	+= Math::randomGaussian(turnNoise);
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
    float maxProbability = 0.0;

    for (unsigned int i = 0; i < particles.size(); i++) {
        particle = particles[i];

		if (
			particle->location.x - 3 > Config::fieldWidth ||
			particle->location.y - 3 > Config::fieldHeight ||
			particle->location.x + 3 < 0 ||
			particle->location.y + 3 < 0
		) {
			particle->probability = 0;
			continue;
		}


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

    for (Measurements::const_iterator it = measurements.begin(); it != measurements.end(); ++it) {
		std::string landmarkName = it->first;
		Measurement measurement = it->second;

		LandmarkMap::iterator landmarkSearch = landmarks.find(landmarkName);
        if (landmarkSearch == landmarks.end()) {
            std::cout << "- Didnt find landmark '" << landmarkName << "', this should not happen" << std::endl;
            continue;
        }

		Landmark* landmark = landmarkSearch->second;

		Math::Vector diff = (landmark->location - particle->location);

		//FIX!!!
		diff = Math::Vector(diff.x, -diff.y);

		diff = diff.getRotated(particle->orientation);

		CameraTranslator* translator = (measurement.cameraDirection == Dir::FRONT ? frontCameraTranslator : rearCameraTranslator);
		if(measurement.cameraDirection == Dir::FRONT)
		{
			translator = frontCameraTranslator;
		}
		else
		{
			translator = rearCameraTranslator;
			diff.x = -diff.x;
			diff.y = -diff.y;
		}
		CameraTranslator::CameraPosition excpectedCamPos = translator->getCameraPosition(diff);
		Math::Vector expectation(excpectedCamPos.x, excpectedCamPos.y);
		float error = measurement.bottomPixel.distanceTo(expectation);
		probability *= Math::getGaussian(0, 50.0, error);
    }

    return probability;
}

void ParticleFilterLocalizer::resample() {
    ParticleList newParticles;
	int particleCount = (int)particles.size();
	int randomParticleCount = particleCount / 10;
	int resampledParticleCount = particleCount - randomParticleCount;

    int index = Math::randomInt(0, particleCount - 1);
    float beta = 0.0f;
    float maxProbability = 1.0f;

	generateRandomParticles(newParticles, randomParticleCount);

    for (int i = 0; i < resampledParticleCount; i++) {
        beta += Math::randomFloat() * 2.0f * maxProbability;

        while (beta > particles[index]->probability) {
            beta -= particles[index]->probability;
            index = (index + 1) % particleCount;
        }

		newParticles.push_back(new Particle(*particles[index]));
    }

    for (ParticleList::const_iterator it = particles.begin(); it != particles.end(); ++it) {
        delete *it;
    }

    particles.clear();

    particles = newParticles;
}

Math::Position ParticleFilterLocalizer::getPosition() {
    float xSum = 0.0f;
    float ySum = 0.0f;
    float orientationSum = 0.0f;
	float weightSum = 0.0f;
    unsigned int particleCount = particles.size();
    Particle* particle;

	if (particleCount == 0) {
		std::cout << "@ NO PARTICLES FOR POSITION" << std::endl;
		return Math::Position();
	}

    for (unsigned int i = 0; i < particleCount; i++) {
        particle = particles[i];
		float weight = particle->probability;

        xSum += particle->location.x * weight;
        ySum += particle->location.y * weight;
        orientationSum += particle->orientation * weight;
		weightSum += weight;
    }

	if (weightSum != 0.0f) {
		x = xSum / weightSum;
		y = ySum / weightSum;
		orientation = Math::floatModulus(orientationSum / weightSum, Math::TWO_PI);
	}

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
