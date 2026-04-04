#pragma once

#include "miniaudio.h"

#include <array>
#include <vector>
#include <memory>

enum class Sound
{
	Splash = 0,
	Lava,
	Goat,
	Burn,
	Pig,
	Eagle,
	Fish_Flop,
	Frog,
	COUNT
};

class Audio
{
public:
	static Audio &instance()
	{
		static Audio a;
		return a;
	}

	void init();
	void shutdown();
	void update();

	void play(Sound s, float volume = 1.0f);

	bool muted = false;

private:
	Audio() = default;

	ma_engine engine;

	// one base sound per enum
	std::array<ma_sound, (size_t)Sound::COUNT> baseSounds;
	std::array<int, (size_t)Sound::COUNT> sound_count;

	// active instances (for overlap). Store them by pointer so ma_sound is never
	// copied or moved by std::vector reallocation.
	std::vector<std::pair<std::unique_ptr<ma_sound>, int>> active;
};