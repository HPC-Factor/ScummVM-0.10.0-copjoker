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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/engines/scumm/he/sound_he.cpp $
 * $Id: sound_he.cpp 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

#include "common/stdafx.h"
#include "scumm/actor.h"
#include "scumm/file.h"
#include "scumm/imuse/imuse.h"
#include "scumm/scumm.h"
#include "scumm/he/sound_he.h"
#include "scumm/he/intern_he.h"
#include "scumm/util.h"

#include "common/config-manager.h"
#include "common/timer.h"
#include "common/util.h"

#include "sound/adpcm.h"
#include "sound/audiocd.h"
#include "sound/flac.h"
#include "sound/mididrv.h"
#include "sound/mixer.h"
#include "sound/mp3.h"
#include "sound/voc.h"
#include "sound/vorbis.h"
#include "sound/wave.h"

namespace Scumm {

SoundHE::SoundHE(ScummEngine *parent, Audio::Mixer *mixer)
	:
	Sound(parent, mixer),
	_vm((ScummEngine_v60he *)parent),
	_overrideFreq(0),
	_heMusic(0),
	_heMusicTracks(0) {

	memset(_heChannel, 0, sizeof(_heChannel));
}	

SoundHE::~SoundHE() {
	free(_heMusic);
}

void SoundHE::addSoundToQueue(int sound, int heOffset, int heChannel, int heFlags) {
	if (_vm->VAR_LAST_SOUND != 0xFF)
		_vm->VAR(_vm->VAR_LAST_SOUND) = sound;

	if (heFlags & 16) {
		playHESound(sound, heOffset, heChannel, heFlags);
		return;
	}

	Sound::addSoundToQueue(sound, heOffset, heChannel, heFlags);
}

void SoundHE::addSoundToQueue2(int sound, int heOffset, int heChannel, int heFlags) {
	int i = _soundQue2Pos;
	while (i--) {
		if (_soundQue2[i].sound == sound && !(heFlags & 2))
			return;
	}
	
	Sound::addSoundToQueue2(sound, heOffset, heChannel, heFlags);
}

void SoundHE::processSoundQueues() {
	int snd, heOffset, heChannel, heFlags;

	if (_vm->_game.heversion >= 72) {
		for (int i = 0; i <_soundQue2Pos; i++) {
			snd = _soundQue2[i].sound;
			heOffset = _soundQue2[i].offset;
			heChannel = _soundQue2[i].channel;
			heFlags = _soundQue2[i].flags;
			if (snd)
				playHESound(snd, heOffset, heChannel, heFlags);
		}
		_soundQue2Pos = 0;
	} else {
		while (_soundQue2Pos) {
			_soundQue2Pos--;
			snd = _soundQue2[_soundQue2Pos].sound;
			heOffset = _soundQue2[_soundQue2Pos].offset;
			heChannel = _soundQue2[_soundQue2Pos].channel;
			heFlags = _soundQue2[_soundQue2Pos].flags;
			if (snd)
				playHESound(snd, heOffset, heChannel, heFlags);
		}
	}

	Sound::processSoundQueues();
}

int SoundHE::isSoundRunning(int sound) const {
	if (_vm->_game.heversion >= 70) {
		if (sound >= 10000) {
			return _mixer->getSoundID(_heSoundChannels[sound - 10000]);
		}
	} else if (_vm->_game.heversion >= 60) {
		if (sound == -2) {
			sound = _heChannel[0].sound;
		} else if (sound == -1) {
			sound = _currentMusic;
		}
	}

	if (_mixer->isSoundIDActive(sound))
		return sound;

	if (isSoundInQueue(sound))
		return sound;

	if (_vm->_musicEngine &&_vm->_musicEngine->getSoundStatus(sound))
		return sound;

	return 0;
}

void SoundHE::stopSound(int sound) {
	if (_vm->_game.heversion >= 70) {
		if ( sound >= 10000) {
			stopSoundChannel(sound - 10000);
		}
	} else if (_vm->_game.heversion >= 60) {
		if (sound == -2) {
			sound = _heChannel[0].sound;
		} else if (sound == -1) {
			sound = _currentMusic;
		}
	}

	Sound::stopSound(sound);

	for (int i = 0; i < ARRAYSIZE(_heChannel); i++) {
		if (_heChannel[i].sound == sound) {
			_heChannel[i].sound = 0;
			_heChannel[i].priority = 0;
			_heChannel[i].timer = 0;
			_heChannel[i].sbngBlock = 0;
			_heChannel[i].codeOffs = 0;
			memset(_heChannel[i].soundVars, 0, sizeof(_heChannel[i].soundVars));
		}
	}

	if (_vm->_game.heversion >= 70 && sound == 1) {
		_vm->_haveMsg = 3;
		_vm->_talkDelay = 0;
	}
}

void SoundHE::stopAllSounds() {
	// Clear sound channels for HE games
	memset(_heChannel, 0, sizeof(_heChannel));

	Sound::stopAllSounds();
}

void SoundHE::setupSound() {
	Sound::setupSound();

	if (_vm->_game.heversion >= 70) {
		setupHEMusicFile();
	}
}

void SoundHE::stopSoundChannel(int chan) {
	if (_heChannel[chan].sound == 1) {
		_vm->_haveMsg = 3;
		_vm->_talkDelay = 0;
	}

	_mixer->stopHandle(_heSoundChannels[chan]);

	_heChannel[chan].sound = 0;
	_heChannel[chan].priority = 0;
	_heChannel[chan].timer = 0;
	_heChannel[chan].sbngBlock = 0;
	_heChannel[chan].codeOffs = 0;
	memset(_heChannel[chan].soundVars, 0, sizeof(_heChannel[chan].soundVars));

	for (int i = 0; i < ARRAYSIZE(_soundQue2); i++) {
		if (_soundQue2[i].channel == chan) {
			_soundQue2[i].sound = 0;
			_soundQue2[i].offset = 0;
			_soundQue2[i].channel = 0;
			_soundQue2[i].flags = 0;
		}
	}
}

int SoundHE::findFreeSoundChannel() {
	int chan, min;

	min = _vm->VAR(_vm->VAR_RESERVED_SOUND_CHANNELS);
	if (min == 0) {
		_vm->VAR(_vm->VAR_RESERVED_SOUND_CHANNELS) = 8;
		return 1;
	}

	if (min < 8) {
		for (chan = min; chan < ARRAYSIZE(_heChannel); chan++) {
			if (_mixer->isSoundHandleActive(_heSoundChannels[chan]) == 0)
				return chan;
		}
	} else {
		return 1;
	}

	return min;
}

int SoundHE::isSoundCodeUsed(int sound) {
	int chan = -1;
	for (int i = 0; i < ARRAYSIZE(_heChannel); i ++) {
		if (_heChannel[i].sound == sound)
			chan = i;
	}

	if (_mixer->isSoundHandleActive(_heSoundChannels[chan]) && chan != -1) {
		return _heChannel[chan].sbngBlock;
	} else {
		return 0;
	}
}

int SoundHE::getSoundPos(int sound) {
	int chan = -1;
	for (int i = 0; i < ARRAYSIZE(_heChannel); i ++) {
		if (_heChannel[i].sound == sound)
			chan = i;
	}

	if (_mixer->isSoundHandleActive(_heSoundChannels[chan]) && chan != -1) {
		int time =  _vm->getHETimer(chan + 4) * 11025 / 1000;
		return time;
	} else {
		return 0;
	}
}

int SoundHE::getSoundVar(int sound, int var) {
	if (_vm->_game.heversion >= 90 && var == 26) {
		return isSoundCodeUsed(sound);
	}

	assertRange(0, var, 25, "sound variable");

	int chan = -1;
	for (int i = 0; i < ARRAYSIZE(_heChannel); i ++) {
		if (_heChannel[i].sound == sound)
			chan = i;
	}

	if (_mixer->isSoundHandleActive(_heSoundChannels[chan]) && chan != -1) {
		debug(5, "getSoundVar: sound %d var %d result %d", sound, var, _heChannel[chan].soundVars[var]);
		return _heChannel[chan].soundVars[var];
	} else {
		return 0;
	}
}

void SoundHE::setSoundVar(int sound, int var, int val) {
	assertRange(0, var, 25, "sound variable");

	int chan = -1;
	for (int i = 0; i < ARRAYSIZE(_heChannel); i ++) {
		if (_heChannel[i].sound == sound)
			chan = i;
	}

	if (chan != -1) {
		debug(5, "setSoundVar: sound %d var %d val %d", sound, var, val);
		_heChannel[chan].soundVars[var] = val;
	}
}

void SoundHE::setOverrideFreq(int freq) {
	_overrideFreq = freq;
}

void SoundHE::setupHEMusicFile() {
	int i, total_size;
	Common::File musicFile;
	Common::String buf(_vm->generateFilename(-4));

	if (musicFile.open(buf) == true) {
		musicFile.seek(4, SEEK_SET);
		total_size = musicFile.readUint32BE();
		musicFile.seek(16, SEEK_SET);
		_heMusicTracks = musicFile.readUint32LE();
		debug(5, "Total music tracks %d", _heMusicTracks);

		int musicStart = (_vm->_game.heversion >= 80) ? 56 : 20;
		musicFile.seek(musicStart, SEEK_SET);

		_heMusic = (HEMusic *)malloc((_heMusicTracks + 1) * sizeof(HEMusic));
		for (i = 0; i < _heMusicTracks; i++) {
			_heMusic[i].id = musicFile.readUint32LE();
			_heMusic[i].offset = musicFile.readUint32LE();
			_heMusic[i].size = musicFile.readUint32LE();

			if (_vm->_game.heversion >= 80) {
				musicFile.seek(+9, SEEK_CUR);
			} else {
				musicFile.seek(+13, SEEK_CUR);
			}
		}

		musicFile.close();
	}
}

bool SoundHE::getHEMusicDetails(int id, int &musicOffs, int &musicSize) {
	int i;

	for (i = 0; i < _heMusicTracks; i++) {
		if (_heMusic[i].id == id) {
			musicOffs = _heMusic[i].offset;
			musicSize = _heMusic[i].size;
			return 1;
		}
	}

	return 0;
}

void SoundHE::processSoundCode() {
	byte *codePtr;
	int chan, tmr, size, time;

	for (chan = 0; chan < ARRAYSIZE(_heChannel); chan++) {
		if (_heChannel[chan].sound == 0) {
			continue;
		}

		if (_heChannel[chan].codeOffs == -1) {
			continue;
		}

		tmr = _vm->getHETimer(chan + 4) * 11025 / 1000;
		tmr += _vm->VAR(_vm->VAR_SOUNDCODE_TMR);
		if (tmr < 0)
			tmr = 0;

		if (_heChannel[chan].sound > _vm->_numSounds) {
			codePtr = _vm->getResourceAddress(rtSpoolBuffer, chan);
		} else {
			codePtr = _vm->getResourceAddress(rtSound, _heChannel[chan].sound);
		}
		assert(codePtr);
		codePtr += _heChannel[chan].codeOffs;

		while (1) {
			size = READ_LE_UINT16(codePtr);
			time = READ_LE_UINT32(codePtr + 2);

			if (size == 0) {
				_heChannel[chan].codeOffs = -1;
				break;
			}

			debug(5, "Channel %d Timer %d Time %d", chan, tmr, time);
			if (time >= tmr)
				break;

			processSoundOpcodes(_heChannel[chan].sound, codePtr + 6, _heChannel[chan].soundVars);

			codePtr += size;
			_heChannel[chan].codeOffs += size;
		}
	}

	for (chan = 0; chan < ARRAYSIZE(_heChannel); chan++) {
		if (_heChannel[chan].sound == 0)
			continue;

		if (_heChannel[chan].timer == 0)
			continue;

		if (_vm->getHETimer(chan + 4) > _heChannel[chan].timer) {
			if (_heChannel[chan].sound == 1) {
				_vm->stopTalk();
			}

			_heChannel[chan].sound = 0;
			_heChannel[chan].priority = 0;
			_heChannel[chan].timer = 0;
			_heChannel[chan].sbngBlock = 0;
			_heChannel[chan].codeOffs = 0;
			_heChannel[chan].soundVars[0] = 0;
		}
	}
}

void SoundHE::processSoundOpcodes(int sound, byte *codePtr, int *soundVars) {
	int arg, opcode, var, val;

	while (READ_LE_UINT16(codePtr) != 0) {
		codePtr += 2;
		opcode = READ_LE_UINT16(codePtr); codePtr += 2;
		opcode = (opcode & 0xFFF) >> 4;
		arg = opcode & 3;
		opcode &= ~3;
		debug(5, "processSoundOpcodes: sound %d opcode %d", sound, opcode);
		switch (opcode) {
		case 0: // Continue
			break;
		case 16: // Set talk state
			val = READ_LE_UINT16(codePtr); codePtr += 2;
			setSoundVar(sound, 19, val);
			break;
		case 32: // Set var
			var = READ_LE_UINT16(codePtr); codePtr += 2;
			val = READ_LE_UINT16(codePtr); codePtr += 2;
			if (arg == 2) {
				val = getSoundVar(sound, val);
			}
			setSoundVar(sound, var, val);
			break;
		case 48: // Add
			var = READ_LE_UINT16(codePtr); codePtr += 2;
			val = READ_LE_UINT16(codePtr); codePtr += 2;
			if (arg == 2) {
				val = getSoundVar(sound, val);
			}
			val = getSoundVar(sound, var) + val;
			setSoundVar(sound, var, val);
			break;
		case 56: // Subtract
			var = READ_LE_UINT16(codePtr); codePtr += 2;
			val = READ_LE_UINT16(codePtr); codePtr += 2;
			if (arg == 2) {
				val = getSoundVar(sound, val);
			}
			val = getSoundVar(sound, var) - val;
			setSoundVar(sound, var, val);
			break;
		case 64: // Multiple
			var = READ_LE_UINT16(codePtr); codePtr += 2;
			val = READ_LE_UINT16(codePtr); codePtr += 2;
			if (arg == 2) {
				val = getSoundVar(sound, val);
			}
			val = getSoundVar(sound, var) * val;
			setSoundVar(sound, var, val);
			break;
		case 80: // Divide
			var = READ_LE_UINT16(codePtr); codePtr += 2;
			val = READ_LE_UINT16(codePtr); codePtr += 2;
			if (arg == 2) {
				val = getSoundVar(sound, val);
			}
			val = getSoundVar(sound, var) / val;
			setSoundVar(sound, var, val);
			break;
		case 96: // Increment
			var = READ_LE_UINT16(codePtr); codePtr += 2;
			val = getSoundVar(sound, var) + 1;
			setSoundVar(sound, var, val);
			break;
		case 104: // Decrement
			var = READ_LE_UINT16(codePtr); codePtr += 2;
			val = getSoundVar(sound, var) - 1;
			setSoundVar(sound, var, val);
			break;
		default:
			error("Illegal sound %d opcode %d", sound, opcode);
		}
	}
}

void SoundHE::playHESound(int soundID, int heOffset, int heChannel, int heFlags) {
	byte *ptr, *spoolPtr;
	int size = -1;
	int priority, rate;
	byte flags = Audio::Mixer::FLAG_UNSIGNED;

	Audio::Mixer::SoundType type = Audio::Mixer::kSFXSoundType;
	if (soundID > _vm->_numSounds)
		type = Audio::Mixer::kMusicSoundType;
	else if (soundID == 1)
		type = Audio::Mixer::kSpeechSoundType;


	if (heChannel == -1)
		heChannel = (_vm->VAR_RESERVED_SOUND_CHANNELS != 0xFF) ? findFreeSoundChannel() : 1;

	debug(5,"playHESound: soundID %d heOffset %d heChannel %d heFlags %d", soundID, heOffset, heChannel, heFlags);

	if (soundID >= 10000) {
		// Special codes, used in pjgames
		return;
	}

	if (soundID > _vm->_numSounds) {
		int music_offs;
		Common::File musicFile;
		Common::String buf(_vm->generateFilename(-4));

		if (musicFile.open(buf) == false) {
			warning("playHESound: Can't open music file %s", buf.c_str());
			return;
		}
		if (!getHEMusicDetails(soundID, music_offs, size)) {
			debug(0, "playHESound: musicID %d not found", soundID);
			return;
		}

		musicFile.seek(music_offs, SEEK_SET);

		_mixer->stopHandle(_heSoundChannels[heChannel]);
		spoolPtr = _vm->_res->createResource(rtSpoolBuffer, heChannel, size);
		assert(spoolPtr);
		musicFile.read(spoolPtr, size);
		musicFile.close();

		if (_vm->_game.heversion == 70) {
			_mixer->playRaw(type, &_heSoundChannels[heChannel], spoolPtr, size, 11025, flags, soundID);
			return;
		}
	}

	if (soundID > _vm->_numSounds) {
		ptr = _vm->getResourceAddress(rtSpoolBuffer, heChannel);
	} else {
		ptr = _vm->getResourceAddress(rtSound, soundID);
	}

	if (!ptr) {
		return;
	}

	// Support for sound in later Backyard sports games
	if (READ_BE_UINT32(ptr) == MKID_BE('RIFF') || READ_BE_UINT32(ptr) == MKID_BE('WSOU')) {
		uint16 compType;
		int blockAlign;
		char *sound;

		if (READ_BE_UINT32(ptr) == MKID_BE('WSOU'))
			ptr += 8;

		size = READ_LE_UINT32(ptr + 4);
		Common::MemoryReadStream stream(ptr, size);

		if (!Audio::loadWAVFromStream(stream, size, rate, flags, &compType, &blockAlign)) {
			error("playHESound: Not a valid WAV file (%d)", soundID);
		}

		if (compType == 17) {
			Audio::AudioStream *voxStream = Audio::makeADPCMStream(&stream, size, Audio::kADPCMMSIma, rate, (flags & Audio::Mixer::FLAG_STEREO) ? 2 : 1, blockAlign);

			sound = (char *)malloc(size * 4);
			size = voxStream->readBuffer((int16*)sound, size * 2);
			size *= 2; // 16bits.
			delete voxStream;
		} else {
			// Allocate a sound buffer, copy the data into it, and play
			sound = (char *)malloc(size);
			memcpy(sound, ptr + stream.pos(), size);
		}

		flags |= Audio::Mixer::FLAG_AUTOFREE;
		// TODO: Extra sound flags
		if (heFlags & 1) {
			flags |= Audio::Mixer::FLAG_LOOP;
		}

		_mixer->stopHandle(_heSoundChannels[heChannel]);
		_mixer->playRaw(type, &_heSoundChannels[heChannel], sound, size, rate, flags, soundID);
	}
	// Support for sound in Humongous Entertainment games
	else if (READ_BE_UINT32(ptr) == MKID_BE('DIGI') || READ_BE_UINT32(ptr) == MKID_BE('TALK')) {
		byte *sndPtr = ptr;

		priority = (soundID > _vm->_numSounds) ? 255 : *(ptr + 18);
		rate = READ_LE_UINT16(ptr + 22);

		// Skip DIGI/TALK (8) and HSHD (24) blocks
		ptr += 32;

		if (_mixer->isSoundHandleActive(_heSoundChannels[heChannel])) {
			int curSnd = _heChannel[heChannel].sound;
			if (curSnd == 1 && soundID != 1)
				return;
			if (curSnd != 0 && curSnd != 1 && soundID != 1 && _heChannel[heChannel].priority > priority)
				return;
		}

		int codeOffs = -1;
		if (READ_BE_UINT32(ptr) == MKID_BE('SBNG')) {
			codeOffs = ptr - sndPtr + 8;
			ptr += READ_BE_UINT32(ptr + 4);
		}

		assert(READ_BE_UINT32(ptr) == MKID_BE('SDAT'));
		size = READ_BE_UINT32(ptr + 4) - 8;
		if (heOffset < 0 || heOffset > size) {
			// Occurs when making fireworks in puttmoon
			heOffset = 0;
		}
		size -= heOffset;

		if (_overrideFreq) {
			// Used by the piano in Fatty Bear's Birthday Surprise
			rate = _overrideFreq;
			_overrideFreq = 0;
		}

		_vm->setHETimer(heChannel + 4);
		_heChannel[heChannel].sound = soundID;
		_heChannel[heChannel].priority = priority;
		_heChannel[heChannel].sbngBlock = (codeOffs != -1) ? 1 : 0;
		_heChannel[heChannel].codeOffs = codeOffs;
		memset(_heChannel[heChannel].soundVars, 0, sizeof(_heChannel[heChannel].soundVars));

		// TODO: Extra sound flags
		if (heFlags & 1) {
			flags |= Audio::Mixer::FLAG_LOOP;
			_heChannel[heChannel].timer = 0;
		} else {
			_heChannel[heChannel].timer = size * 1000 / rate;
		}

		_mixer->stopHandle(_heSoundChannels[heChannel]);
		_mixer->playRaw(type, &_heSoundChannels[heChannel], ptr + heOffset + 8, size, rate, flags, soundID);

	}
	// Support for PCM music in 3DO versions of Humongous Entertainment games
	else if (READ_BE_UINT32(ptr) == MKID_BE('MRAW')) {
		priority = *(ptr + 18);
		rate = READ_LE_UINT16(ptr + 22);

		// Skip DIGI (8) and HSHD (24) blocks
		ptr += 32;

		assert(READ_BE_UINT32(ptr) == MKID_BE('SDAT'));
		size = READ_BE_UINT32(ptr + 4) - 8;

		flags = Audio::Mixer::FLAG_AUTOFREE;

		_mixer->stopID(_currentMusic);
		_currentMusic = soundID;
		_mixer->playRaw(Audio::Mixer::kMusicSoundType, NULL, ptr + 8, size, rate, flags, soundID);
	}
	else if (READ_BE_UINT32(ptr) == MKID_BE('MIDI')) {
		if (_vm->_imuse) {
			_vm->_imuse->stopSound(_currentMusic);
			_currentMusic = soundID;
			_vm->_imuse->startSound(soundID);
		}
	}
}

void SoundHE::startHETalkSound(uint32 offset) {
	byte *ptr;
	int32 size;

	if (ConfMan.getBool("speech_mute"))
		return;

	assert(_sfxFile);
	if (!_sfxFile->isOpen()) {
		// This happens in the Pajama Sam's Lost & Found demo, on the
		// main menu screen, so don't make it a fatal error.
		warning("startHETalkSound: Speech file is not open");
		return;
	}

	_sfxMode |= 2;
	_vm->_res->nukeResource(rtSound, 1);

	_sfxFile->seek(offset + 4, SEEK_SET);
	 size = _sfxFile->readUint32BE();
	_sfxFile->seek(offset, SEEK_SET);

	_vm->_res->createResource(rtSound, 1, size);
	ptr = _vm->getResourceAddress(rtSound, 1);
	_sfxFile->read(ptr, size);

	int channel = (_vm->VAR_TALK_CHANNEL != 0xFF) ? _vm->VAR(_vm->VAR_TALK_CHANNEL) : 0;
	addSoundToQueue2(1, 0, channel, 0);
}

} // End of namespace Scumm
