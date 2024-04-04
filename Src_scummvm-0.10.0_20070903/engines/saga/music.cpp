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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/engines/saga/music.cpp $
 * $Id: music.cpp 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */
#include "saga/saga.h"

#include "saga/rscfile.h"
#include "saga/sagaresnames.h"
#include "saga/music.h"
#include "saga/stream.h"
#include "sound/audiostream.h"
#include "sound/mididrv.h"
#include "sound/midiparser.h"
#include "common/config-manager.h"
#include "common/file.h"

namespace Saga {

#define BUFFER_SIZE 4096

// I haven't decided yet if it's a good idea to make looping part of the audio
// stream class, or if I should use a "wrapper" class, like I did for Broken
// Sword 2, to make it easier to add support for compressed music... but I'll
// worry about that later.

class RAWInputStream : public Audio::AudioStream {
private:
	ResourceContext *_context;
	Common::File *_file;
	uint32 _filePos;
	uint32 _startPos;
	uint32 _endPos;
	bool _finished;
	bool _looping;
	int16 _buf[BUFFER_SIZE];
	const int16 *_bufferEnd;
	const int16 *_pos;
	const GameSoundInfo *_musicInfo;

	void refill();
	bool eosIntern() const {
		return _pos >= _bufferEnd;
	}

public:
	RAWInputStream(SagaEngine *vm, ResourceContext *context, uint32 resourceId, bool looping, uint32 loopStart);

	int readBuffer(int16 *buffer, const int numSamples);

