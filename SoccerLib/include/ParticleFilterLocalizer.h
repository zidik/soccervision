#ifndef PARTICLEFILTERLOCALIZER_H
#define PARTICLEFILTERLOCALIZER_H

#include "Localizer.h"
#include "Maths.h"
#include "Config.h"

#include <string>
#include <unordered_map>
#include <vector>
#include "CameraTranslator.h"
#include "Pixel.h"

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
		Measurement(Pixel bottomPixel, Dir cameraDirection) : bottomPixel{ bottomPixel }, cameraDirection{ cameraDirection } {};
		Pixel bottomPixel;
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

    void addLandmark(Landmark* landmark);
	void addLandmark(const Landmark::Type type, const Location& location);
	void addLandmark(const Landmark::Type type, const Locations& locations);

	void move(float velocityX, float velocityY, float omega, float dt) override { move(velocityX, velocityY, omega, dt, false); }
    void move(float velocityX, float velocityY, float omega, float dt, bool exact);
    double getMeasurementProbability(Particle* const particle, const MeasurementMap& measurements) const;
	double evaluateParticleProbabilityPart(const Particle& particle, const Landmark& landmark, const Measurement& measurement) const;
	void setPosition(float x, float y, float orientation) override;
    void update(const MeasurementMap& measurements);
    void resample();
	
	void calculatePosition();
	Math::Position getPosition() const;
	const ParticleList& getParticles() const { return particles; }
	std::string getJSON() const;
	Math::Vector getMeasurementVector(Measurement measurement);
private:
	void removeZeroProbabilityParticles();
	void sampleParticles(ParticleList& newParticles, int resampledParticleCount);
	void generateRandomParticles(std::vector<Particle*>& particleVector, int particleCount);

private:
	CameraTranslator* frontCameraTranslator;
	CameraTranslator* rearCameraTranslator;
    const int particleCount;
    float forwardNoise;
    float turnNoise;
    LandmarkMap landmarks;
    ParticleList particles;
};

#endif // PARTICLEFILTERLOCALIZER_H
