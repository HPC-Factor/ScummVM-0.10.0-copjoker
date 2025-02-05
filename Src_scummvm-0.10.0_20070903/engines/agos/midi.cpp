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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/engines/agos/midi.cpp $
 * $Id: midi.cpp 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

#include "common/stdafx.h"

#include "common/file.h"
#include "common/system.h"

#include "agos/agos.h"

namespace AGOS {


// MidiParser_S1D is not considered part of the standard
// MidiParser suite, but we still try to mask its details
// and just provide a factory function.
extern MidiParser *MidiParser_createS1D();

MidiPlayer::MidiPlayer() {
	// Since initialize() is called every time the music changes,
	// this is where we'll initialize stuff that must persist
	// between songs.
	_driver = 0;
	_map_mt32_to_gm = false;
	_passThrough = false;

	_enable_sfx = true;
	_current = 0;

	_masterVolume = 255;
	resetVolumeTable();
	_paused = false;

	_currentTrack = 255;
	_loopTrack = 0;
	_queuedTrack = 255;
	_loopQueuedTrack = 0;
}

MidiPlayer::~MidiPlayer() {
	_mutex.lock();
	close();
	_mutex.unlock();
}

int MidiPlayer::open() {
	// Don't ever call open without first setting the output driver!
	if (!_driver)
		return 255;

	int ret = _driver->open();
	if (ret)
		return ret;
	_driver->setTimerCallback(this, &onTimer);

	// General MIDI System On message
	// Resets all GM devices to default settings
	_driver->sysEx((const byte *)"\x7E\x7F\x09\x01", 4);
	g_system->delayMillis(20);

	return 0;
}

void MidiPlayer::close() {
	stop();
//	_system->lockMutex(_mutex);
	if (_driver)
		_driver->close();
	_driver = NULL;
	clearConstructs();
//	_system->unlockMutex(_mutex);
}

void MidiPlayer::send(uint32 b) {
	if (!_current)
		return;

	if (_passThrough) {
		_driver->send(b);
		return;
	}

	byte channel = (byte)(b & 0x0F);
	if ((b & 0xFFF0) == 0x07B0) {
		// Adjust volume changes by master volume.
		byte volume = (byte)((b >> 16) & 0x7F);
		_current->volume[channel] = volume;
		volume = volume * _masterVolume / 255;
		b = (b & 0xFF00FFFF) | (volume << 16);
	} else if ((b & 0xF0) == 0xC0 && _map_mt32_to_gm) {
		b = (b & 0xFFFF00FF) | (MidiDriver::_mt32ToGm[(b >> 8) & 0xFF] << 8);
	} else if ((b & 0xFFF0) == 0x007BB0) {
		// Only respond to an All Notes Off if this channel
		// has already been allocated.
		if (!_current->channel[b & 0x0F])
			return;
	} else if ((b & 0xFFF0) == 0x79B0) {
		// "Reset All Controllers". There seems to be some confusion
		// about what this message should do to the volume controller.
		// See http://www.midi.org/about-midi/rp15.shtml for more
		// information.
		//
		// If I understand it correctly, the current standard indicates
		// that the volume should be reset, but the next revision will
		// exclude it. On my system, both ALSA and FluidSynth seem to
		// reset it, while Adlib does not. Let's follow the majority.

		_current->volume[channel] = 127;
	}

	if (!_current->channel[channel])
		_current->channel[channel] = (channel == 9) ? _driver->getPercussionChannel() : _driver->allocateChannel();
	if (_current->channel[channel]) {
		if (channel == 9)
			_current->channel[9]->volume(_current->volume[9] * _masterVolume / 255);
		_current->channel[channel]->send(b);
		if ((b & 0xFFF0) == 0x79B0) {
			// We have received a "Reset All Controllers" message
			// and passed it on to the MIDI driver. This may or may
			// not have affected the volume controller. To ensure
			// consistent behaviour, explicitly set the volume to
			// what we think it should be.

			_current->channel[channel]->volume(_current->volume[channel] * _masterVolume / 255);
		}
	}
}

void MidiPlayer::metaEvent(byte type, byte *data, uint16 length) {
	// Only thing we care about is End of Track.
	if (!_current || type != 0x2F) {
		return;
	} else if (_current == &_sfx) {
		clearConstructs(_sfx);
	} else if (_loopTrack) {
		_current->parser->jumpToTick(0);
	} else if (_queuedTrack != 255) {
		_currentTrack = 255;
		byte destination = _queuedTrack;
		_queuedTrack = 255;
		_loopTrack = _loopQueuedTrack;
		_loopQueuedTrack = false;

		// Remember, we're still inside the locked mutex.
		// Have to unlock it before calling jump()
		// (which locks it itself), and then relock it
		// upon returning.
		_mutex.unlock();
		startTrack(destination);
		_mutex.lock();
	} else {
		stop();
	}
}

void MidiPlayer::onTimer(void *data) {
	MidiPlayer *p = (MidiPlayer *)data;
	Common::StackLock lock(p->_mutex);

	if (!p->_paused) {
		if (p->_music.parser && p->_currentTrack != 255) {
			p->_current = &p->_music;
			p->_music.parser->onTimer();
		}
	}
	if (p->_sfx.parser) {
		p->_current = &p->_sfx;
		p->_sfx.parser->onTimer();
	}
	p->_current = 0;
}

void MidiPlayer::startTrack(int track) {
	if (track == _currentTrack)
		return;

	if (_music.num_songs > 0) {
		if (track >= _music.num_songs)
			return;

		_mutex.lock();

		if (_music.parser) {
			_current = &_music;
			delete _music.parser;
			_current = 0;
			_music.parser = 0;
		}

		MidiParser *parser = MidiParser::createParser_SMF();
		parser->property (MidiParser::mpMalformedPitchBends, 1);
		parser->setMidiDriver(this);
		parser->setTimerRate(_driver->getBaseTempo());
		if (!parser->loadMusic(_music.songs[track], _music.song_sizes[track])) {
			printf ("Error reading track!\n");
			delete parser;
			parser = 0;
		}

		_currentTrack = (byte)track;
		_music.parser = parser; // That plugs the power cord into the wall
	} else if (_music.parser) {
		_mutex.lock();
		if (!_music.parser->setTrack(track)) {
			_mutex.unlock();
			return;
		}
		_currentTrack = (byte)track;
		_current = &_music;
		_music.parser->jumpToTick(0);
		_current = 0;
	}

	_mutex.unlock();
}

void MidiPlayer::stop() {
	Common::StackLock lock(_mutex);

	if (_music.parser) {
		_current = &_music;
		_music.parser->jumpToTick(0);
	}
	_current = 0;
	_currentTrack = 255;
}

void MidiPlayer::pause(bool b) {
	if (_paused == b || !_driver)
		return;
	_paused = b;

	Common::StackLock lock(_mutex);
	for (int i = 0; i < 16; ++i) {
		if (_music.channel[i])
			_music.channel[i]->volume(_paused ? 0 : (_music.volume[i] * _masterVolume / 255));
		if (_sfx.channel[i])
			_sfx.channel[i]->volume(_paused ? 0 : (_sfx.volume[i] * _masterVolume / 255));
	}
}

void MidiPlayer::setVolume(int volume) {
	if (volume < 0)
		volume = 0;
	else if (volume > 255)
		volume = 255;

	if (_masterVolume == volume)
		return;
	_masterVolume = volume;

	// Now tell all the channels this.
	Common::StackLock lock(_mutex);
	if (_driver && !_paused) {
		for (int i = 0; i < 16; ++i) {
			if (_music.channel[i])
				_music.channel[i]->volume(_music.volume[i] * _masterVolume / 255);
			if (_sfx.channel[i])
				_sfx.channel[i]->volume(_sfx.volume[i] * _masterVolume / 255);
		}
	}
}

void MidiPlayer::setDriver(MidiDriver *md) {
	// Don't try to set this more than once.
	if (_driver)
		return;
	_driver = md;
}

void MidiPlayer::mapMT32toGM(bool map) {
	Common::StackLock lock(_mutex);

	_map_mt32_to_gm = map;
}

void MidiPlayer::setLoop(bool loop) {
	Common::StackLock lock(_mutex);

	_loopTrack = loop;
}

void MidiPlayer::queueTrack(int track, bool loop) {
	_mutex.lock();
	if (_currentTrack == 255) {
		_mutex.unlock();
		setLoop(loop);
		startTrack(track);
	} else {
		_queuedTrack = track;
		_loopQueuedTrack = loop;
		_mutex.unlock();
	}
}

void MidiPlayer::clearConstructs() {
	clearConstructs(_music);
	clearConstructs(_sfx);
}

void MidiPlayer::clearConstructs(MusicInfo &info) {
	int i;
	if (info.num_songs > 0) {
		for (i = 0; i < info.num_songs; ++i)
			free(info.songs[i]);
		info.num_songs = 0;
	}

	if (info.data) {
		free(info.data);
		info.data = 0;
	} // end if

	if (info.parser) {
		delete info.parser;
		info.parser = 0;
	}

	if (_driver) {
		for (i = 0; i < 16; ++i) {
			if (info.channel[i]) {
				info.channel[i]->allNotesOff();
				info.channel[i]->release();
			}
		}
	}
	info.clear();
}

void MidiPlayer::resetVolumeTable() {
	int i;
	for (i = 0; i < 16; ++i) {
		_music.volume[i] = _sfx.volume[i] = 127;
		if (_driver)
			_driver->send(((_masterVolume >> 1) << 16) | 0x7B0 | i);
	}
}

static const int simon1_gmf_size[] = {
	8900, 12166, 2848, 3442, 4034, 4508, 7064, 9730, 6014, 4742, 3138,
	6570, 5384, 8909, 6457, 16321, 2742, 8968, 4804, 8442, 7717,
	9444, 5800, 1381, 5660, 6684, 2456, 4744, 2455, 1177, 1232,
	17256, 5103, 8794, 4884, 16
};

void MidiPlayer::loadSMF(Common::File *in, int song, bool sfx) {
	Common::StackLock lock(_mutex);

	MusicInfo *p = sfx ? &_sfx : &_music;
	clearConstructs(*p);

	uint32 startpos = in->pos();
	byte header[4];
	in->read(header, 4);
	bool isGMF = !memcmp(header, "GMF\x1", 4);
	in->seek(startpos, SEEK_SET);

	uint32 size = in->size() - in->pos();
	if (isGMF) {
		if (sfx) {
			// Multiple GMF resources are stored in the SFX files,
			// but each one is referenced by a pointer at the
			// beginning of the file. Those pointers can be used
			// to determine file size.
			in->seek(0, SEEK_SET);
			uint16 value = in->readUint16LE() >> 2; // Number of resources
			if (song != value - 1) {
				in->seek(song * 2 + 2, SEEK_SET);
				value = in->readUint16LE();
				size = value - startpos;
			}
			in->seek(startpos, SEEK_SET);
		} else if (size >= 64000) {
			// For GMF resources not in separate
			// files, we're going to have to use
			// hardcoded size tables.
			size = simon1_gmf_size[song];
		}
	}

	// When allocating space, add 4 bytes in case
	// this is a GMF and we have to tack on our own
	// End of Track event.
	p->data = (byte *)calloc(size + 4, 1);
	in->read(p->data, size);

	uint32 timerRate = _driver->getBaseTempo();

	if (!memcmp(p->data, "GMF\x1", 4)) {
		// The GMF header
		// 3 BYTES: 'GMF'
		// 1 BYTE : Major version
		// 1 BYTE : Minor version
		// 1 BYTE : Ticks (Ranges from 2 - 8, always 2 for SFX)
		// 1 BYTE : Loop control. 0 = no loop, 1 = loop

		// In the original, the ticks value indicated how many
		// times the music timer was called before it actually
		// did something. The larger the value the slower the
		// music.
		//
		// We, on the other hand, have a timer rate which is
		// used to control by how much the music advances on
		// each onTimer() call. The larger the value, the
		// faster the music.
		//
		// It seems that 4 corresponds to our base tempo, so
		// this should be the right way to calculate it.
		timerRate = (4 * _driver->getBaseTempo()) / p->data[5];

		// According to bug #1004919 calling setLoop() from
		// within a lock causes a lockup, though I have no
		// idea when this actually happens.
		_loopTrack = (p->data[6] != 0);
	}

	MidiParser *parser = MidiParser::createParser_SMF();
	parser->property(MidiParser::mpMalformedPitchBends, 1);
	parser->setMidiDriver(this);
	parser->setTimerRate(timerRate);
	if (!parser->loadMusic(p->data, size)) {
		printf("Error reading track!\n");
		delete parser;
		parser = 0;
	}

	if (!sfx) {
		_currentTrack = 255;
		resetVolumeTable();
	}
	p->parser = parser; // That plugs the power cord into the wall
}

void MidiPlayer::loadMultipleSMF(Common::File *in, bool sfx) {
	// This is a special case for Simon 2 Windows.
	// Instead of having multiple sequences as
	// separate tracks in a Type 2 file, simon2win
	// has multiple songs, each of which is a Type 1
	// file. Thus, preceding the songs is a single
	// byte specifying how many songs are coming.
	// We need to load ALL the songs and then
	// treat them as separate tracks -- for the
	// purpose of jumps, anyway.
	Common::StackLock lock(_mutex);

	MusicInfo *p = sfx ? &_sfx : &_music;
	clearConstructs(*p);

	p->num_songs = in->readByte();
	if (p->num_songs > 16) {
		printf ("playMultipleSMF: %d is too many songs to keep track of!\n", (int)p->num_songs);
		return;
	}

	byte i;
	for (i = 0; i < p->num_songs; ++i) {
		byte buf[5];
		uint32 pos = in->pos();

		// Make sure there's a MThd
		in->read(buf, 4);
		if (memcmp(buf, "MThd", 4)) {
			printf("Expected MThd but found '%c%c%c%c' instead!\n", buf[0], buf[1], buf[2], buf[3]);
			return;
		}
		in->seek(in->readUint32BE(), SEEK_CUR);

		// Now skip all the MTrk blocks
		while (true) {
			in->read(buf, 4);
			if (memcmp(buf, "MTrk", 4))
				break;
			in->seek(in->readUint32BE(), SEEK_CUR);
		}

		uint32 pos2 = in->pos() - 4;
		uint32 size = pos2 - pos;
		p->songs[i] = (byte *)calloc(size, 1);
		in->seek(pos, SEEK_SET);
		in->read(p->songs[i], size);
		p->song_sizes[i] = size;
	}

	if (!sfx) {
		_currentTrack = 255;
		resetVolumeTable();
	}
}

void MidiPlayer::loadXMIDI(Common::File *in, bool sfx) {
	Common::StackLock lock(_mutex);
	MusicInfo *p = sfx ? &_sfx : &_music;
	clearConstructs(*p);

	char buf[4];
	uint32 pos = in->pos();
	uint32 size = 4;
	in->read(buf, 4);
	if (!memcmp(buf, "FORM", 4)) {
		int i;
		for (i = 0; i < 16; ++i) {
			if (!memcmp(buf, "CAT ", 4))
				break;
			size += 2;
			memcpy(buf, &buf[2], 2);
			in->read(&buf[2], 2);
		}
		if (memcmp(buf, "CAT ", 4)) {
			error("Could not find 'CAT ' tag to determine resource size");
		}
		size += 4 + in->readUint32BE();
		in->seek(pos, 0);
		p->data = (byte *)calloc(size, 1);
		in->read(p->data, size);
	} else {
		error("Expected 'FORM' tag but found '%c%c%c%c' instead", buf[0], buf[1], buf[2], buf[3]);
	}

	MidiParser *parser = MidiParser::createParser_XMIDI();
	parser->setMidiDriver(this);
	parser->setTimerRate(_driver->getBaseTempo());
	if (!parser->loadMusic(p->data, size))
		error("Error reading track");

	if (!sfx) {
		_currentTrack = 255;
		resetVolumeTable();
	}
	p->parser = parser; // That plugs the power cord into the wall
}

void MidiPlayer::loadS1D(Common::File *in, bool sfx) {
	Common::StackLock lock(_mutex);
	MusicInfo *p = sfx ? &_sfx : &_music;
	clearConstructs(*p);

	uint16 size = in->readUint16LE();
	if (size != in->size() - 2) {
		error("Size mismatch in MUS file (%ld versus reported %d)", (long)in->size() - 2, (int)size);
	}

	p->data = (byte *)calloc(size, 1);
	in->read(p->data, size);

	MidiParser *parser = MidiParser_createS1D();
	parser->setMidiDriver(this);
	parser->setTimerRate(_driver->getBaseTempo());
	if (!parser->loadMusic(p->data, size))
		error("Error reading track");

	if (!sfx) {
		_currentTrack = 255;
		resetVolumeTable();
	}
	p->parser = parser; // That plugs the power cord into the wall
}

} // End of namespace AGOS
