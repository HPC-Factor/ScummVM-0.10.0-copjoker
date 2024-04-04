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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/engines/agos/res_snd.cpp $
 * $Id: res_snd.cpp 27159 2007-06-07 02:45:19Z Kirben $
 *
 */

#include "common/stdafx.h"

#include "common/config-manager.h"
#include "common/file.h"

#include "agos/intern.h"
#include "agos/agos.h"
#include "agos/vga.h"

#include "sound/audiocd.h"
#include "sound/audiostream.h"
#include "sound/mididrv.h"
#include "sound/mods/protracker.h"

using Common::File;

namespace AGOS {

void AGOSEngine_Simon1::playSpeech(uint speech_id, uint vgaSpriteId) {
	if (speech_id == 9999) {
		if (_subtitles)
			return;
		if (!getBitFlag(14) && !getBitFlag(28)) {
			setBitFlag(14, true);
			_variableArray[100] = 15;
			animate(4, 1, 130, 0, 0, 0);
			waitForSync(130);
		}
		_skipVgaWait = true;
	} else {
		if (_subtitles && _scriptVar2) {
			animate(4, 2, 204, 0, 0, 0);
			waitForSync(204);
			stopAnimate(204);
		}
		if (vgaSpriteId < 100)
			stopAnimate(vgaSpriteId + 201);

		loadVoice(speech_id);

		if (vgaSpriteId < 100)
			animate(4, 2, vgaSpriteId + 201, 0, 0, 0);
	}
}

void AGOSEngine_Simon2::playSpeech(uint speech_id, uint vgaSpriteId) {
	if (speech_id == 0xFFFF) {
		if (_subtitles)
			return;
		if (!getBitFlag(14) && !getBitFlag(28)) {
			setBitFlag(14, true);
			_variableArray[100] = 5;
			animate(4, 1, 30, 0, 0, 0);
			waitForSync(130);
		}
		_skipVgaWait = true;
	} else {
		if (getGameType() == GType_SIMON2 && _subtitles && _language != Common::HB_ISR) {
			loadVoice(speech_id);
			return;
		}

		if (_subtitles && _scriptVar2) {
			animate(4, 2, 5, 0, 0, 0);
			waitForSync(205);
			stopAnimateSimon2(2,5);
		}

		stopAnimateSimon2(2, vgaSpriteId + 2);
		loadVoice(speech_id);
		animate(4, 2, vgaSpriteId + 2, 0, 0, 0);
	}
}

void AGOSEngine::skipSpeech() {
	_sound->stopVoice();
	if (!getBitFlag(28)) {
		setBitFlag(14, true);
		if (getGameType() == GType_FF) {
			_variableArray[103] = 5;
			animate(4, 2, 13, 0, 0, 0);
			waitForSync(213);
			stopAnimateSimon2(2, 1);
		} else if (getGameType() == GType_SIMON2) {
			_variableArray[100] = 5;
			animate(4, 1, 30, 0, 0, 0);
			waitForSync(130);
			stopAnimateSimon2(2, 1);
		} else {
			_variableArray[100] = 15;
			animate(4, 1, 130, 0, 0, 0);
			waitForSync(130);
			stopAnimate(1);
		}
	}
}

void AGOSEngine::loadMusic(uint music) {
	char buf[4];

	stopMusic();

	_gameFile->seek(_gameOffsetsPtr[_musicIndexBase + music - 1], SEEK_SET);
	_gameFile->read(buf, 4);
	if (!memcmp(buf, "FORM", 4)) {
		_gameFile->seek(_gameOffsetsPtr[_musicIndexBase + music - 1], SEEK_SET);
		_midi.loadXMIDI(_gameFile);
	} else {
		_gameFile->seek(_gameOffsetsPtr[_musicIndexBase + music - 1], SEEK_SET);
		_midi.loadMultipleSMF(_gameFile);
	}

	_lastMusicPlayed = music;
	_nextMusicToPlay = -1;
}

void AGOSEngine::playModule(uint music) {
	char filename[15];
	File f;

	if (getGameType() == GType_ELVIRA1 && getFeatures() & GF_DEMO)
		sprintf(filename, "elvira2");
	else if (getPlatform() == Common::kPlatformAcorn)
		sprintf(filename, "%dtune.DAT", music);
	else
		sprintf(filename, "%dtune", music);

	f.open(filename);
	if (f.isOpen() == false) {
		error("playModule: Can't load module from '%s'", filename);
	}

	Audio::AudioStream *audioStream;
	if (!(getGameType() == GType_ELVIRA1 && getFeatures() & GF_DEMO) &&
		getFeatures() & GF_CRUNCHED) {

		uint srcSize = f.size();
		byte *srcBuf = (byte *)malloc(srcSize);
		if (f.read(srcBuf, srcSize) != srcSize)
			error("playModule: Read failed");

		uint dstSize = READ_BE_UINT32(srcBuf + srcSize - 4);
		byte *dstBuf = (byte *)malloc(dstSize);
		decrunchFile(srcBuf, dstBuf, srcSize);
		free(srcBuf);

		Common::MemoryReadStream stream(dstBuf, dstSize);
		audioStream = Audio::makeProtrackerStream(&stream, _mixer->getOutputRate());
		free(dstBuf);
	} else {
		audioStream = Audio::makeProtrackerStream(&f, _mixer->getOutputRate());
	}

	_mixer->playInputStream(Audio::Mixer::kMusicSoundType, &_modHandle, audioStream);
}

void AGOSEngine_Simon1::playMusic(uint16 music, uint16 track) {
	stopMusic();

	// Support for compressed music from the ScummVM Music Enhancement Project
	AudioCD.stop();
	AudioCD.play(music + 1, -1, 0, 0);
	if (AudioCD.isPlaying())
		return;

	if (getGameId() == GID_SIMON1ACORN) {
		// TODO: Add support for Desktop Tracker format
	} else if (getPlatform() == Common::kPlatformAmiga) {
		playModule(music);
	} else if (getFeatures() & GF_TALKIE) {
		char buf[4];

		// WORKAROUND: For a script bug in the CD versions
		// We skip this music resource, as it was replaced by
		// a sound effect, and the script was never updated.
		if (music == 35)
			return;

		_midi.setLoop(true); // Must do this BEFORE loading music. (GMF may have its own override.)

		_gameFile->seek(_gameOffsetsPtr[_musicIndexBase + music], SEEK_SET);
		_gameFile->read(buf, 4);
		if (!memcmp(buf, "GMF\x1", 4)) {
			_gameFile->seek(_gameOffsetsPtr[_musicIndexBase + music], SEEK_SET);
			_midi.loadSMF(_gameFile, music);
		} else {
			_gameFile->seek(_gameOffsetsPtr[_musicIndexBase + music], SEEK_SET);
			_midi.loadMultipleSMF(_gameFile);
		}

		_midi.startTrack(0);
		_midi.startTrack(track);
	} else {
		char filename[15];
		File f;
		sprintf(filename, "MOD%d.MUS", music);
		f.open(filename);
		if (f.isOpen() == false)
			error("playMusic: Can't load music from '%s'", filename);

		_midi.setLoop(true); // Must do this BEFORE loading music. (GMF may have its own override.)

		if (getFeatures() & GF_DEMO)
			_midi.loadS1D(&f);
		else
			_midi.loadSMF(&f, music);

		_midi.startTrack(0);
		_midi.startTrack(track);
	}
}

void AGOSEngine::playMusic(uint16 music, uint16 track) {
	stopMusic();

	if (getPlatform() == Common::kPlatformAmiga) {
		playModule(music);
	} else if (getPlatform() == Common::kPlatformAtariST) {
		// TODO: Add support for music formats used
	} else {
		_midi.setLoop(true); // Must do this BEFORE loading music.

		char filename[15];
		File f;
		sprintf(filename, "MOD%d.MUS", music);
		f.open(filename);
		if (f.isOpen() == false)
			error("playMusic: Can't load music from '%s'", filename);

		_midi.loadS1D(&f);
		_midi.startTrack(0);
		_midi.startTrack(track);
	}
}

void AGOSEngine::stopMusic() {
	if (_midiEnabled) {
		_midi.stop();
	} 
	_mixer->stopHandle(_modHandle);
}

void AGOSEngine::playSting(uint a) {
	if (!_midi._enable_sfx)
		return;

	char filename[15];

	File mus_file;
	uint16 mus_offset;

	sprintf(filename, "STINGS%i.MUS", _soundFileId);
	mus_file.open(filename);
	if (!mus_file.isOpen())
		error("playSting: Can't load sound effect from '%s'", filename);

	mus_file.seek(a * 2, SEEK_SET);
	mus_offset = mus_file.readUint16LE();
	if (mus_file.ioFailed())
		error("playSting: Can't read sting %d offset", a);

	mus_file.seek(mus_offset, SEEK_SET);
	_midi.loadSMF(&mus_file, a, true);
	_midi.startTrack(0);
}

static const byte elvira1_soundTable[100] = {
	0, 2, 0, 1, 0, 0, 0, 0, 0, 3,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 6, 4, 0, 0, 9, 0,
	0, 2, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 8, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 1, 0, 0, 5, 0, 6, 6, 0, 0,
	0, 5, 0, 0, 6, 0, 0, 0, 0, 8,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

bool AGOSEngine::loadVGASoundFile(uint id, uint type) {
	File in;
	char filename[15];
	byte *dst;
	uint32 srcSize, dstSize;

	if (getPlatform() == Common::kPlatformAmiga || getPlatform() == Common::kPlatformAtariST) {
		if (getGameType() == GType_ELVIRA1 && getFeatures() & GF_DEMO) {
			if (id == 20)
				sprintf(filename, "D%d.out", type);
			else if (id == 26)
				sprintf(filename, "J%d.out", type);
			else if (id == 27)
				sprintf(filename, "K%d.out", type);
			else if (id == 33)
				sprintf(filename, "Q%d.out", type);
			else if (id == 34)
				sprintf(filename, "R%d.out", type);
			else
				sprintf(filename, "%.1d%d.out", id, type);
		} else if (getGameType() == GType_ELVIRA1 || getGameType() == GType_ELVIRA2) {
			sprintf(filename, "%.2d%d.out", id, type);
		} else {
			sprintf(filename, "%.3d%d.out", id, type);
		}
	} else {
		if (getGameType() == GType_ELVIRA1) {
			if (elvira1_soundTable[id] == 0)
				return false;

			sprintf(filename, "%.2d.SND", elvira1_soundTable[id]);
		} else if (getGameType() == GType_ELVIRA2 || getGameType() == GType_WW) {
			sprintf(filename, "%.2d%d.VGA", id, type);
		} else {
			sprintf(filename, "%.3d%d.VGA", id, type);
		}
	}

	in.open(filename);
	if (in.isOpen() == false || in.size() == 0) {
		return false;
	}

	dstSize = srcSize = in.size();
	if (getGameType() == GType_ELVIRA1 && getFeatures() & GF_DEMO) {
		byte *srcBuffer = (byte *)malloc(srcSize);
		if (in.read(srcBuffer, srcSize) != srcSize)
			error("loadVGASoundFile: Read failed");

		dstSize = READ_BE_UINT32(srcBuffer + srcSize - 4);
		dst = allocBlock (dstSize);
		decrunchFile(srcBuffer, dst, srcSize);
		free(srcBuffer);
	} else {
		dst = allocBlock(dstSize);
		if (in.read(dst, dstSize) != dstSize)
			error("loadVGASoundFile: Read failed");
	}
	in.close();

	return true;
}

static const char *dimpSoundList[32] = {
	"Beep",
	"Birth",
	"Boiling",
	"Burp",
	"Cough",
	"Die1",
	"Die2",
	"Fart",
	"Inject",
	"Killchik",
	"Puke",
	"Lights",
	"Shock",
	"Snore",
	"Snotty",
	"Whip",
	"Whistle",
	"Work1",
	"Work2",
	"Yawn",
	"And0w",
	"And0x",
	"And0y",
	"And0z",
	"And10",
	"And11",
	"And12",
	"And13",
	"And14",
	"And15",
	"And16",
	"And17",
};


void AGOSEngine::loadSoundFile(const char* filename) {
	File in;

	in.open(filename);
	if (in.isOpen() == false)
		error("loadSound: Can't load %s", filename);

	uint32 dstSize = in.size();
	byte *dst = (byte *)malloc(dstSize);
	if (in.read(dst, dstSize) != dstSize)
		error("loadSound: Read failed");
	in.close();

	_sound->playSfxData(dst, 0, 0, 0);
}

void AGOSEngine::loadSound(uint sound) {
	byte *dst;
	uint32 offs, size;

	if (_curSfxFile == NULL)
		return;

	dst = _curSfxFile;
	if (getGameType() == GType_WW) {
		uint tmp = sound;
		while (tmp--)
			dst += READ_LE_UINT16(dst) + 4;

		size = READ_LE_UINT16(dst);
		offs = 4;
	} else if (getGameType() == GType_ELVIRA2) {
		while (READ_BE_UINT32(dst + 4) != sound)
			dst += 12;

		size = READ_BE_UINT32(dst);
		offs = READ_BE_UINT32(dst + 8);
	} else {
		while (READ_BE_UINT16(dst + 6) != sound)
			dst += 12;

		size = READ_BE_UINT16(dst + 2);
		offs = READ_BE_UINT32(dst + 8);
	}

	_sound->playRawData(dst + offs, sound, size);
}

void AGOSEngine::loadSound(uint sound, int pan, int vol, uint type) {
	byte *dst;

	if (getGameId() == GID_DIMP) {
		File in;
		char filename[15];

		assert(sound >= 1 && sound <= 32);
		sprintf(filename, "%s.wav", dimpSoundList[sound - 1]);

		in.open(filename);
		if (in.isOpen() == false)
			error("loadSound: Can't load %s", filename);

		uint32 dstSize = in.size();
		dst = (byte *)malloc(dstSize);
		if (in.read(dst, dstSize) != dstSize)
			error("loadSound: Read failed");
		in.close();
	} else if (getFeatures() & GF_ZLIBCOMP) {
		char filename[15];

		uint32 file, offset, srcSize, dstSize;
		if (getPlatform() == Common::kPlatformAmiga) {
			loadOffsets((const char*)"sfxindex.dat", _zoneNumber * 22 + sound, file, offset, srcSize, dstSize);
		} else {
			loadOffsets((const char*)"effects.wav", _zoneNumber * 22 + sound, file, offset, srcSize, dstSize);
		}

		if (getPlatform() == Common::kPlatformAmiga)
			sprintf(filename, "sfx%d.wav", file);
		else
			sprintf(filename, "effects.wav");

		dst = (byte *)malloc(dstSize);
		decompressData(filename, dst, offset, srcSize, dstSize);
	} else {
		if (!_curSfxFile)
			error("loadSound: Can't load sound data file '%d3.VGA'", _zoneNumber);

		dst = _curSfxFile + READ_LE_UINT32(_curSfxFile + sound * 4);
	}

	if (type == 3)
		_sound->playSfx5Data(dst, sound, pan, vol);
	else if (type == 2)
		_sound->playAmbientData(dst, sound, pan, vol);
	else
		_sound->playSfxData(dst, sound, pan, vol);
}

void AGOSEngine::loadVoice(uint speechId) {
	if (getGameType() == GType_PP && speechId == 99) {
		_sound->stopVoice();
		return;
	}

	if (getFeatures() & GF_ZLIBCOMP) {
		char filename[15];

		uint32 file, offset, srcSize, dstSize;
		if (getPlatform() == Common::kPlatformAmiga) {
			loadOffsets((const char*)"spindex.dat", speechId, file, offset, srcSize, dstSize);
		} else {
			loadOffsets((const char*)"speech.wav", speechId, file, offset, srcSize, dstSize);
		}

		// Voice segment doesn't exist
		if (offset == 0xFFFFFFFF && srcSize == 0xFFFFFFFF && dstSize == 0xFFFFFFFF) {
			debug(0, "loadVoice: speechId %d removed", speechId);
			return;
		}

		if (getPlatform() == Common::kPlatformAmiga)
			sprintf(filename, "sp%d.wav", file);
		else
			sprintf(filename, "speech.wav");

		byte *dst = (byte *)malloc(dstSize);
		decompressData(filename, dst, offset, srcSize, dstSize);
		_sound->playVoiceData(dst, speechId);
	} else {
		_sound->playVoice(speechId);
	}
}

} // End of namespace AGOS
