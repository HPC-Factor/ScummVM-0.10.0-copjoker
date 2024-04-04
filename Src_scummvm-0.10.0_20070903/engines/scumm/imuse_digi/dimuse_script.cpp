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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/engines/scumm/imuse_digi/dimuse_script.cpp $
 * $Id: dimuse_script.cpp 27039 2007-06-01 11:22:49Z aquadran $
 */

#include "common/stdafx.h"
#include "common/system.h"
#include "common/timer.h"

#include "scumm/actor.h"
#include "scumm/intern.h"
#include "scumm/sound.h"
#include "scumm/imuse_digi/dimuse.h"
#include "scumm/imuse_digi/dimuse_bndmgr.h"

#include "sound/audiostream.h"
#include "sound/mixer.h"

namespace Scumm {

void IMuseDigital::parseScriptCmds(int cmd, int b, int c, int d, int e, int f, int g, int h) {
	int soundId = b;
	int sub_cmd = c;

	if (!cmd)
		return;

	switch (cmd) {
	case 10: // ImuseStopAllSounds
		stopAllSounds();
		break;
	case 12: // ImuseSetParam
		switch (sub_cmd) {
		case 0x400: // select group volume
			selectVolumeGroup(soundId, d);
			break;
		case 0x500: // set priority
			setPriority(soundId, d);
			break;
		case 0x600: // set volume
			setVolume(soundId, d);
			break;
		case 0x700: // set pan
			setPan(soundId, d);
			break;
		default:
			warning("IMuseDigital::doCommand SetParam DEFAULT command %d", sub_cmd);
			break;
		}
		break;
	case 14: // ImuseFadeParam
		switch (sub_cmd) {
		case 0x600: // set volume fading
			if ((d != 0) && (e == 0))
				setVolume(soundId, d);
			else if ((d == 0) && (e == 0))
				stopSound(soundId);
			else
				setFade(soundId, d, e);
			break;
		default:
			warning("IMuseDigital::doCommand FadeParam DEFAULT sub command %d", sub_cmd);
			break;
		}
		break;
	case 25: // ImuseStartStream
		debug(5, "ImuseStartStream (%d, %d, %d)", soundId, c, d);
		break;
	case 26: // ImuseSwitchStream
		debug(5, "ImuseSwitchStream (%d, %d, %d, %d, %d)", soundId, c, d, e, f);
		break;
	case 0x1000: // ImuseSetState
		debug(5, "ImuseSetState (%d)", b);
		if ((_vm->_game.id == GID_DIG) && (_vm->_game.features & GF_DEMO)) {
			if (b == 1) {
				fadeOutMusic(200);
				startMusic(1, 127);
			} else {
				if (getSoundStatus(2) == 0) {
					fadeOutMusic(200);
					startMusic(2, 127);
				}
			}
		} else if ((_vm->_game.id == GID_CMI) && (_vm->_game.features & GF_DEMO)) {
			fadeOutMusic(120);
			if (b == 2) {
				startMusic("in1.imx", 1100, 0, 127);
			} else if (b == 4) {
				startMusic("in2.imx", 1120, 0, 127);
			} else if (b == 8) {
				startMusic("out1.imx", 1140, 0, 127);
			} else if (b == 9) {
				startMusic("out2.imx", 1150, 0, 127);
			} else if (b == 16) {
				startMusic("gun.imx", 1210, 0, 127);
			} else {
				warning("imuse digital: set state unknown for cmi demo: %d, room: %d", b, _vm->_currentRoom);
			}
		} else if (_vm->_game.id == GID_DIG) {
			setDigMusicState(b);
		} else if (_vm->_game.id == GID_CMI) {
			setComiMusicState(b);
		} else if (_vm->_game.id == GID_FT) {
			setFtMusicState(b);
		}
		break;
	case 0x1001: // ImuseSetSequence
		debug(5, "ImuseSetSequence (%d)", b);
		if (_vm->_game.id == GID_DIG) {
			setDigMusicSequence(b);
		} else if (_vm->_game.id == GID_CMI) {
			setComiMusicSequence(b);
		} else if (_vm->_game.id == GID_FT) {
			setFtMusicSequence(b);
		}
		break;
	case 0x1002: // ImuseSetCuePoint
		debug(5, "ImuseSetCuePoint (%d)", b);
		if (_vm->_game.id == GID_FT) {
			setFtMusicCuePoint(b);
		}
		break;
	case 0x1003: // ImuseSetAttribute
		debug(5, "ImuseSetAttribute (%d, %d)", b, c);
		assert((_vm->_game.id == GID_DIG) || (_vm->_game.id == GID_FT));
		if (_vm->_game.id == GID_DIG) {
			_attributes[b] = c;
		}
		break;
	case 0x2000: // ImuseSetGroupSfxVolume
		debug(5, "ImuseSetGroupSFXVolume (%d)", b);
//		setGroupSfxVolume(b);
		break;
	case 0x2001: // ImuseSetGroupVoiceVolume
		debug(5, "ImuseSetGroupVoiceVolume (%d)", b);
//		setGroupVoiceVolume(b);
		break;
	case 0x2002: // ImuseSetGroupMusicVolume
		debug(5, "ImuseSetGroupMusicVolume (%d)", b);
//		setGroupMusicVolume(b);
		break;
	default:
		error("IMuseDigital::doCommand DEFAULT command %d", cmd);
	}
}

void IMuseDigital::flushTracks() {
	Common::StackLock lock(_mutex, "IMuseDigital::flushTracks()");
	debug(5, "flushTracks()");
	for (int l = 0; l < MAX_DIGITAL_TRACKS + MAX_DIGITAL_FADETRACKS; l++) {
		Track *track = _track[l];
		if (track->used && track->readyToRemove) {
			if (track->stream) {
				// Finalize the appendable stream
 				track->stream->finish();
 				// There might still be some data left in the buffers of the
 				// appendable stream. We play it nice and wait till all of it
 				// played.
				if (track->stream->endOfStream()) {
					_mixer->stopHandle(track->mixChanHandle);
					delete track->stream;
					track->stream = NULL;
					_sound->closeSound(track->soundHandle);
					track->soundHandle = NULL;
					track->used = false;
				}
			} else if (track->streamSou) {
				_mixer->stopHandle(track->mixChanHandle);
				delete track->streamSou;
				track->streamSou = NULL;
				track->used = false;
			}
		}
	}
}

void IMuseDigital::refreshScripts() {
	Common::StackLock lock(_mutex, "IMuseDigital::refreshScripts()");
	debug(5, "refreshScripts()");
	bool found = false;
	for (int l = 0; l < MAX_DIGITAL_TRACKS; l++) {
		Track *track = _track[l];
		if (track->used && !track->toBeRemoved && (track->volGroupId == IMUSE_VOLGRP_MUSIC)) {
			found = true;
		}
	}

	if (!found && (_curMusicSeq != 0)) {
		debug(5, "refreshScripts() Start Sequence");
		parseScriptCmds(0x1001, 0, 0, 0, 0, 0, 0, 0);
	}
}

void IMuseDigital::startVoice(int soundId, Audio::AudioStream *input) {
	debug(5, "startVoiceStream(%d)", soundId);
	startSound(soundId, "", 0, IMUSE_VOLGRP_VOICE, input, 0, 127, 127);
}

void IMuseDigital::startVoice(int soundId, const char *soundName) {
	debug(5, "startVoiceBundle(%s)", soundName);
	startSound(soundId, soundName, IMUSE_BUNDLE, IMUSE_VOLGRP_VOICE, NULL, 0, 127, 127);
}

void IMuseDigital::startMusic(int soundId, int volume) {
	debug(5, "startMusicResource(%d)", soundId);
	startSound(soundId, "", IMUSE_RESOURCE, IMUSE_VOLGRP_MUSIC, NULL, 0, volume, 126);
}

void IMuseDigital::startMusic(const char *soundName, int soundId, int hookId, int volume) {
	debug(5, "startMusicBundle(%s)", soundName);
	startSound(soundId, soundName, IMUSE_BUNDLE, IMUSE_VOLGRP_MUSIC, NULL, hookId, volume, 126);
}

void IMuseDigital::startSfx(int soundId, int priority) {
	debug(5, "startSfx(%d)", soundId);
	startSound(soundId, "", IMUSE_RESOURCE, IMUSE_VOLGRP_SFX, NULL, 0, 127, priority);
}

void IMuseDigital::getLipSync(int soundId, int syncId, int32 msPos, int32 &width, int32 &height) {
	int32 sync_size;
	byte *sync_ptr;

	msPos /= 16;
	if (msPos < 65536) {
		Common::StackLock lock(_mutex, "IMuseDigital::getLipSync()");
		for (int l = 0; l < MAX_DIGITAL_TRACKS; l++) {
			Track *track = _track[l];
			if ((track->soundId == soundId) && track->used && !track->toBeRemoved) {
				_sound->getSyncSizeAndPtrById(track->soundHandle, syncId, sync_size, &sync_ptr);
				if ((sync_size != 0) && (sync_ptr != NULL)) {
					sync_size /= 4;
					while (sync_size--) {
						if (READ_BE_UINT16(sync_ptr) >= msPos)
							break;
						sync_ptr += 4;
					}
					if (sync_size < 0)
						sync_ptr -= 4;
					else
						if (READ_BE_UINT16(sync_ptr) > msPos)
							sync_ptr -= 4;

					width = sync_ptr[2];
					height = sync_ptr[3];
					return;
				}
			}
		}
	}
}

int32 IMuseDigital::getPosInMs(int soundId) {
	Common::StackLock lock(_mutex, "IMuseDigital::getPosInMs()");
	for (int l = 0; l < MAX_DIGITAL_TRACKS; l++) {
		Track *track = _track[l];
		if ((track->soundId == soundId) && track->used && !track->toBeRemoved) {
			int32 pos = (5 * (track->dataOffset + track->regionOffset)) / (track->feedSize / 200);
			return pos;
		}
	}

	return 0;
}

int IMuseDigital::getSoundStatus(int sound) const {
	Common::StackLock lock(_mutex, "IMuseDigital::getSoundStatus()");
	debug(5, "IMuseDigital::getSoundStatus(%d)", sound);
	for (int l = 0; l < MAX_DIGITAL_TRACKS; l++) {
		Track *track = _track[l];
		if (track->soundId == sound) {
			if ((track->streamSou && _mixer->isSoundHandleActive(track->mixChanHandle)) ||
				(track->stream && track->used && !track->readyToRemove)) {
					return 1;
			}
		}
	}

	return 0;
}

void IMuseDigital::stopSound(int soundId) {
	Common::StackLock lock(_mutex, "IMuseDigital::stopSound()");
	debug(5, "IMuseDigital::stopSound(%d)", soundId);
	for (int l = 0; l < MAX_DIGITAL_TRACKS; l++) {
		Track *track = _track[l];
		if ((track->soundId == soundId) && track->used && !track->toBeRemoved) {
			track->toBeRemoved = true;
		}
	}
}

int32 IMuseDigital::getCurMusicPosInMs() {
	Common::StackLock lock(_mutex, "IMuseDigital::getCurMusicPosInMs()");
	int soundId = -1;

	for (int l = 0; l < MAX_DIGITAL_TRACKS; l++) {
		Track *track = _track[l];
		if (track->used && !track->toBeRemoved && (track->volGroupId == IMUSE_VOLGRP_MUSIC)) {
			soundId = track->soundId;
		}
	}

	int32 msPos = getPosInMs(soundId);
	debug(5, "IMuseDigital::getCurMusicPosInMs(%d) = %d", soundId, msPos);
	return msPos;
}

int32 IMuseDigital::getCurVoiceLipSyncWidth() {
	Common::StackLock lock(_mutex, "IMuseDigital::getCurVoiceLipSyncWidth()");
	int32 msPos = getPosInMs(kTalkSoundID) + 50;
	int32 width = 0, height = 0;

	debug(5, "IMuseDigital::getCurVoiceLipSyncWidth(%d)", kTalkSoundID);
	getLipSync(kTalkSoundID, 0, msPos, width, height);
	return width;
}

int32 IMuseDigital::getCurVoiceLipSyncHeight() {
	Common::StackLock lock(_mutex, "IMuseDigital::getCurVoiceLipSyncHeight()");
	int32 msPos = getPosInMs(kTalkSoundID) + 50;
	int32 width = 0, height = 0;

	debug(5, "IMuseDigital::getCurVoiceLipSyncHeight(%d)", kTalkSoundID);
	getLipSync(kTalkSoundID, 0, msPos, width, height);
	return height;
}

int32 IMuseDigital::getCurMusicLipSyncWidth(int syncId) {
	Common::StackLock lock(_mutex, "IMuseDigital::getCurMusicLipSyncWidth()");
	int soundId = -1;

	for (int l = 0; l < MAX_DIGITAL_TRACKS; l++) {
		Track *track = _track[l];
		if (track->used && !track->toBeRemoved && (track->volGroupId == IMUSE_VOLGRP_MUSIC)) {
			soundId = track->soundId;
		}
	}

	int32 msPos = getPosInMs(soundId) + 50;
	int32 width = 0, height = 0;

	debug(5, "IMuseDigital::getCurVoiceLipSyncWidth(%d, %d)", soundId, msPos);
	getLipSync(soundId, syncId, msPos, width, height);
	return width;
}

int32 IMuseDigital::getCurMusicLipSyncHeight(int syncId) {
	Common::StackLock lock(_mutex, "IMuseDigital::getCurMusicLipSyncHeight()");
	int soundId = -1;

	for (int l = 0; l < MAX_DIGITAL_TRACKS; l++) {
		Track *track = _track[l];
		if (track->used && !track->toBeRemoved && (track->volGroupId == IMUSE_VOLGRP_MUSIC)) {
			soundId = track->soundId;
		}
	}

	int32 msPos = getPosInMs(soundId) + 50;
	int32 width = 0, height = 0;

	debug(5, "IMuseDigital::getCurVoiceLipSyncHeight(%d, %d)", soundId, msPos);
	getLipSync(soundId, syncId, msPos, width, height);
	return height;
}

void IMuseDigital::stopAllSounds() {
	Common::StackLock lock(_mutex, "IMuseDigital::stopAllSounds()");
	debug(0, "IMuseDigital::stopAllSounds");

	for (int l = 0; l < MAX_DIGITAL_TRACKS + MAX_DIGITAL_FADETRACKS; l++) {
		Track *track = _track[l];
		if (track->used) {
			// Stop the sound output, *now*. No need to use toBeRemoved etc.
			// as we are protected by a mutex, and this method is never called
			// from IMuseDigital::callback either.
			if (track->stream) {
				_mixer->stopHandle(track->mixChanHandle);
				delete track->stream;
				track->stream = NULL;
				_sound->closeSound(track->soundHandle);
				track->soundHandle = NULL;
			} else if (track->streamSou) {
				_mixer->stopHandle(track->mixChanHandle);
				delete track->streamSou;
				track->streamSou = NULL;
			}
			
			// Mark the track as unused
			track->used = false;
		}
	}
}

void IMuseDigital::pause(bool p) {
	_pause = p;
}

} // End of namespace Scumm
