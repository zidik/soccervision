#include <boost/test/unit_test.hpp>
#include "ParticleFilterLocalizer.h"

BOOST_AUTO_TEST_SUITE(ParticleFilterLocalizerTests)
BOOST_AUTO_TEST_CASE(ParticleFilterLocalizer_InitialParticleCount_Test)
{
	int particle_count = 1234;
	ParticleFilterLocalizer particleFilterLocalizer(nullptr, nullptr, particle_count, 0, 0);
	BOOST_TEST(particleFilterLocalizer.getParticles().size() == particle_count);
}
BOOST_AUTO_TEST_CASE(ParticleFilterLocalizer_ResampleParticleCount_Test)
{
	int particle_count = 1234;
	ParticleFilterLocalizer particleFilterLocalizer(nullptr, nullptr, particle_count, 0, 0);
	particleFilterLocalizer.resample();
	BOOST_TEST(particleFilterLocalizer.getParticles().size() == particle_count);
}

BOOST_AUTO_TEST_SUITE_END()