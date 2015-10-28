#ifndef PARTICLEFILTERLOCALIZER_H
#define PARTICLEFILTERLOCALIZER_H

#include "Localizer.h"
#include "Maths.h"
#include "Config.h"

#include <string>
#include <map>
#include <vector>
#include "CameraTranslator.h"

// TODO Move this into the actual class

class ParticleFilterLocalizer : public Localizer {

public:
	struct Landmark {
		Landmark(std::string name, float x, float y) : name(name), location(x, y) {}
		
		const std::string name;
		const Math::Vector location;
	};

	struct Particle {
		Particle(float x, float y, float orientation, float probability) : location(x, y), orientation(orientation), probability(probability) {};
		
		Math::Vector location;
		float orientation;
		float probability;
	};

	struct Measurement
	{
		Measurement() = default;
		Measurement(Math::Vector bottomPixel, Dir cameraDirection) : bottomPixel{ bottomPixel }, cameraDirection{ cameraDirection } {};
		Math::Vector bottomPixel;
		Dir cameraDirection;
	};

	typedef std::map<std::string, Landmark*> LandmarkMap;
	typedef std::vector<Particle*> ParticleList;
	typedef std::map<std::string, Measurement> Measurements;

	ParticleFilterLocalizer(
		CameraTranslator* frontCameraTranslator,
		CameraTranslator* rearCameraTranslator,
		int particleCount = Config::robotLocalizerParticleCount,
		float forwardNoise = Config::robotLocalizerForwardNoise,
		float turnNoise = Config::robotLocalizerTurnNoise
	);
    ~ParticleFilterLocalizer();

    void addLandmark(Landmark* landmark);
    void addLandmark(std::string name, float x, float y);
	void move(float velocityX, float velocityY, float omega, float dt) { move(velocityX, velocityY, omega, dt, false); }
    void move(float velocityX, float velocityY, float omega, float dt, bool exact);
    float getMeasurementProbability(Particle* particle, const Measurements& measurements) const;
	void setPosition(float x, float y, float orientation);
    void update(const Measurements& measurements);
    void resample();
	
	void calculatePosition();
	Math::Position getPosition() const;
	const ParticleList& getParticles() const { return particles; }
	std::string getJSON() const;

private:
	void removeZeroProbabilityParticles();
	void sampleParticles(ParticleList& newParticles, int resampledParticleCount);
	void generateRandomParticles(std::vector<Particle*>& particleVector, int particleCount);

	CameraTranslator* frontCameraTranslator;
	CameraTranslator* rearCameraTranslator;
	const int particleCount;
    float forwardNoise;
    float turnNoise;
    LandmarkMap landmarks;
    ParticleList particles;
};

#endif // PARTICLEFILTERLOCALIZER_H
