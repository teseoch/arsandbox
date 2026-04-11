#include "audio.hpp"

#include <string>

static int MAX_SOUND_INSTANCES = 7; // max overlapping instances of the same sound

void Audio::init()
{
	ma_engine_init(nullptr, &engine);

	const std::string folder = AR_SOUND_FOLDER;

	const std::vector<std::string> paths = {
		folder + "/splash.wav",
		folder + "/lava.wav",
		folder + "/goat.wav",
		folder + "/burn.wav",
		folder + "/pig.wav",
		folder + "/eagle.wav",
		folder + "/fish-flop.wav",
		folder + "/frog.wav",
		folder + "/wolf.wav",
		folder + "/rare.wav"};

	for (int i = 0; i < paths.size(); ++i)
	{
		ma_sound_init_from_file(&engine, paths[i].c_str(), 0, nullptr, nullptr, &baseSounds[i]);
	}
}

void Audio::play(Sound s, float volume)
{
	if (muted)
		return;

	if (sound_count[(int)s] >= MAX_SOUND_INSTANCES) // limit max overlapping instances of the same sound
		return;

	auto instance = std::make_unique<ma_sound>();

	if (ma_sound_init_copy(&engine, &baseSounds[(int)s], 0, nullptr, instance.get()) != MA_SUCCESS)
		return;

	ma_sound_set_volume(instance.get(), volume);
	if (ma_sound_start(instance.get()) != MA_SUCCESS)
	{
		ma_sound_uninit(instance.get());
		return;
	}

	active.emplace_back(std::move(instance), (int)s);
	sound_count[(int)s]++;
}

void Audio::update()
{
	for (size_t i = 0; i < active.size();)
	{
		if (!ma_sound_is_playing(active[i].first.get()))
		{
			ma_sound_uninit(active[i].first.get());
			sound_count[active[i].second]--;
			active.erase(active.begin() + i);
		}
		else
		{
			++i;
		}
	}
}

void Audio::shutdown()
{
	for (auto &s : baseSounds)
		ma_sound_uninit(&s);

	for (auto &s : active)
		ma_sound_uninit(s.first.get());
	active.clear();

	ma_engine_uninit(&engine);
}