	bool endOfData() const	{ return eosIntern(); }
	bool isStereo() const	{ return _musicInfo->stereo; }
	int getRate() const	{ return _musicInfo->frequency; }
};

RAWInputStream::RAWInputStream(SagaEngine *vm, ResourceContext *context, uint32 resourceId, bool looping, uint32 loopStart)
	: _context(context), _finished(false), _looping(looping), _bufferEnd(_buf + BUFFER_SIZE) {

	ResourceData * resourceData;

	resourceData = vm->_resource->getResourceData(context, resourceId);
	_file = context->getFile(resourceData);
	_musicInfo = vm->getMusicInfo();

	if (_musicInfo == NULL) {
		error("RAWInputStream() wrong musicInfo");
	}

	// Determine the end position
	_filePos = resourceData->offset;
	_endPos = _filePos + resourceData->size;
	_startPos = _filePos + loopStart;
	if (_startPos >= _endPos)
		_startPos = _filePos;

	// Read in initial data
	refill();
}

int RAWInputStream::readBuffer(int16 *buffer, const int numSamples) {
	int samples = 0;
	while (samples < numSamples && !eosIntern()) {
		const int len = MIN(numSamples - samples, (int) (_bufferEnd - _pos));
		memcpy(buffer, _pos, len * 2);
		buffer += len;
		_pos += len;
		samples += len;
		if (_pos >= _bufferEnd) {
			refill();
		}
	}
	return samples;
}

void RAWInputStream::refill() {
	if (_finished)
		return;

	uint32 lengthLeft;
	byte *ptr = (byte *) _buf;


	_file->seek(_filePos, SEEK_SET);

	if (_looping)
		lengthLeft = 2 * BUFFER_SIZE;
	else
		lengthLeft = MIN((uint32) (2 * BUFFER_SIZE), _endPos - _filePos);

	while (lengthLeft > 0) {
		uint32 len = _file->read(ptr, MIN(lengthLeft, _endPos - _file->pos()));

		if (len & 1)
			len--;

#ifdef SCUMM_BIG_ENDIAN
		if (!_context->isBigEndian) {
#else
		if (_context->isBigEndian) {
#endif
			uint16 *ptr16 = (uint16 *)ptr;
			for (uint32 i = 0; i < (len / 2); i++)
				ptr16[i] = SWAP_BYTES_16(ptr16[i]);
		}

		lengthLeft -= len;
		ptr += len;

		if (lengthLeft > 0)
			_file->seek(_startPos);
	}

	_filePos = _file->pos();
	_pos = _buf;
	_bufferEnd = (int16 *)ptr;

	if (!_looping && _filePos >= _endPos) {
		_finished = true;
	}
}


MusicPlayer::MusicPlayer(MidiDriver *driver) : _parser(0), _driver(driver), _looping(false), _isPlaying(false), _passThrough(false), _isGM(false) {
	memset(_channel, 0, sizeof(_channel));
	_masterVolume = 0;
	this->open();
}

MusicPlayer::~MusicPlayer() {
	_driver->setTimerCallback(NULL, NULL);
	stopMusic();
	this->close();
}

void MusicPlayer::setVolume(int volume) {
	if (volume < 0)
		volume = 0;
	else if (volume > 255)
		volume = 255;

	if (_masterVolume == volume)
		return;

	_masterVolume = volume;

	for (int i = 0; i < 16; ++i) {
		if (_channel[i]) {
			_channel[i]->volume(_channelVolume[i] * _masterVolume / 255);
		}
	}
}

int MusicPlayer::open() {
	// Don't ever call open without first setting the output driver!
	if (!_driver)
		return 255;

	int ret = _driver->open();
	if (ret)
		return ret;

	_driver->setTimerCallback(this, &onTimer);
	return 0;
}

void MusicPlayer::close() {
	stopMusic();
	if (_driver)
		_driver->close();
	_driver = 0;
}

void MusicPlayer::send(uint32 b) {
	if (_passThrough) {
		_driver->send(b);
		return;
	}

	byte channel = (byte)(b & 0x0F);
	if ((b & 0xFFF0) == 0x07B0) {
		// Adjust volume changes by master volume
		byte volume = (byte)((b >> 16) & 0x7F);
		_channelVolume[channel] = volume;
		volume = volume * _masterVolume / 255;
		b = (b & 0xFF00FFFF) | (volume << 16);
	} else if ((b & 0xF0) == 0xC0 && !_isGM && !_nativeMT32) {
		b = (b & 0xFFFF00FF) | MidiDriver::_mt32ToGm[(b >> 8) & 0xFF] << 8;
	}
	else if ((b & 0xFFF0) == 0x007BB0) {
		//Only respond to All Notes Off if this channel
		//has currently been allocated
		if (_channel[b & 0x0F])
			return;
	}

	if (!_channel[channel])
		_channel[channel] = (channel == 9) ? _driver->getPercussionChannel() : _driver->allocateChannel();

	if (_channel[channel])
		_channel[channel]->send(b);
}

void MusicPlayer::metaEvent(byte type, byte *data, uint16 length) {
	// FIXME: The "elkfanfare" is played much too quickly. There are some
	//        meta events that we don't handle. Perhaps there is a
	//        connection...?

	switch (type) {
	case 0x2F:	// End of Track
		if (_looping)
			_parser->jumpToTick(0);
		else
			stopMusic();
		break;
	default:
		//warning("Unhandled meta event: %02x", type);
		break;
	}
}

void MusicPlayer::onTimer(void *refCon) {
	MusicPlayer *music = (MusicPlayer *)refCon;
	if (music->_isPlaying)
		music->_parser->onTimer();
}

void MusicPlayer::playMusic() {
	_isPlaying = true;
}

void MusicPlayer::stopMusic() {
	_isPlaying = false;
	if (_parser) {
		_parser->unloadMusic();
		_parser = NULL;
	}
}

Music::Music(SagaEngine *vm, Audio::Mixer *mixer, MidiDriver *driver, int enabled) : _vm(vm), _mixer(mixer), _enabled(enabled), _adlib(false) {
	_player = new MusicPlayer(driver);
	_currentVolume = 0;

	xmidiParser = MidiParser::createParser_XMIDI();
	smfParser = MidiParser::createParser_SMF();	

	_musicContext = _vm->_resource->getContext(GAME_MUSICFILE);

	_songTableLen = 0;
	_songTable = 0;

	_midiMusicData = NULL;
}

Music::~Music() {
	_mixer->stopHandle(_musicHandle);
	delete _player;
	xmidiParser->setMidiDriver(NULL);
	smfParser->setMidiDriver(NULL);
	delete xmidiParser;
	delete smfParser;

	free(_songTable);
	free(_midiMusicData);
}

void Music::musicVolumeGaugeCallback(void *refCon) {
	((Music *)refCon)->musicVolumeGauge();
}

void Music::musicVolumeGauge() {
	int volume;

	_currentVolumePercent += 10;

	if (_currentVolume - _targetVolume > 0) { // Volume decrease
		volume = _targetVolume + (_currentVolume - _targetVolume) * (100 - _currentVolumePercent) / 100;
	} else {
		volume = _currentVolume + (_targetVolume - _currentVolume) * _currentVolumePercent / 100;
	}

	if (volume < 0)
		volume = 1;

	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, volume);
	_player->setVolume(volume);

	if (_currentVolumePercent == 100) {
		_vm->_timer->removeTimerProc(&musicVolumeGaugeCallback);
		_currentVolume = _targetVolume;
	}
}

void Music::setVolume(int volume, int time) {
	_targetVolume = volume * 2; // ScummVM has different volume scale
	_currentVolumePercent = 0;

	if (volume == -1) // Set Full volume
		volume = 255;

	if (time == 1) {
		_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, volume);
		_player->setVolume(volume);
		_vm->_timer->removeTimerProc(&musicVolumeGaugeCallback);
		_currentVolume = volume;
		return;
	}

	_vm->_timer->installTimerProc(&musicVolumeGaugeCallback, time * 100L, this);
}

bool Music::isPlaying() {
	return _mixer->isSoundHandleActive(_musicHandle) || _player->isPlaying();
}

