/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/engines/cine/sound.h $
 * $Id: sound.h 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

#ifndef CINE_SOUND_H_
#define CINE_SOUND_H_

#include "common/util.h"
#include "sound/mixer.h"

namespace Audio {
	class AudioStream;
}

namespace Cine {

class CineEngine;

class Sound {
public:

	Sound(Audio::Mixer *mixer, CineEngine *vm) : _mixer(mixer), _vm(vm) {}
	virtual ~Sound() {}

	virtual void loadMusic(const char *name) = 0;
	virtual void playMusic() = 0;
	virtual void stopMusic() = 0;
	virtual void fadeOutMusic() = 0;

	virtual void playSound(int channel, int frequency, const uint8 *data, int size, int volumeStep, int stepCount, int volume, int repeat) = 0;
	virtual void stopSound(int channel) = 0;
	virtual void update() {}

protected:

	Audio::Mixer *_mixer;
	CineEngine *_vm;
};

class PCSoundDriver;
class PCSoundFxPlayer;

class PCSound : public Sound {
public:

	PCSound(Audio::Mixer *mixer, CineEngine *vm);
	virtual ~PCSound();

	virtual void loadMusic(const char *name);
	virtual void playMusic();
	virtual void stopMusic();
	virtual void fadeOutMusic();

	virtual void playSound(int channel, int frequency, const uint8 *data, int size, int volumeStep, int stepCount, int volume, int repeat);
	virtual void stopSound(int channel);

protected:

	PCSoundDriver *_soundDriver;
	PCSoundFxPlayer *_player;
};

class PaulaSound : public Sound {
public:

	PaulaSound(Audio::Mixer *mixer, CineEngine *vm);
	virtual ~PaulaSound();

	virtual void loadMusic(const char *name);
	virtual void playMusic();
	virtual void stopMusic();
	virtual void fadeOutMusic();

	virtual void playSound(int channel, int frequency, const uint8 *data, int size, int volumeStep, int stepCount, int volume, int repeat);
	virtual void stopSound(int channel);
	virtual void update();

	enum {
		PAULA_FREQ = 7093789,
		NUM_CHANNELS = 4,
		SPL_HDR_SIZE = 22
	};

	struct SoundChannel {
		int frequency;
		const uint8 *data;
		int size;
		int volumeStep;
		int stepCount;
		int step;
		bool repeat;
		int volume;
	};

protected:

	void playSoundChannel(int channel, int frequency, const uint8 *data, int size, int volume);

	Audio::SoundHandle _channelsTable[NUM_CHANNELS];
	SoundChannel _soundChannelsTable[NUM_CHANNELS];
	Audio::SoundHandle _moduleHandle;
	Audio::AudioStream *_moduleStream;
};

extern Sound *g_sound;

} // End of namespace Cine

#endif /* CINE_SOUND_H_ */
