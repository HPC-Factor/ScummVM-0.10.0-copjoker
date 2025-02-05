/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1994-1998 Revolution Software Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/engines/sword2/sound.cpp $
 * $Id: sound.cpp 27030 2007-05-31 20:28:29Z fingolfin $
 */

// ---------------------------------------------------------------------------
//								BROKEN SWORD 2
//
//	SOUND.CPP	Contains the sound engine, fx & music functions
//			Some very 'sound' code in here ;)
//
//	(16Dec96 JEL)
//
// ---------------------------------------------------------------------------

#include "common/stdafx.h"
#include "common/file.h"
#include "common/system.h"

#include "sword2/sword2.h"
#include "sword2/defs.h"
#include "sword2/header.h"
#include "sword2/logic.h"
#include "sword2/resman.h"
#include "sword2/sound.h"

#include "sound/wave.h"

namespace Sword2 {

Sound::Sound(Sword2Engine *vm) {
	int i;

	_vm = vm;

	for (i = 0; i < FXQ_LENGTH; i++)
		_fxQueue[i].resource = 0;

	for (i = 0; i < MAXMUS; i++) {
		_music[i] = NULL;

		_musicFile[i].idxTab = NULL;
		_musicFile[i].idxLen = 0;
		_musicFile[i].fileSize = 0;
		_musicFile[i].fileType = 0;
		_musicFile[i].inUse = false;

		_speechFile[i].idxTab = NULL;
		_speechFile[i].idxLen = 0;
		_speechFile[i].fileSize = 0;
		_speechFile[i].fileType = 0;
		_speechFile[i].inUse = false;
	}

	_speechPaused = false;
	_musicPaused = false;
	_fxPaused = false;

	_speechMuted = false;
	_musicMuted = false;
	_fxMuted = false;

	_reverseStereo = false;

	_loopingMusicId = 0;

	_mixBuffer = NULL;
	_mixBufferLen = 0;

	_vm->_mixer->playInputStream(Audio::Mixer::kMusicSoundType, &_mixerSoundHandle, this, -1, Audio::Mixer::kMaxChannelVolume, 0, false, true);
}

Sound::~Sound() {
	_vm->_mixer->stopHandle(_mixerSoundHandle);

	clearFxQueue(true);
	stopMusic(true);
	stopSpeech();

	free(_mixBuffer);

	for (int i = 0; i < MAXMUS; i++) {
		if (_musicFile[i].file.isOpen())
			_musicFile[i].file.close();
		if (_speechFile[i].file.isOpen())
			_speechFile[i].file.close();

		free(_musicFile[i].idxTab);
		free(_speechFile[i].idxTab);
	}
}

void Sound::setReverseStereo(bool reverse) {
	if (reverse != _reverseStereo) {
		_reverseStereo = reverse;

		for (int i = 0; i < FXQ_LENGTH; i++) {
			if (!_fxQueue[i].resource)
				continue;

			_fxQueue[i].pan = -_fxQueue[i].pan;
			_vm->_mixer->setChannelBalance(_fxQueue[i].handle, _fxQueue[i].pan);
		}
	}
}

/**
 * Stop all sounds, close their resources and clear the FX queue. This is used
 * when going from one room to another, among other things.
 */

void Sound::clearFxQueue(bool killMovieSounds) {
	for (int i = 0; i < FXQ_LENGTH; i++) {
		if (_fxQueue[i].resource) {
			stopFx(i);
		}
	}

	// We aren't just going to change rooms or anything like that, we are
	// killing off resources (e.g. when restoring or restarting). We need
	// to also kill any movie lead-in/out sounds.

	if (killMovieSounds) {
		_vm->_mixer->stopHandle(_leadInHandle);
		_vm->_mixer->stopHandle(_leadOutHandle);
	}
}

/**
 * Process the FX queue. This function is called once every game cycle.
 */

void Sound::processFxQueue() {
	for (int i = 0; i < FXQ_LENGTH; i++) {
		if (!_fxQueue[i].resource)
			continue;

		switch (_fxQueue[i].type) {
		case FX_RANDOM:
			// 1 in 'delay' chance of this fx occurring
			if (_vm->_rnd.getRandomNumber(_fxQueue[i].delay) == 0)
				playFx(&_fxQueue[i]);
			break;
		case FX_SPOT:
			if (_fxQueue[i].delay)
				_fxQueue[i].delay--;
			else {
				playFx(&_fxQueue[i]);
				_fxQueue[i].type = FX_SPOT2;
			}
			break;
		case FX_LOOP:
			playFx(&_fxQueue[i]);
			_fxQueue[i].type = FX_LOOPING;
			break;
		case FX_SPOT2:
			// Once the FX has finished remove it from the queue.
			if (!_vm->_mixer->isSoundHandleActive(_fxQueue[i].handle)) {
				_vm->_resman->closeResource(_fxQueue[i].resource);
				_fxQueue[i].resource = 0;
			}
			break;
		case FX_LOOPING:
			// Once the looped FX has started we can ignore it,
			// but we can't close it since the WAV data is in use.
			break;
		}
	}
}

/**
 * This function is used by the cutscene player to play the movie lead-in/out.
 * @param res The sound resource to play
 * @param type Either kLeadInSound or kLeadOutSound
 */

void Sound::playMovieSound(int32 res, int type) {
	Audio::SoundHandle *handle;

	if (type == kLeadInSound)
		handle = &_leadInHandle;
	else
		handle = &_leadOutHandle;

	if (_vm->_mixer->isSoundHandleActive(*handle)) {
		_vm->_mixer->stopHandle(*handle);
	}

	byte *data = _vm->_resman->openResource(res);
	uint32 len = _vm->_resman->fetchLen(res) - ResHeader::size();

	assert(_vm->_resman->fetchType(data) == WAV_FILE);
	data += ResHeader::size();

	_vm->_sound->playFx(handle, data, len, Audio::Mixer::kMaxChannelVolume, 0, false, Audio::Mixer::kMusicSoundType);
}

/**
 * Queue a sound effect for playing later.
 * @param res the sound resource number
 * @param type the type of sound effect
 * @param delay when to play the sound effect
 * @param volume the sound effect volume (0 through 16)
 * @param pan the sound effect panning (-16 through 16)
 */

void Sound::queueFx(int32 res, int32 type, int32 delay, int32 volume, int32 pan) {
	if (_vm->_wantSfxDebug) {
		const char *typeStr;

		switch (type) {
		case FX_SPOT:
			typeStr = "SPOT";
			break;
		case FX_LOOP:
			typeStr = "LOOPED";
			break;
		case FX_RANDOM:
			typeStr = "RANDOM";
			break;
		default:
			typeStr = "INVALID";
			break;
		}

		debug(0, "SFX (sample=\"%s\", vol=%d, pan=%d, delay=%d, type=%s)", _vm->_resman->fetchName(res), volume, pan, delay, typeStr);
	}

	for (int i = 0; i < FXQ_LENGTH; i++) {
		if (!_fxQueue[i].resource) {
			byte *data = _vm->_resman->openResource(res);

			assert(_vm->_resman->fetchType(data) == WAV_FILE);

			uint32 len = _vm->_resman->fetchLen(res) - ResHeader::size();

			if (type == FX_RANDOM) {
				// For spot effects and loops the delay is the
				// number of frames to wait. For random
				// effects, however, it's the average number of
				// seconds between playing the sound, so we
				// have to multiply by the frame rate.
				delay *= FRAMES_PER_SECOND;
			}

			volume = (volume * Audio::Mixer::kMaxChannelVolume) / 16;
			pan = (pan * 127) / 16;

			if (isReverseStereo())
				pan = -pan;

			_fxQueue[i].resource = res;
			_fxQueue[i].data = data + ResHeader::size();
			_fxQueue[i].len = len;
			_fxQueue[i].delay = delay;
			_fxQueue[i].volume = volume;
			_fxQueue[i].pan = pan;
			_fxQueue[i].type = type;

			// Keep track of the index in the loop so that
			// fnStopFx() can be used later to kill this sound.
			// Mainly for FX_LOOP and FX_RANDOM.

			_vm->_logic->writeVar(RESULT, i);
			return;
		}
	}

	warning("No free slot in FX queue");
}

int32 Sound::playFx(FxQueueEntry *fx) {
	return playFx(&fx->handle, fx->data, fx->len, fx->volume, fx->pan, (fx->type == FX_LOOP), Audio::Mixer::kSFXSoundType);
}

int32 Sound::playFx(Audio::SoundHandle *handle, byte *data, uint32 len, uint8 vol, int8 pan, bool loop, Audio::Mixer::SoundType soundType) {
	if (_fxMuted)
		return RD_OK;

	if (_vm->_mixer->isSoundHandleActive(*handle))
		return RDERR_FXALREADYOPEN;

	Common::MemoryReadStream stream(data, len);
	int rate, size;
	byte flags;

	if (!Audio::loadWAVFromStream(stream, size, rate, flags)) {
		warning("playFX: Not a valid WAV file");
		return RDERR_INVALIDWAV;
	}

	if (isReverseStereo())
		flags |= Audio::Mixer::FLAG_REVERSE_STEREO;

	if (loop)
		flags |= Audio::Mixer::FLAG_LOOP;

	_vm->_mixer->playRaw(soundType, handle, data + stream.pos(), size, rate, flags, -1, vol, pan, 0, 0);
	return RD_OK;
}

/**
 * This function closes a sound effect which has been previously opened for
 * playing. Sound effects must be closed when they are finished with, otherwise
 * you will run out of sound effect buffers.
 * @param i the index of the sound to close
 */

int32 Sound::stopFx(int32 i) {
	if (!_fxQueue[i].resource)
		return RDERR_FXNOTOPEN;

	_vm->_mixer->stopHandle(_fxQueue[i].handle);

	_vm->_resman->closeResource(_fxQueue[i].resource);
	_fxQueue[i].resource = 0;
	return RD_OK;
}

void Sound::pauseAllSound() {
	pauseMusic();
	pauseSpeech();
	pauseFx();
}

void Sound::unpauseAllSound() {
	unpauseMusic();
	unpauseSpeech();
	unpauseFx();
}

} // End of namespace Sword2
