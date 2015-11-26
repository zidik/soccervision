#ifndef PARTICLEFILTERLOCALIZER_H
#define PARTICLEFILTERLOCALIZER_H

#include "Localizer.h"
#include "Maths.h"

#include <string>
#include <unordered_map>
#include <vector>
#include "CameraTranslator.h"
#include "Pixel.h"


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
		Measurement(Landmark::Type type, Pixel bottomPixel, Dir cameraDirection) : type(type), bottomPixel{ bottomPixel }, cameraDirection{ cameraDirection } {};
		Landmark::Type type;
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
	typedef std::vector<Measurement> Measurements;
	typedef std::vector<Particle*> ParticleList;
	

	ParticleFilterLocalizer(
		CameraTranslator* frontCameraTranslator,
		CameraTranslator* rearCameraTranslator,
		int particleCount,
		float forwardNoise,
		float turnNoise
	);
    ~ParticleFilterLocalizer();

    void addLandmark(Landmark* landmark);
	void addLandmark(const Landmark::Type type, const Location& location);
	void addLandmark(const Landmark::Type type, const Locations& locations);

	void move(float velocityX, float velocityY, float omega, float dt) override { move(velocityX, velocityY, omega, dt, false); }
    void move(float velocityX, float velocityY, float omega, float dt, bool exact);
    double getMeasurementProbability(Particle* const particle, const Measurements& measurements) const;
	double evaluateParticleProbabilityPart(const Particle& particle, const Landmark& landmark, const Measurement& measurement) const;
	void setPosition(float x, float y, float orientation) override;
    void update(const Measurements& measurements);
    void resample();
	
	void calculatePosition();
	Math::Position getPosition() const;
	const ParticleList& getParticles() const { return particles; }
	std::string getJSON() const;
	CameraTranslator* getTranslator(const Dir cameraDirection) const;
	Math::Vector getWorldPosition(Measurement measurement);

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
    bool lost = false;
};

std::ostream& operator<< (std::ostream & os, ParticleFilterLocalizer::Landmark::Type type);

#endif // PARTICLEFILTERLOCALIZER_H
