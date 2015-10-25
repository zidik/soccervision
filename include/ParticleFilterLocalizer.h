#ifndef PARTICLEFILTERLOCALIZER_H
#define PARTICLEFILTERLOCALIZER_H

#include "Localizer.h"
#include "Maths.h"
#include "Config.h"

#include <string>
#include <vector>
#include "CameraTranslator.h"
#include <unordered_map>

// TODO Move this into the actual class

class ParticleFilterLocalizer : public Localizer {

public:
	typedef Math::Vector Location;
	typedef std::vector<Location> Locations;

	struct Landmark {
		enum class Type { YellowGoalCenter, BlueGoalCenter, FieldCorner};
		Landmark(Type type, const Location& location) : type(type), locations(1, location) {}
		Landmark(Type type, const Locations& locations) : type(type), locations(locations) {}
		
		Type type;
		Locations locations;
	};

	struct Measurement
	{
		Measurement() = default;
		Measurement(Math::Vector bottomPixel, Dir cameraDirection) : bottomPixel{ bottomPixel }, cameraDirection{ cameraDirection } {};
		Math::Vector bottomPixel;
		Dir cameraDirection;
	};

	struct Particle {
		Particle(float x, float y, float orientation, double probability) : location(x, y), orientation(orientation), probability(probability) {};

		Location location;
		float orientation;
		double probability;
	};

	typedef std::unordered_map<Landmark::Type, Landmark*> LandmarkMap;
	typedef std::unordered_map<Landmark::Type, Measurement> MeasurementMap;
	typedef std::vector<Particle*> ParticleList;
	

	ParticleFilterLocalizer(
		CameraTranslator* frontCameraTranslator,
		CameraTranslator* rearCameraTranslator,
		int particleCount = Config::robotLocalizerParticleCount,
		float forwardNoise = Config::robotLocalizerForwardNoise,
		float turnNoise = Config::robotLocalizerTurnNoise
	);
    ~ParticleFilterLocalizer();

	void generateRandomParticles(std::vector<Particle*>& particleVector, int particleCount);
    void addLandmark(Landmark* landmark);
	void addLandmark(Landmark::Type type, const Location& location);
	void addLandmark(Landmark::Type type, const Locations& locations);

	void move(float velocityX, float velocityY, float omega, float dt) { move(velocityX, velocityY, omega, dt, false); }
    void move(float velocityX, float velocityY, float omega, float dt, bool exact);
    double evaluateParticleProbability(Particle* particle, const MeasurementMap& measurements);
	double evaluateParticleProbabilityPart(const Particle& particle, const Landmark& landmark, const Measurement& measurement);
	void setPosition(float x, float y, float orientation);
    void update(const MeasurementMap& measurements);
    void resample();
    Math::Position getPosition();
	const ParticleList& getParticles() const { return particles; }
	std::string getJSON() { return json; }

private:
	CameraTranslator* frontCameraTranslator;
	CameraTranslator* rearCameraTranslator;
    const int particleCount;
    float forwardNoise;
    float turnNoise;
    LandmarkMap landmarks;
    ParticleList particles;
	std::string json;

};

#endif // PARTICLEFILTERLOCALIZER_H
