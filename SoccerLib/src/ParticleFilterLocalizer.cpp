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

    for (std::pair<Landmark::Type, Landmark*> pair : landmarks) {
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
			0.0
			));
	}
}

void ParticleFilterLocalizer::addLandmark(Landmark* landmark) {
    landmarks[landmark->type] = landmark;
}

void ParticleFilterLocalizer::addLandmark(const Landmark::Type type, const Location& location) {
	addLandmark(new Landmark(type, location));
}

void ParticleFilterLocalizer::addLandmark(const Landmark::Type type, const Locations & locations) {
	addLandmark(new Landmark(type, locations));
}

void ParticleFilterLocalizer::setPosition(float x, float y, float orientation) {
	for (unsigned int i = 0; i < particles.size(); i++) {
		particles[i]->orientation = orientation;
        particles[i]->location.x = x;
        particles[i]->location.y = y;
		particles[i]->probability = 1.0;
    }
}

void ParticleFilterLocalizer::move(float velocityX, float velocityY, float velocityOmega, float dt, bool exact) {
	for (unsigned int i = 0; i < particles.size(); i++) {
		float particleVelocityX		= velocityX;
		float particleVelocityY		= velocityY;
		float particleVelocityOmega = velocityOmega;
		
		if (!exact)
		{
			//TODO: actually this should maybe not be gaussian noise but linear
			//also, noise should be higher when speed/acceleration is high
			particleVelocityX		+= Math::randomGaussian(forwardNoise);
			particleVelocityY		+= Math::randomGaussian(forwardNoise);
			particleVelocityOmega	+= Math::randomGaussian(turnNoise * (lost ? 5 : 1)); //This should help particles to rotate (and find the position) when lost
		}

		float particleVelocityXLocal = particleVelocityX * Math::cos(particles[i]->orientation) - particleVelocityY * Math::sin(particles[i]->orientation);
		float particleVelocityYLocal = particleVelocityX * Math::sin(particles[i]->orientation) + particleVelocityY * Math::cos(particles[i]->orientation);
		
		particles[i]->orientation += particleVelocityOmega * dt;
		particles[i]->location.x += particleVelocityXLocal * dt;
		particles[i]->location.y += particleVelocityYLocal * dt;
	}
}

