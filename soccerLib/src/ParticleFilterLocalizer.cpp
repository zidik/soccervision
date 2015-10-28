#include "ParticleFilterLocalizer.h"
#include "Maths.h"
#include "Config.h"

#include <boost/range/adaptor/strided.hpp>

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
		particles.reserve(particleCount);
		generateRandomParticles(particles, particleCount);
	}

ParticleFilterLocalizer::~ParticleFilterLocalizer() {
	for (Particle* particle : particles){
        delete particle;
    }
    particles.clear();

    for (std::pair<std::string, Landmark*> pair : landmarks) {
        delete pair.second;
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
    float maxProbability = 0.0f;

	for (Particle* particle : particles){
        particle->probability = getMeasurementProbability(particle, measurements);

		if (
			particle->location.x - 3 > Config::fieldWidth ||
			particle->location.y - 3 > Config::fieldHeight ||
			particle->location.x + 3 < 0 ||
			particle->location.y + 3 < 0
			) {
			particle->probability = 0;
		}

		maxProbability = Math::max(maxProbability, particle->probability);
    }

	if (maxProbability != 0) {
		for (Particle* particle : particles){
			particle->probability /= maxProbability;
		}
	}

    resample();
}

float ParticleFilterLocalizer::getMeasurementProbability(Particle* particle, const Measurements& measurements) const {
    float probability = 1.0f;

	for (std::pair<std::string, Measurement> pair : measurements){
		std::string landmarkName = pair.first;
		Measurement measurement = pair.second;

		LandmarkMap::const_iterator landmarkSearch = landmarks.find(landmarkName);
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

		if (diff.x < 0)
		{
			// Camera can not see behind itself
			probability = 0.0f;
		}
		CameraTranslator::CameraPosition excpectedCamPos = translator->getCameraPosition(diff);
		Math::Vector expectation(excpectedCamPos.x, excpectedCamPos.y);
		float error = measurement.bottomPixel.distanceTo(expectation);
		probability *= Math::getGaussian(0, 50.0, error);
    }

    return probability;
}

void ParticleFilterLocalizer::resample() {
	removeZeroProbabilityParticles();

	ParticleList newParticles;
	newParticles.reserve(particleCount);
	if (particles.size() > 0) {
		int randomParticleCount = particleCount / 10;
		int resampledParticleCount = particleCount - randomParticleCount;
		generateRandomParticles(newParticles, randomParticleCount);
		sampleParticles(newParticles, resampledParticleCount);
	}
	else {
		//All particles had probability 0.0
		generateRandomParticles(newParticles, particleCount);
	}
	
	for (Particle* particle : particles){
        delete particle;
    }

    particles.clear();

    particles = newParticles;
}

void ParticleFilterLocalizer::removeZeroProbabilityParticles(){
	for (ParticleList::iterator it = particles.begin(); it != particles.end(); ) {
		if ((*it)->probability == 0.0f) {
			delete *it;
			it = particles.erase(it);
		}
		else {
			++it;
		}
	}
}

void ParticleFilterLocalizer::sampleParticles(ParticleList& newParticles, int resampledParticleCount){
	float beta = 0.0f;
	float maxProbability = 1.0f;
	int index = Math::randomInt(0, particles.size() - 1);
	for (int i = 0; i < resampledParticleCount; i++) {
		beta += Math::randomFloat() * 2.0f * maxProbability;

		while (beta > particles[index]->probability) {
			beta -= particles[index]->probability;
			index = (index + 1) % particles.size();
		}

		newParticles.push_back(new Particle(*particles[index]));
	}
}


void ParticleFilterLocalizer::calculatePosition()
{
	float xSum = 0.0f;
	float ySum = 0.0f;
	float orientationSum = 0.0f;
	float weightSum = 0.0f;
	unsigned int particleCount = particles.size();
	Particle* particle;

	if (particleCount == 0) {
		std::cout << "@ NO PARTICLES FOR POSITION" << std::endl;
		return;
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
}

std::string ParticleFilterLocalizer::getJSON() const {
	std::stringstream stream;

	stream << "{";
	stream << "\"x\": " << x << ",";
	stream << "\"y\": " << y << ",";
	stream << "\"orientation\": " << orientation << ",";
	stream << "\"particles\": [";
	bool first = true;
	for (Particle* particle : particles | boost::adaptors::strided(10)){
		if (first) {
			stream << ",";
			first = false;
		}

		stream << "[" << particle->location.x << ", " << particle->location.y << "]";
	}

	stream << "]}";

	return stream.str();
}

Math::Position ParticleFilterLocalizer::getPosition() const {
    return Math::Position(
        x,
        y,
        orientation
    );
}
