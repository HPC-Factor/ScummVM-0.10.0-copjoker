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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/engines/agos/sound.h $
 * $Id: sound.h 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

#ifndef AGOS_SOUND_H
#define AGOS_SOUND_H

#include "sound/mixer.h"
#include "agos/intern.h"
#include "common/str.h"

namespace AGOS {

class BaseSound;

class AGOSEngine;

class Sound {
private:
	AGOSEngine *_vm;

	Audio::Mixer *_mixer;

	BaseSound *_voice;
	BaseSound *_effects;

	bool _effectsPaused;
	bool _ambientPaused;
	bool _sfx5Paused;

	uint16 *_filenums;
	uint32 *_offsets;
	uint16 _lastVoiceFile;

	Audio::SoundHandle _voiceHandle;
	Audio::SoundHandle _effectsHandle;
	Audio::SoundHandle _ambientHandle;
	Audio::SoundHandle _sfx5Handle;

	bool _hasEffectsFile;
	bool _hasVoiceFile;
	uint _ambientPlaying;

public:
	Sound(AGOSEngine *vm, const GameSpecificSettings *gss, Audio::Mixer *mixer);
	~Sound();

	void loadVoiceFile(const GameSpecificSettings *gss);
	void loadSfxFile(const GameSpecificSettings *gss);

	void readSfxFile(const char *filename);
	void loadSfxTable(Common::File *gameFile, uint32 base);
	void readVoiceFile(const char *filename);

	void playVoice(uint sound);
	void playEffects(uint sound);
	void playAmbient(uint sound);

	// Elvira 1/2 and Waxworks specific
	void playRawData(byte *soundData, uint sound, uint size);

	// Feeble Files specific
	void playAmbientData(byte *soundData, uint sound, uint pan, uint vol);
	void playSfxData(byte *soundData, uint sound, uint pan, uint vol);
	void playSfx5Data(byte *soundData, uint sound, uint pan, uint vol);
	void playSoundData(Audio::SoundHandle *handle, byte *soundData, uint sound, int pan = 0, int vol = 0, bool loop = false);
	void playVoiceData(byte *soundData, uint sound);
	void switchVoiceFile(const GameSpecificSettings *gss, uint disc);

	bool hasVoice() const;
	bool isVoiceActive() const;
	void stopAllSfx();
	void stopSfx5();
	void stopVoice();
	void stopAll();
	void effectsPause(bool b);
	void ambientPause(bool b);
};

} // End of namespace AGOS

#endif
