#include "Simulation.hpp"

#include "PlainBiome.hpp"
#include "LavaBiome.hpp"
#include "goat.hpp"
#include "pig.hpp"

float random_float(float a, float b)
{
	return a + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (b - a)));
}

Simulation::Simulation()
{
	biomes.push_back(std::make_shared<PlainBiome>());
	biomes.push_back(std::make_shared<LavaBiome>());
	biomeIndex = 0;
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
			random_float(0.3f, 2.0f));
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
			random_float(25.0f, 30.0f));

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
				random_float(25.0f, 30.0f));
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
			random_float(0.3f, 2.0f));

		drops2.push_back(d);
	}

	float centerX, centerY;
}

void Simulation::spawnGoat(const Depth &depth, float t, float x, float y, float dirx, float diry)
{
	if (t < 0)
	{
		x = random_float(0.0f, 1.0f);
		y = random_float(0.0f, 1.0f);

		std::shared_ptr<Goat> c = std::make_shared<Goat>();
		c->u = x;
		c->v = y;
		c->h0 = depth.sample_bilinear(c->u, c->v);
		c->dir = (std::rand() % 2 == 0) ? 1.0f : -1.0f;

		c->init_dx = random_float(-1.0f, 1.0f);
		c->init_dy = random_float(-1.0f, 1.0f);

		creatures.push_back(c);

		return;
	}

	if (t - lastGoatTime < 2.0f)
		return;

	lastGoatTime = t;

	std::shared_ptr<Goat> c = std::make_shared<Goat>();
	c->u = x;
	c->v = y;
	c->h0 = depth.sample_bilinear(c->u, c->v);
	c->dir = (std::rand() % 2 == 0) ? 1.0f : -1.0f;

	// store initial direction (normalized)
	float n = std::sqrt(dirx * dirx + diry * diry);
	if (n > 1e-6f)
	{
		c->init_dx = dirx / n;
		c->init_dy = diry / n;
	}
	else
	{
		c->init_dx = 0.0f;
		c->init_dy = 0.0f;
	}

	creatures.push_back(c);
}

void Simulation::spawnPig(const Depth &depth, float t, float x, float y, float dirx, float diry)
{
	if (t < 0)
	{
		x = random_float(0.0f, 1.0f);
		y = random_float(0.0f, 1.0f);

		std::shared_ptr<Pig> c = std::make_shared<Pig>();
		c->u = x;
		c->v = y;
		c->dir = (std::rand() % 2 == 0) ? 1.0f : -1.0f;

		c->init_dx = random_float(-1.0f, 1.0f);
		c->init_dy = random_float(-1.0f, 1.0f);

		creatures.push_back(c);

		return;
	}

	if (t - lastPigTime < 2.0f)
		return;

	lastPigTime = t;

	std::shared_ptr<Pig> c = std::make_shared<Pig>();
	c->u = x;
	c->v = y;
	c->dir = (std::rand() % 2 == 0) ? 1.0f : -1.0f;

	// store initial direction (normalized)
	float n = std::sqrt(dirx * dirx + diry * diry);
	if (n > 1e-6f)
	{
		c->init_dx = dirx / n;
		c->init_dy = diry / n;
	}
	else
	{
		c->init_dx = 0.0f;
		c->init_dy = 0.0f;
	}

	creatures.push_back(c);
}

void Simulation::spawnFish(const Depth &depth, float t, float x, float y, float dirx, float diry)
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
	const float gravity1 = 0.20f;
	const float damping1 = 0.992f;
	const float speed_cap1 = 0.06f;
	const float bounce1 = -0.5f;

	const float gravity2 = 0.10f;
	const float damping2 = 0.97f;
	const float speed_cap2 = 0.025f;
	const float bounce2 = -0.1f;

	for (auto &c : creatures)
		c->step(depth, flowMap1, flowMap2, dt);

	creatures.erase(std::remove_if(creatures.begin(), creatures.end(), [](const std::shared_ptr<Creature> &c) { return !c->alive(); }), creatures.end());

	for (auto &d : rain_)
		d.step(depth, dt, gravity1, damping1, speed_cap1, bounce1, flowMap2, 0.15f);
	for (auto &d : drops1)
		d.step(depth, dt, gravity1, damping1, speed_cap1, bounce1, flowMap2, 0.15f);
	for (auto &d : drops2)
		d.step(depth, dt, gravity2, damping2, speed_cap2, bounce2, flowMap1, 0.15f);

	rain_.erase(std::remove_if(rain_.begin(), rain_.end(), [](const Drop &d) { return !d.isAlive(); }), rain_.end());
	drops1.erase(std::remove_if(drops1.begin(), drops1.end(), [](const Drop &d) { return !d.isAlive(); }), drops1.end());
	drops2.erase(std::remove_if(drops2.begin(), drops2.end(), [](const Drop &d) { return !d.isAlive(); }), drops2.end());

	// Water: lighter, faster-moving, spreads more and fades a bit faster.
	flowMap1.decay(0.975f);
	flowMap1.flowDrops(rain_, 4.5f, 0.0005f);
	flowMap1.flowDrops(drops1, 5.5f, 0.0005f);
	flowMap1.diffuse_once(0.78f, 0.055f);
	flowMap1.diffuse_once(0.78f, 0.055f);

	// Lava: thicker and more honey-like: slower spreading, stronger deposit,
	// and much more persistent.
	flowMap2.decay(0.992f);
	flowMap2.flowDrops(drops2, 7.0f, 0.0002f);
	flowMap2.diffuse_once(0.94f, 0.015f);
}

void Simulation::init(const int w, const int h)
{
	flowMap1.resize(w, h);
	flowMap2.resize(w, h);
}

void Simulation::nextBiome()
{
	biomeIndex = (biomeIndex + 1) % biomes.size();
}

void Simulation::prevBiome()
{
	biomeIndex = (biomeIndex - 1 + biomes.size()) % biomes.size();
}

void Simulation::goToBiome(int index)
{
	if (index >= 0 && index < biomes.size())
		biomeIndex = index;
}