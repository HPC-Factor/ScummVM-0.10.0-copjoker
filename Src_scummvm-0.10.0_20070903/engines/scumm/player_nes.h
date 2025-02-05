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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/engines/scumm/player_nes.h $
 * $Id: player_nes.h 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

#ifndef SCUMM_PLAYER_NES_H
#define SCUMM_PLAYER_NES_H

#include "common/scummsys.h"
#include "scumm/music.h"
#include "sound/audiostream.h"
#include "sound/mixer.h"

namespace Scumm {

class ScummEngine;
namespace APUe {
	class APU;
}

static const int MAXVOLUME = 0x7F;
static const int NUMSLOTS = 3;
static const int NUMCHANS = 4;

/**
 * Scumm NES sound/music driver.
 */
class Player_NES : public Audio::AudioStream, public MusicEngine {
public:
	Player_NES(ScummEngine *scumm, Audio::Mixer *mixer);
	virtual ~Player_NES();

	virtual void setMusicVolume(int vol);
	virtual void startSound(int sound);
	virtual void stopSound(int sound);
	virtual void stopAllSounds();
	virtual int  getSoundStatus(int sound) const;

	// AudioStream API
	int readBuffer(int16 *buffer, const int numSamples);
	bool isStereo() const { return false; }
	bool endOfData() const { return false; }
	int getRate() const { return _sample_rate; }

private:

	void sound_play();
	void playSFX(int nr);
	void playMusic();
	byte fetchSoundByte(int nr);
	void chainCommand(int chan);
	void checkSilenceChannels(int chan);

	void APU_writeChannel(int chan, int offset, byte value);
	void APU_writeControl(byte value);
	byte APU_readStatus();

	void do_mix(int16 *buf, uint len);

	ScummEngine *_vm;
	Audio::Mixer *_mixer;
	Audio::SoundHandle _soundHandle;
	APUe::APU *_apu;
	int _sample_rate;
	int _samples_per_frame;
	int _current_sample;
	int _maxvol;

	struct slot {
		int framesleft;
		int id;
		int type;
		byte *data;
		int offset;
	} _slot[NUMSLOTS];

	struct mchan {
		int command;
		int framedelay;
		int pitch;
		int volume;
		int voldelta;
		int envflags;
		int cmdlock;
	} _mchan[NUMCHANS];

	bool isSFXplaying, wasSFXplaying;

	byte *dataStart;
	int numNotes;
	byte *auxData1;
	byte *auxData2;

	byte *soundptr;
};

} // End of namespace Scumm

#endif