void ParticleFilterLocalizer::update(const Measurements& measurements) {
    double maxProbability = 0.0;

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

double ParticleFilterLocalizer::getMeasurementProbability(Particle* const particle, const Measurements& measurements) const {
    double probability = 1.0;

	for (const Measurement measurement : measurements)
	{
		LandmarkMap::const_iterator landmarkSearch = landmarks.find(measurement.type);
        if (landmarkSearch == landmarks.end()) {
            std::cout << "- Didnt find landmark, this should not happen" << std::endl;
            continue;
        }
		Landmark* landmark = landmarkSearch->second;
		probability *= evaluateParticleProbabilityPart(*particle, *landmark, measurement);

    }

    return probability;
}

double ParticleFilterLocalizer::evaluateParticleProbabilityPart(const Particle& particle, const Landmark& landmark, const Measurement& measurement) const
{
	double maximumProbability = 0.0;
	for (Location landmarkLocation : landmark.locations)
	{
		Math::Vector diff = (landmarkLocation - particle.location);
		
		diff = diff.getRotated(particle.orientation);

		CameraTranslator* translator;
		if(measurement.cameraDirection == Dir::FRONT) {
			translator = frontCameraTranslator;
		}
		else {
			translator = rearCameraTranslator;
			diff.x = -diff.x;
			diff.y = -diff.y;
		}

		if (diff.x < 0) {
			// Camera can not see behind itself
		}
		else {
		Pixel expectation = translator->getCameraPosition(diff);
		float error = measurement.bottomPixel.distanceTo(expectation);
			double probability = Math::getGaussian(0.0, 30.0, (double)error);
			maximumProbability = Math::max(maximumProbability, probability);
    }

    }
	return maximumProbability; //TODO:Maybe this should not find the maximum but just multiply them all together? (would cause reallly small numbers, but maybe better represetation?)
}

void ParticleFilterLocalizer::resample() {
	//removeZeroProbabilityParticles(); // Is that a good idea?

	ParticleList newParticles;
	newParticles.reserve(particleCount);

	double probability_sum = 0;
	for (Particle* particle : particles) {
		probability_sum += particle->probability;
	}
    lost = probability_sum < 0.1;

	static bool wasLost = false;

	if (lost) {
		if (!wasLost) {
			std::cout << "Particles had probability sum less than 0.1 - robot is lost" << std::endl;
			wasLost = true;
		}
	    int randomCount = 0;//particleCount / 100;
	    for (int i = 0; i < particleCount - randomCount; i++) {
	        newParticles.push_back(new Particle(*particles[i]));
	    }
	    generateRandomParticles(newParticles, randomCount);
	}
	else {
		wasLost = false;
	    int randomParticleCount = 0;//particleCount / 100;
	    int resampledParticleCount = particleCount - randomParticleCount;
	    generateRandomParticles(newParticles, randomParticleCount);
	    sampleParticles(newParticles, resampledParticleCount);
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

/**
	Takes a number of random samples from particles based on probability of particles.
	Particles must have maximum probability of 1.0
	@param newParticles a reference to a list of particles where samples are placed
	@param resampledParticleCount number of particles to sample
*/
void ParticleFilterLocalizer::sampleParticles(ParticleList& newParticles, int resampledParticleCount){
	const double maxProbability = 1.0;
	double beta = 0.0;
	int index = Math::randomInt(0, particles.size() - 1);
	for (int i = 0; i < resampledParticleCount; i++) {
        beta += Math::randomFloat() * 2.0 * maxProbability;
		//Limit loops by random!
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
	unsigned int particleCount = particles.size();

	if (particleCount == 0) {
		throw std::runtime_error("Cannot calculate position with 0 particles");
	}

	for (Particle* particle : particles){
		xSum += particle->location.x;
		ySum += particle->location.y;
		orientationSum += particle->orientation;
	}

	x = (float)(xSum / particleCount);
	y = (float)(ySum / particleCount);
	orientation = Math::floatModulus((float)(orientationSum / particleCount), Math::TWO_PI);
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
			first = false;
		}
		else {
			stream << ",";
		}

		stream << "[" << particle->location.x << ", " << particle->location.y << "]";
	}

	stream << "]}";

	return stream.str();
}

CameraTranslator* ParticleFilterLocalizer::getTranslator(const Dir cameraDirection) const {
	switch (cameraDirection) {
	case FRONT:
		return frontCameraTranslator;
	case REAR:
		return rearCameraTranslator;
	default:
		throw std::runtime_error("Unexpected camera direction - can not choose translator");
	}
}

Math::Vector ParticleFilterLocalizer::getWorldPosition(Measurement measurement) {
	CameraTranslator* translator = getTranslator(measurement.cameraDirection);
	Math::Vector position = translator->getWorldPosition(measurement.bottomPixel);

	if (measurement.cameraDirection == Dir::REAR) {
		position = -position;
	}
	return position;
}

Math::Position ParticleFilterLocalizer::getPosition() const {
    return Math::Position(
        x,
        y,
        orientation
    );
}

std::ostream& operator<< (std::ostream & os, ParticleFilterLocalizer::Landmark::Type type) {
	switch (type) {
		case ParticleFilterLocalizer::Landmark::Type::BlueGoalCenter: return os << "blueGoalCenter";
		case ParticleFilterLocalizer::Landmark::Type::YellowGoalCenter: return os << "yellowGoalCenter";
		case ParticleFilterLocalizer::Landmark::Type::FieldCorner: return os << "fieldCorner";
		// omit default case to trigger compiler warning for missing cases
	};
	return os << "Error: No string specified for this landmark type!";
}
