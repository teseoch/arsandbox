#include "Simulation.hpp"

#include "PlainBiome.hpp"

float random_float(float a, float b)
{
	return a + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (b - a)));
}

Simulation::Simulation()
{
	currentBiome = std::make_shared<PlainBiome>();
}

void Simulation::rain(const float t, const float u, const float v)
{
	for (int i = 0; i < 5; i++)
	{
		float r = random_float(0.01f, 0.36f);
		float angle = random_float(0.0f, 2 * 3.14159f);

		float du = ru * r * std::cos(angle);
		float dv = rv * r * std::sin(angle);

		Drop d;
		d.reset(
			std::clamp(u + du, 0.0f, 1.0f),
			std::clamp(v + dv, 0.0f, 1.0f),
			random_float(0.3f, 2.0f));
		rain_.push_back(d);
	}
}

void Simulation::randomRain()
{
	for (int i = 0; i < 5; i++)
	{
		Drop d;
		d.reset(
			random_float(0.0f, 1.0f),
			random_float(0.0f, 1.0f),
			random_float(25.0f, 30.0f));
		rain_.push_back(d);
	}
}

void Simulation::mega1(float t, float u, float v)
{
	if (u < 0 && v < 0)
	{
		const float centerX = random_float(0.0f, 1.0f);
		const float centerY = random_float(0.0f, 1.0f);

		for (int i = 0; i < 500; i++)
		{
			Drop d;
			float r = random_float(-0.05f, 0.05f);
			float angle = random_float(0.0f, 2 * 3.14159f);
			d.reset(
				centerX + r * std::cos(angle),
				centerY + r * std::sin(angle),
				random_float(25.0f, 30.0f));
			drops1.push_back(d);
		}

		return;
	}

	if (t - lastMega1Time < 0.5)
		return;

	lastMega1Time = t;

	for (int i = 0; i < 100; i++)
	{
		Drop d;

		float r = random_float(0.01f, 0.06f);
		float angle = random_float(0.0f, 2 * 3.14159f);

		float du = ru * r * std::cos(angle);
		float dv = rv * r * std::sin(angle);

		d.reset(
			std::clamp(u + du, 0.0f, 1.0f),
			std::clamp(v + dv, 0.0f, 1.0f),
			random_float(25, 5));

		drops1.push_back(d);
	}

	float centerX, centerY;
}

void Simulation::mega2(float t, float u, float v)
{
	if (u < 0 && v < 0)
	{
		const float centerX = random_float(0.0f, 1.0f);
		const float centerY = random_float(0.0f, 1.0f);

		for (int i = 0; i < 500; i++)
		{
			Drop d;
			float r = random_float(-0.05f, 0.05f);
			float angle = random_float(0.0f, 2 * 3.14159f);
			d.reset(
				centerX + r * std::cos(angle),
				centerY + r * std::sin(angle),
				random_float(25, 5));
			drops2.push_back(d);
		}

		return;
	}

	if (t - lastMega2Time < 0.5)
		return;

	lastMega2Time = t;

	for (int i = 0; i < 100; i++)
	{
		Drop d;

		float r = random_float(0.01f, 0.06f);
		float angle = random_float(0.0f, 2 * 3.14159f);

		float du = ru * r * std::cos(angle);
		float dv = rv * r * std::sin(angle);

		d.reset(
			std::clamp(u + du, 0.0f, 1.0f),
			std::clamp(v + dv, 0.0f, 1.0f),
			25.0f + 5.0f * (((float)std::rand()) / RAND_MAX));

		drops2.push_back(d);
	}

	float centerX, centerY;
}

void Simulation::spawnGoat(const Depth &depth, float x, float y, float h0, float dirx, float diry)
{
	for (int i = 0; i < 5; i++)
	{
		Creature c;
		c.u = ((float)std::rand()) / RAND_MAX;
		c.v = ((float)std::rand()) / RAND_MAX;
		c.h0 = depth.sample_bilinear(c.u, c.v);
		c.dir = (std::rand() % 2 == 0) ? 1.0f : -1.0f;
		creatures.push_back(c);
	}
}

void Simulation::spawnPig(const Depth &depth, float x, float y, float h0, float dirx, float diry)
{
}

void Simulation::spawnFish(const Depth &depth, float x, float y, float h0, float dirx, float diry)
{
}

void Simulation::clear()
{
	rain_.clear();
	drops1.clear();
	drops2.clear();
	creatures.clear();

	flowMap1.clear();
	flowMap2.clear();
}

void Simulation::step(const Depth &depth, float dt)
{
	for (auto &c : creatures)
		c.step(depth, dt);

	for (auto &d : rain_)
		d.step(depth, dt);
	for (auto &d : drops1)
		d.step(depth, dt);
	for (auto &d : drops2)
		d.step(depth, dt);

	rain_.erase(std::remove_if(rain_.begin(), rain_.end(), [](const Drop &d) { return !d.isAlive(); }), rain_.end());
	drops1.erase(std::remove_if(drops1.begin(), drops1.end(), [](const Drop &d) { return !d.isAlive(); }), drops1.end());
	drops2.erase(std::remove_if(drops2.begin(), drops2.end(), [](const Drop &d) { return !d.isAlive(); }), drops2.end());

	flowMap1.decay(0.98f);
	flowMap2.decay(0.98f);

	flowMap1.flowDrops(rain_);
	flowMap1.flowDrops(drops1);
	flowMap2.flowDrops(drops2);

	flowMap1.diffuse_once();
	flowMap2.diffuse_once();
}

void Simulation::init(const int w, const int h)
{
	flowMap1.resize(w, h);
	flowMap2.resize(w, h);
}

void Simulation::nextBiome()
{
	// TODO
}

void Simulation::prevBiome()
{
	// TODO
}