void Music::play(uint32 resourceId, MusicFlags flags) {
	Audio::AudioStream *audioStream = NULL;
	MidiParser *parser;
	ResourceContext *context;
	byte *resourceData;
	size_t resourceSize;	
	uint32 loopStart;

	debug(2, "Music::play %d, %d", resourceId, flags);

	if (!_enabled) {
		return;
	}

	if (isPlaying() && _trackNumber == resourceId) {
		return;
	}

	_trackNumber = resourceId;
	_player->stopMusic();
	_mixer->stopHandle(_musicHandle);

	int realTrackNumber;

	if (_vm->getGameType() == GType_ITE) {
		if (flags == MUSIC_DEFAULT) {
			if (resourceId == 13 || resourceId == 19) {
				flags = MUSIC_NORMAL;
			} else {
				flags = MUSIC_LOOP;
			}
		}
		realTrackNumber = resourceId - 8;
	} else {
		realTrackNumber = resourceId + 1;
	}

	// Try to open standalone digital track
	char trackName[2][16];
	sprintf(trackName[0], "track%d", realTrackNumber);
	sprintf(trackName[1], "track%02d", realTrackNumber);
	Audio::AudioStream *stream = 0;
	for (int i = 0; i < 2; ++i) {
		// We multiply by 40 / 3 = 1000 / 75 to convert frames to milliseconds
		// FIXME: Do we really want a duration of 10000 frames = 133 seconds, or is that just a random value?
		stream = Audio::AudioStream::openStreamFile(trackName[i], 0, 10000 * 40 / 3, (flags == MUSIC_LOOP) ? 0 : 1);
		if (stream) {
			_mixer->playInputStream(Audio::Mixer::kMusicSoundType, &_musicHandle, stream);
			return;
		}
	}

	if (_vm->getGameType() == GType_ITE) {
		if (resourceId >= 9 && resourceId <= 34) {
			if (_musicContext != NULL) {
				//TODO: check resource size
				loopStart = 0;
				// fix ITE sunstatm score
				if ((_vm->getGameType() == GType_ITE) && (resourceId == MUSIC_SUNSPOT)) {
					loopStart = 4 * 18727;
				}

				audioStream = new RAWInputStream(_vm, _musicContext, resourceId - 9, flags == MUSIC_LOOP, loopStart);
			}
		}
	}

	if (audioStream) {
		debug(2, "Playing digitized music");
		_mixer->playInputStream(Audio::Mixer::kMusicSoundType, &_musicHandle, audioStream);
		return;
	}

	if (flags == MUSIC_DEFAULT) {
		flags = MUSIC_NORMAL;
	}

	// Load MIDI/XMI resource data

	if (_vm->getGameType() == GType_ITE) {
		context = _vm->_resource->getContext(GAME_MUSICFILE_GM);
		if (context == NULL) {
			context = _vm->_resource->getContext(GAME_RESOURCEFILE);
		}		
	} else {
		// I've listened to music from both the FM and the GM
		// file, and I've tentatively reached the conclusion
		// that they are both General MIDI. My guess is that
		// the FM file has been reorchestrated to sound better
		// on Adlib and other FM synths.
		//
		// Sev says the Adlib music does not sound like in the
		// original, but I still think assuming General MIDI is
		// the right thing to do. Some music, like the End
		// Title (song 0) sound absolutely atrocious when piped
		// through our MT-32 to GM mapping.
		//
		// It is, however, quite possible that the original
		// used a different GM to FM mapping. If the original
		// sounded markedly better, perhaps we should add some
		// way of replacing our stock mapping in adlib.cpp?
		//
		// For the composer's own recording of the End Title,
		// see http://www.johnottman.com/

		// Oddly enough, the intro music (song 1) is very
		// different in the two files. I have no idea why.

		if (hasAdlib()) {
			context = _vm->_resource->getContext(GAME_MUSICFILE_FM);
		} else {
			context = _vm->_resource->getContext(GAME_MUSICFILE_GM);
		}
	}

	_player->setGM(true);

	_vm->_resource->loadResource(context, resourceId, resourceData, resourceSize);

	if (resourceSize < 4) {
		error("Music::play() wrong music resource size");
	}

	if (xmidiParser->loadMusic(resourceData, resourceSize)) {
		if (_vm->getGameType() == GType_ITE)
			_player->setGM(false);

		parser = xmidiParser;
	} else {
		if (smfParser->loadMusic(resourceData, resourceSize)) {
			parser = smfParser;
		} else {
			error("Music::play() wrong music resource");
		}
	}

	parser->setTrack(0);
	parser->setMidiDriver(_player);
	parser->setTimerRate(_player->getBaseTempo());
	parser->property(MidiParser::mpCenterPitchWheelOnUnload, 1);

	_player->_parser = parser;
	_player->setVolume(_vm->_musicVolume == 10 ? 255 : _vm->_musicVolume * 25);

	if (flags & MUSIC_LOOP)
		_player->setLoop(true);
	else
		_player->setLoop(false);

	_player->playMusic();
	free(_midiMusicData);
	_midiMusicData = resourceData;
}

void Music::pause(void) {
	//TODO: do it
}

void Music::resume(void) {
	//TODO: do it}
}

void Music::stop(void) {
	//TODO: do it
}

} // End of namespace Saga

