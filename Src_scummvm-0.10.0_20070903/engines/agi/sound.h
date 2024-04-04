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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/engines/agi/sound.h $
 * $Id: sound.h 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

#ifndef AGI_SOUND_H
#define AGI_SOUND_H

#include "sound/audiostream.h"
#include "sound/mixer.h"

namespace Agi {

#define BUFFER_SIZE	410

#define SOUND_EMU_NONE	0
#define SOUND_EMU_PC	1
#define SOUND_EMU_TANDY	2
#define SOUND_EMU_MAC	3
#define SOUND_EMU_AMIGA	4

#define SOUND_PLAYING   0x01
#define WAVEFORM_SIZE   64
#define ENV_ATTACK	10000		/**< envelope attack rate */
#define ENV_DECAY       1000		/**< envelope decay rate */
#define ENV_SUSTAIN     100		/**< envelope sustain level */
#define ENV_RELEASE	7500		/**< envelope release rate */
#define NUM_CHANNELS    7		/**< number of sound channels */

/**
 * AGI sound resource structure.
 */
struct AgiSound {
	uint32 flen;		/**< size of raw data */
	uint8 *rdata;		/**< raw sound data */
	uint8 flags;		/**< sound flags */
	uint16 type;		/**< sound resource type */
};

#include "common/pack-start.h"

/**
 * AGI sound note structure.
 */
struct AgiNote {
	uint8 durLo;		/**< LSB of note duration */
	uint8 durHi;			/**< MSB of note duration */
	uint8 frq0;			/**< LSB of note frequency */
	uint8 frq1;			/**< MSB of note frequency */
	uint8 vol;			/**< note volume */
};

#include "common/pack-end.h"

/**
 * AGI engine sound channel structure.
 */
struct ChannelInfo {
#define AGI_SOUND_SAMPLE	0x0001
#define AGI_SOUND_MIDI		0x0002
#define AGI_SOUND_4CHN		0x0008
	uint32 type;
	struct AgiNote *ptr;
	int16 *ins;
	int32 size;
	uint32 phase;
#define AGI_SOUND_LOOP		0x0001
#define AGI_SOUND_ENVELOPE	0x0002
	uint32 flags;
#define AGI_SOUND_ENV_ATTACK	3
#define AGI_SOUND_ENV_DECAY	2
#define AGI_SOUND_ENV_SUSTAIN	1
#define AGI_SOUND_ENV_RELEASE	0
	uint32 adsr;
	int32 timer;
	uint32 end;
	uint32 freq;
	uint32 vol;
	uint32 env;
};

class AgiEngine;

class SoundMgr : public Audio::AudioStream {
	AgiEngine *_vm;

public:
	SoundMgr(AgiEngine *agi, Audio::Mixer *pMixer);
	~SoundMgr();
	virtual void setVolume(uint8 volume);

	// AudioStream API
	int readBuffer(int16 *buffer, const int numSamples) {
		premixerCall(buffer, numSamples / 2);
		return numSamples;
	}

	bool isStereo() const {
		return false;
	}

	bool endOfData() const {
		return false;
	}

	int getRate() const {
		// FIXME: Ideally, we should use _sampleRate.
		return 22050;
	}

private:
	Audio::Mixer *_mixer;
	Audio::SoundHandle _soundHandle;
	uint32 _sampleRate;

	void premixerCall(int16 *buf, uint len);

public:

	void decodeSound(int);
	void unloadSound(int);
	void playSound();
	int initSound();
	void deinitSound();
	void startSound(int, int);
	void stopSound();
	void stopNote(int i);
	void playNote(int i, int freq, int vol);
	void playAgiSound();
	uint32 mixSound();
	int loadInstruments(char *fname);
#ifdef USE_IIGS_SOUND
	void playMidiSound();
	void playSampleSound();
#endif
};

} // End of namespace Agi

#endif /* AGI_SOUND_H */
