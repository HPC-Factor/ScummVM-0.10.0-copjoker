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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/engines/scumm/smush/smush_player.cpp $
 * $Id: smush_player.cpp 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

#include "common/stdafx.h"

#include "engines/engine.h"

#include "common/config-manager.h"
#include "common/file.h"
#include "common/system.h"
#include "common/util.h"

#include "graphics/cursorman.h"

#include "scumm/bomp.h"
#include "scumm/file.h"
#include "scumm/imuse_digi/dimuse.h"
#include "scumm/imuse/imuse.h"
#include "scumm/scumm.h"
#include "scumm/sound.h"
#include "scumm/util.h"
#include "scumm/smush/channel.h"
#include "scumm/smush/chunk.h"
#include "scumm/smush/codec37.h"
#include "scumm/smush/codec47.h"
#include "scumm/smush/smush_font.h"
#include "scumm/smush/smush_mixer.h"
#include "scumm/smush/smush_player.h"

#include "scumm/insane/insane.h"

#include "sound/mixer.h"
#include "sound/vorbis.h"
#include "sound/mp3.h"

#ifdef DUMP_SMUSH_FRAMES
#include <png.h>
#endif

#ifdef USE_ZLIB
#include <zlib.h>
#endif

namespace Scumm {

static const int MAX_STRINGS = 200;
static const int ETRS_HEADER_LENGTH = 16;

class StringResource {
private:

	struct {
		int id;
		char *string;
	} _strings[MAX_STRINGS];

	int _nbStrings;
	int _lastId;
	const char *_lastString;

public:

	StringResource() :
		_nbStrings(0),
		_lastId(-1) {
	}
	~StringResource() {
		for (int32 i = 0; i < _nbStrings; i++) {
			delete []_strings[i].string;
		}
	}

	bool init(char *buffer, int32 length) {
		char *def_start = strchr(buffer, '#');
		while (def_start != NULL) {
			char *def_end = strchr(def_start, '\n');
			assert(def_end != NULL);

			char *id_end = def_end;
			while (id_end >= def_start && !isdigit(*(id_end-1))) {
				id_end--;
			}

			assert(id_end > def_start);
			char *id_start = id_end;
			while (isdigit(*(id_start - 1))) {
				id_start--;
			}

			char idstring[32];
			memcpy(idstring, id_start, id_end - id_start);
			idstring[id_end - id_start] = 0;
			int32 id = atoi(idstring);
			char *data_start = def_end;

			while (*data_start == '\n' || *data_start == '\r') {
				data_start++;
			}
			char *data_end = data_start;

			while (1) {
				if (data_end[-2] == '\r' && data_end[-1] == '\n' && data_end[0] == '\r' && data_end[1] == '\n') {
					break;
				}
				// In Russian Full Throttle strings are finished with
				// just one pair of CR-LF
				if (data_end[-2] == '\r' && data_end[-1] == '\n' && data_end[0] == '#') {
					break;
				}
				data_end++;
				if (data_end >= buffer + length) {
					data_end = buffer + length;
					break;
				}
			}

			data_end -= 2;
			assert(data_end > data_start);
			char *value = new char[data_end - data_start + 1];
			assert(value);
			memcpy(value, data_start, data_end - data_start);
			value[data_end - data_start] = 0;
			char *line_start = value;
			char *line_end;

			while ((line_end = strchr(line_start, '\n'))) {
				line_start = line_end+1;
				if (line_start[0] == '/' && line_start[1] == '/') {
					line_start += 2;
					if	(line_end[-1] == '\r')
						line_end[-1] = ' ';
					else
						*line_end++ = ' ';
					memmove(line_end, line_start, strlen(line_start)+1);
				}
			}
			_strings[_nbStrings].id = id;
			_strings[_nbStrings].string = value;
			_nbStrings ++;
			def_start = strchr(data_end + 2, '#');
		}
		return true;
	}

	const char *get(int id) {
		if (id == _lastId) {
			return _lastString;
		}
		debugC(DEBUG_SMUSH, "StringResource::get(%d)", id);
		for (int i = 0; i < _nbStrings; i++) {
			if (_strings[i].id == id) {
				_lastId = id;
				_lastString = _strings[i].string;
				return _strings[i].string;
			}
		}
		warning("invalid string id : %d", id);
		_lastId = -1;
		_lastString = "unknown string";
		return _lastString;
	}
};

static StringResource *getStrings(ScummEngine *vm, const char *file, bool is_encoded) {
	debugC(DEBUG_SMUSH, "trying to read text resources from %s", file);
	ScummFile theFile;

	vm->openFile(theFile, file);
	if (!theFile.isOpen()) {
		return 0;
	}
	int32 length = theFile.size();
	char *filebuffer = new char [length + 1];
	assert(filebuffer);
	theFile.read(filebuffer, length);
	filebuffer[length] = 0;

	if (is_encoded && READ_BE_UINT32(filebuffer) == MKID_BE('ETRS')) {
		assert(length > ETRS_HEADER_LENGTH);
		length -= ETRS_HEADER_LENGTH;
		for (int i = 0; i < length; ++i) {
			filebuffer[i] = filebuffer[i + ETRS_HEADER_LENGTH] ^ 0xCC;
		}
		filebuffer[length] = '\0';
	}
	StringResource *sr = new StringResource;
	assert(sr);
	sr->init(filebuffer, length);
	delete []filebuffer;
	return sr;
}

void SmushPlayer::timerCallback() {
	parseNextFrame();
#ifndef DISABLE_SMUSHREDRAW		// cybercop
#ifdef _WIN32_WCE
	_inTimer = true;
	_inTimerCount++;
#endif
#endif
}

SmushPlayer::SmushPlayer(ScummEngine_v7 *scumm) {
	_vm = scumm;
	_nbframes = 0;
	_codec37 = 0;
	_codec47 = 0;
	_smixer = NULL;
	_strings = NULL;
	_sf[0] = NULL;
	_sf[1] = NULL;
	_sf[2] = NULL;
	_sf[3] = NULL;
	_sf[4] = NULL;
	_base = NULL;
	_frameBuffer = NULL;
	_specialBuffer = NULL;

	_seekPos = -1;

	_skipNext = false;
	_dst = NULL;
	_storeFrame = false;
	_compressedFileMode = false;
	_width = 0;
	_height = 0;
	_IACTpos = 0;
	_speed = -1;
	_insanity = false;
	_middleAudio = false;
	_skipPalette = false;
	_IACTstream = NULL;
	_smixer = _vm->_smixer;
	_paused = false;
	_pauseStartTime = 0;
	_pauseTime = 0;
#ifndef DISABLE_SMUSHREDRAW		// cybercop
#ifdef _WIN32_WCE
	_inTimer = false;
	_inTimerCount = 0;
	_inTimerCountRedraw = ConfMan.getInt("Smush_force_redraw");
#endif
#endif
}

SmushPlayer::~SmushPlayer() {
}

void SmushPlayer::init(int32 speed) {
	_frame = 0;
	_speed = speed;
	_endOfFile = false;

	_vm->_smushVideoShouldFinish = false;
	_vm->_smushActive = true;

	_vm->setDirtyColors(0, 255);
	_dst = _vm->virtscr[0].getPixels(0, 0);

	// HACK HACK HACK: This is an *evil* trick, beware!
	// We do this to fix bug #1037052. A proper solution would change all the
	// drawing code to use the pitch value specified by the virtual screen.
	// However, since a lot of the SMUSH code currently assumes the screen
	// width and pitch to be equal, this will require lots of changes. So
	// we resort to this hackish solution for now.
	_origPitch = _vm->virtscr[0].pitch;
	_origNumStrips = _vm->_gdi->_numStrips;
	_vm->virtscr[0].pitch = _vm->virtscr[0].w;
	_vm->_gdi->_numStrips = _vm->virtscr[0].w / 8;

	_vm->_mixer->stopHandle(_compressedFileSoundHandle);
	_vm->_mixer->stopHandle(_IACTchannel);
	_IACTpos = 0;
	_vm->_smixer->stop();
}

void SmushPlayer::release() {
	_vm->_smushVideoShouldFinish = true;

	for (int i = 0; i < 5; i++) {
		delete _sf[i];
		_sf[i] = NULL;
	}

	delete _strings;
	_strings = NULL;

	delete _base;
	_base = NULL;

	free(_specialBuffer);
	_specialBuffer = NULL;

	free(_frameBuffer);
	_frameBuffer = NULL;

	_IACTstream = NULL;

	_vm->_smushActive = false;
	_vm->_fullRedraw = true;

	// HACK HACK HACK: This is an *evil* trick, beware! See above for
	// some explanation.
	_vm->virtscr[0].pitch = _origPitch;
	_vm->_gdi->_numStrips = _origNumStrips;

	delete _codec37;
	_codec37 = 0;
	delete _codec47;
	_codec47 = 0;
}

void SmushPlayer::checkBlock(const Chunk &b, Chunk::type type_expected, uint32 min_size) {
	if (type_expected != b.getType()) {
		error("Chunk type is different from expected : %x != %x", b.getType(), type_expected);
	}
	if (min_size > b.size()) {
		error("Chunk size is inferior than minimum required size : %d < %d", b.size(), min_size);
	}
}

void SmushPlayer::handleSoundBuffer(int32 track_id, int32 index, int32 max_frames, int32 flags, int32 vol, int32 pan, Chunk &b, int32 size) {
	debugC(DEBUG_SMUSH, "SmushPlayer::handleSoundBuffer(%d, %d)", track_id, index);
//	if ((flags & 128) == 128) {
//		return;
//	}
//	if ((flags & 64) == 64) {
//		return;
//	}
	SmushChannel *c = _smixer->findChannel(track_id);
	if (c == NULL) {
		c = new SaudChannel(track_id);
		_smixer->addChannel(c);
	}

	if (_middleAudio || index == 0) {
		c->setParameters(max_frames, flags, vol, pan, index);
	} else {
		c->checkParameters(index, max_frames, flags, vol, pan);
	}
	_middleAudio = false;
	c->appendData(b, size);
}

void SmushPlayer::handleSoundFrame(Chunk &b) {
	checkBlock(b, MKID_BE('PSAD'));
	debugC(DEBUG_SMUSH, "SmushPlayer::handleSoundFrame()");

	int32 track_id = b.readUint16LE();
	int32 index = b.readUint16LE();
	int32 max_frames = b.readUint16LE();
	int32 flags = b.readUint16LE();
	int32 vol = b.readByte();
	int32 pan = b.readSByte();
	if (index == 0) {
		debugC(DEBUG_SMUSH, "track_id:%d, max_frames:%d, flags:%d, vol:%d, pan:%d", track_id, max_frames, flags, vol, pan);
	}
	int32 size = b.size() - 10;
	handleSoundBuffer(track_id, index, max_frames, flags, vol, pan, b, size);
}

void SmushPlayer::handleStore(Chunk &b) {
	debugC(DEBUG_SMUSH, "SmushPlayer::handleStore()");
	checkBlock(b, MKID_BE('STOR'), 4);
	_storeFrame = true;
}

void SmushPlayer::handleFetch(Chunk &b) {
	debugC(DEBUG_SMUSH, "SmushPlayer::handleFetch()");
	checkBlock(b, MKID_BE('FTCH'), 6);

	if (_frameBuffer != NULL) {
		memcpy(_dst, _frameBuffer, _width * _height);
	}
}

void SmushPlayer::handleIACT(Chunk &b) {
	checkBlock(b, MKID_BE('IACT'), 8);
	debugC(DEBUG_SMUSH, "SmushPlayer::IACT()");

	int code = b.readUint16LE();
	int flags = b.readUint16LE();
	int unknown = b.readSint16LE();
	int track_flags = b.readUint16LE();

	if ((code != 8) && (flags != 46)) {
		_vm->_insane->procIACT(_dst, 0, 0, 0, b, 0, 0, code, flags, unknown, track_flags);
		return;
	}

	if (_compressedFileMode) {
		return;
	}

	assert(flags == 46 && unknown == 0);
	int track_id = b.readUint16LE();
	int index = b.readUint16LE();
	int nbframes = b.readUint16LE();
	int32 size = b.readUint32LE();
	int32 bsize = b.size() - 18;

	if (_vm->_game.id != GID_CMI) {
		int32 track = track_id;
		if (track_flags == 1) {
			track = track_id + 100;
		} else if (track_flags == 2) {
			track = track_id + 200;
		} else if (track_flags == 3) {
			track = track_id + 300;
		} else if ((track_flags >= 100) && (track_flags <= 163)) {
			track = track_id + 400;
		} else if ((track_flags >= 200) && (track_flags <= 263)) {
			track = track_id + 500;
		} else if ((track_flags >= 300) && (track_flags <= 363)) {
			track = track_id + 600;
		} else {
			error("SmushPlayer::handleIACT(): bad track_flags: %d", track_flags);
		}
		debugC(DEBUG_SMUSH, "SmushPlayer::handleIACT(): %d, %d, %d", track, index, track_flags);

		SmushChannel *c = _smixer->findChannel(track);
		if (c == 0) {
			c = new ImuseChannel(track);
			_smixer->addChannel(c);
		}
		if (index == 0)
			c->setParameters(nbframes, size, track_flags, unknown, 0);
		else
			c->checkParameters(index, nbframes, size, track_flags, unknown);
		c->appendData(b, bsize);
	} else {
		// TODO: Move this code into another SmushChannel subclass?
		byte *src = (byte *)malloc(bsize);
		b.read(src, bsize);
		byte *d_src = src;
		byte value;

		while (bsize > 0) {
			if (_IACTpos >= 2) {
				int32 len = READ_BE_UINT16(_IACToutput) + 2;
				len -= _IACTpos;
				if (len > bsize) {
					memcpy(_IACToutput + _IACTpos, d_src, bsize);
					_IACTpos += bsize;
					bsize = 0;
				} else {
					byte *output_data = new byte[4096];

					memcpy(_IACToutput + _IACTpos, d_src, len);
					byte *dst = output_data;
					byte *d_src2 = _IACToutput;
					d_src2 += 2;
					int32 count = 1024;
					byte variable1 = *d_src2++;
					byte variable2 = variable1 / 16;
					variable1 &= 0x0f;
					do {
						value = *(d_src2++);
						if (value == 0x80) {
							*dst++ = *d_src2++;
							*dst++ = *d_src2++;
						} else {
							int16 val = (int8)value << variable2;
							*dst++ = val >> 8;
							*dst++ = (byte)(val);
						}
						value = *(d_src2++);
						if (value == 0x80) {
							*dst++ = *d_src2++;
							*dst++ = *d_src2++;
						} else {
							int16 val = (int8)value << variable1;
							*dst++ = val >> 8;
							*dst++ = (byte)(val);
						}
					} while (--count);

					if (!_IACTstream) {
						_IACTstream = Audio::makeAppendableAudioStream(22050, Audio::Mixer::FLAG_STEREO | Audio::Mixer::FLAG_16BITS);
						_vm->_mixer->playInputStream(Audio::Mixer::kSFXSoundType, &_IACTchannel, _IACTstream);
					}
					_IACTstream->queueBuffer(output_data, 0x1000);

					bsize -= len;
					d_src += len;
					_IACTpos = 0;
				}
			} else {
				if (bsize > 1 && _IACTpos == 0) {
					*(_IACToutput + 0) = *d_src++;
					_IACTpos = 1;
					bsize--;
				}
				*(_IACToutput + _IACTpos) = *d_src++;
				_IACTpos++;
				bsize--;
			}
		}

		free(src);
	}
}

void SmushPlayer::handleTextResource(Chunk &b) {
	int pos_x = b.readSint16LE();
	int pos_y = b.readSint16LE();
	int flags = b.readSint16LE();
	int left = b.readSint16LE();
	int top = b.readSint16LE();
	int right = b.readSint16LE();
	/*int32 height =*/ b.readSint16LE();
	/*int32 unk2 =*/ b.readUint16LE();

	const char *str;
	char *string = NULL, *string2 = NULL;
	if (b.getType() == MKID_BE('TEXT')) {
		string = (char *)malloc(b.size() - 16);
		str = string;
		b.read(string, b.size() - 16);
	} else {
		int string_id = b.readUint16LE();
		if (!_strings)
			return;
		str = _strings->get(string_id);
	}

	// if subtitles disabled and bit 3 is set, then do not draw
	//
	// Query ConfMan here. However it may be slower, but
	// player may want to switch the subtitles on or off during the
	// playback. This fixes bug #1550974
	if ((!ConfMan.getBool("subtitles")) && ((flags & 8) == 8))
		return;

	SmushFont *sf = getFont(0);
	int color = 15;
	while (*str == '/') {
		str++; // For Full Throttle text resources
	}

	byte transBuf[512];
	if (_vm->_game.id == GID_CMI) {
		_vm->translateText((const byte *)str - 1, transBuf);
		while (*str++ != '/')
			;
		string2 = (char *)transBuf;

		// If string2 contains formatting information there probably
		// wasn't any translation for it in the language.tab file. In
		// that case, pretend there is no string2.
		if (string2[0] == '^')
			string2[0] = 0;
	}

	while (str[0] == '^') {
		switch (str[1]) {
		case 'f':
			{
				int id = str[3] - '0';
				str += 4;
				sf = getFont(id);
			}
			break;
		case 'c':
			{
				color = str[4] - '0' + 10 *(str[3] - '0');
				str += 5;
			}
			break;
		default:
			error("invalid escape code in text string");
		}
	}

	// HACK. This is to prevent bug #1310846. In updated Win95 dig
	// there is such line:
	//
	// ^f01^c001LEAD TESTER
	// Chris Purvis
	// ^f01
	// ^f01^c001WINDOWS COMPATIBILITY
	// Chip Hinnenberg
	// ^f01^c001WINDOWS TESTING
	// Jim Davison
	// Lynn Selk
	//
	// i.e. formatting exists not in the first line only
	// We just strip that off and assume that neither font
	// nor font color was altered. Proper fix would be to feed
	// drawString() with each line sequentally
	char *string3 = NULL, *sptr2;
	const char *sptr;

	if (strchr(str, '^')) {
		string3 = (char *)malloc(strlen(str) + 1);

		for (sptr = str, sptr2 = string3; *sptr;) {
			if (*sptr == '^') {
				switch (sptr[1]) {
				case 'f':
					sptr += 4;
					break;
				case 'c':
					sptr += 5;
					break;
				default:
					error("invalid escape code in text string");
				}
			} else {
				*sptr2++ = *sptr++;
			}
		}
		*sptr2++ = *sptr++; // copy zero character
		str = string3;
	}

	assert(sf != NULL);
	sf->setColor(color);

	if (_vm->_game.id == GID_CMI && string2[0] != 0) {
		str = string2;
	}

	// flags:
	// bit 0 - center       1
	// bit 1 - not used     2
	// bit 2 - ???          4
	// bit 3 - wrap around  8
	switch (flags & 9) {
	case 0:
		sf->drawString(str, _dst, _width, _height, pos_x, pos_y, false);
		break;
	case 1:
		sf->drawString(str, _dst, _width, _height, pos_x, MAX(pos_y, top), true);
		break;
	case 8:
		// FIXME: Is 'right' the maximum line width here, just
		// as it is in the next case? It's used several times
		// in The Dig's intro, where 'left' and 'right' are
		// always 0 and 321 respectively, and apparently we
		// handle that correctly.
		sf->drawStringWrap(str, _dst, _width, _height, pos_x, MAX(pos_y, top), left, right, false);
		break;
	case 9:
		// In this case, the 'right' parameter is actually the
		// maximum line width. This explains why it's sometimes
		// smaller than 'left'.
		//
		// Note that in The Dig's "Spacetime Six" movie it's
		// 621. I have no idea what that means.
		sf->drawStringWrap(str, _dst, _width, _height, pos_x, MAX(pos_y, top), left, MIN(left + right, _width), true);
		break;
	default:
		error("SmushPlayer::handleTextResource. Not handled flags: %d", flags);
	}

	if (string != NULL) {
		free (string);
	}
	if (string3 != NULL) {
		free (string3);
	}
}

const char *SmushPlayer::getString(int id) {
	return _strings->get(id);
}

bool SmushPlayer::readString(const char *file) {
	const char *i = strrchr(file, '.');
	if (i == NULL) {
		error("invalid filename : %s", file);
	}
	char fname[260];
	memcpy(fname, file, i - file);
	strcpy(fname + (i - file), ".trs");
	if ((_strings = getStrings(_vm, fname, false)) != 0) {
		return true;
	}

	if (_vm->_game.id == GID_DIG && (_strings = getStrings(_vm, "digtxt.trs", true)) != 0) {
		return true;
	}
	return false;
}

void SmushPlayer::readPalette(byte *out, Chunk &in) {
	in.read(out, 0x300);
}

static byte delta_color(byte org_color, int16 delta_color) {
	int t = (org_color * 129 + delta_color) / 128;
	return CLIP(t, 0, 255);
}

void SmushPlayer::handleDeltaPalette(Chunk &b) {
	checkBlock(b, MKID_BE('XPAL'));
	debugC(DEBUG_SMUSH, "SmushPlayer::handleDeltaPalette()");

	if (b.size() == 0x300 * 3 + 4) {

		b.readUint16LE();
		b.readUint16LE();

		for (int i = 0; i < 0x300; i++) {
			_deltaPal[i] = b.readUint16LE();
		}
		readPalette(_pal, b);
		setDirtyColors(0, 255);
	} else if (b.size() == 6) {

		b.readUint16LE();
		b.readUint16LE();
		b.readUint16LE();

		for (int i = 0; i < 0x300; i++) {
			_pal[i] = delta_color(_pal[i], _deltaPal[i]);
		}
		setDirtyColors(0, 255);
	} else {
		error("SmushPlayer::handleDeltaPalette() Wrong size for DeltaPalette");
	}
}

void SmushPlayer::handleNewPalette(Chunk &b) {
	checkBlock(b, MKID_BE('NPAL'), 0x300);
	debugC(DEBUG_SMUSH, "SmushPlayer::handleNewPalette()");

	if (_skipPalette)
		return;

	readPalette(_pal, b);
	setDirtyColors(0, 255);
}

void smush_decode_codec1(byte *dst, const byte *src, int left, int top, int width, int height, int pitch);

void SmushPlayer::decodeFrameObject(int codec, const uint8 *src, int left, int top, int width, int height) {
	if ((height == 242) && (width == 384)) {
		if (_specialBuffer == 0)
			_specialBuffer = (byte *)malloc(242 * 384);
		_dst = _specialBuffer;
	} else if ((height > _vm->_screenHeight) || (width > _vm->_screenWidth))
		return;
	// FT Insane uses smaller frames to draw overlays with moving objects
	// Other .san files do have them as well but their purpose in unknown
	// and often it causes memory overdraw. So just skip those frames
	else if (!_insanity && ((height != _vm->_screenHeight) || (width != _vm->_screenWidth)))
		return;

	if ((height == 242) && (width == 384)) {
		_width = width;
		_height = height;
	} else {
		_width = _vm->_screenWidth;
		_height = _vm->_screenHeight;
	}

	switch (codec) {
	case 1:
	case 3:
		smush_decode_codec1(_dst, src, left, top, width, height, _vm->_screenWidth);
		break;
	case 37:
		if (!_codec37)
			_codec37 = new Codec37Decoder(width, height);
		if (_codec37)
			_codec37->decode(_dst, src);
		break;
	case 47:
		if (!_codec47)
			_codec47 = new Codec47Decoder(width, height);
		if (_codec47)
			_codec47->decode(_dst, src);
		break;
	default:
		error("Invalid codec for frame object : %d", codec);
	}

	if (_storeFrame) {
		if (_frameBuffer == NULL) {
			_frameBuffer = (byte *)malloc(_width * _height);
		}
		memcpy(_frameBuffer, _dst, _width * _height);
		_storeFrame = false;
	}
}

#ifdef USE_ZLIB
void SmushPlayer::handleZlibFrameObject(Chunk &b) {
	if (_skipNext) {
		_skipNext = false;
		return;
	}

	int32 chunkSize = b.size();
	byte *chunkBuffer = (byte *)malloc(chunkSize);
	assert(chunkBuffer);
	b.read(chunkBuffer, chunkSize);

	unsigned long decompressedSize = READ_BE_UINT32(chunkBuffer);
	byte *fobjBuffer = (byte *)malloc(decompressedSize);
	int result = uncompress(fobjBuffer, &decompressedSize, chunkBuffer + 4, chunkSize - 4);
	if (result != Z_OK)
		error("SmushPlayer::handleZlibFrameObject() Zlib uncompress error");
	free(chunkBuffer);

	byte *ptr = fobjBuffer;
	int codec = READ_LE_UINT16(ptr); ptr += 2;
	int left = READ_LE_UINT16(ptr); ptr += 2;
	int top = READ_LE_UINT16(ptr); ptr += 2;
	int width = READ_LE_UINT16(ptr); ptr += 2;
	int height = READ_LE_UINT16(ptr); ptr += 2;

	decodeFrameObject(codec, fobjBuffer + 14, left, top, width, height);

	free(fobjBuffer);
}
#endif

void SmushPlayer::handleFrameObject(Chunk &b) {
	checkBlock(b, MKID_BE('FOBJ'), 14);
	if (_skipNext) {
		_skipNext = false;
		return;
	}

	int codec = b.readUint16LE();
	int left = b.readUint16LE();
	int top = b.readUint16LE();
	int width = b.readUint16LE();
	int height = b.readUint16LE();

	b.readUint16LE();
	b.readUint16LE();

	int32 chunk_size = b.size() - 14;
	byte *chunk_buffer = (byte *)malloc(chunk_size);
	assert(chunk_buffer);
	b.read(chunk_buffer, chunk_size);

	decodeFrameObject(codec, chunk_buffer, left, top, width, height);

	free(chunk_buffer);
}

void SmushPlayer::handleFrame(Chunk &b) {
	checkBlock(b, MKID_BE('FRME'));
	debugC(DEBUG_SMUSH, "SmushPlayer::handleFrame(%d)", _frame);
	_skipNext = false;

	if (_insanity) {
		_vm->_insane->procPreRendering();
	}

	while (!b.eos()) {
		Chunk *sub = b.subBlock();
		switch (sub->getType()) {
		case MKID_BE('NPAL'):
			handleNewPalette(*sub);
			break;
		case MKID_BE('FOBJ'):
			handleFrameObject(*sub);
			break;
#ifdef USE_ZLIB
		case MKID_BE('ZFOB'):
			handleZlibFrameObject(*sub);
			break;
#endif
		case MKID_BE('PSAD'):
			if (!_compressedFileMode)
				handleSoundFrame(*sub);
			break;
		case MKID_BE('TRES'):
			handleTextResource(*sub);
			break;
		case MKID_BE('XPAL'):
			handleDeltaPalette(*sub);
			break;
		case MKID_BE('IACT'):
			handleIACT(*sub);
			break;
		case MKID_BE('STOR'):
			handleStore(*sub);
			break;
		case MKID_BE('FTCH'):
			handleFetch(*sub);
			break;
		case MKID_BE('SKIP'):
			_vm->_insane->procSKIP(*sub);
			break;
		case MKID_BE('TEXT'):
			handleTextResource(*sub);
			break;
		default:
			error("Unknown frame subChunk found : %s, %d", tag2str(sub->getType()), sub->size());
		}

		b.reseek();
		if (sub->size() & 1)
			b.skip(1);

		delete sub;
	}

	if (_insanity) {
		_vm->_insane->procPostRendering(_dst, 0, 0, 0, _frame, _nbframes-1);
	}

	if (_width != 0 && _height != 0) {
#ifndef DISABLE_SMUSHREDRAW		// cybercop
#ifdef _WIN32_WCE
		if (!_inTimer || _inTimerCount == _inTimerCountRedraw) {
			updateScreen();
			_inTimerCount = 0;
		}
#else
		updateScreen();
#endif
#else
		updateScreen();
#endif
	}
	_smixer->handleFrame();

	_frame++;
}

void SmushPlayer::handleAnimHeader(Chunk &b) {
	checkBlock(b, MKID_BE('AHDR'), 0x300 + 6);
	debugC(DEBUG_SMUSH, "SmushPlayer::handleAnimHeader()");

	/* _version = */ b.readUint16LE();
	_nbframes = b.readUint16LE();
	b.readUint16LE();

	if (_skipPalette)
		return;

	readPalette(_pal, b);
	setDirtyColors(0, 255);
}

void SmushPlayer::setupAnim(const char *file) {
	if (_insanity) {
		if (!((_vm->_game.features & GF_DEMO) && (_vm->_game.platform == Common::kPlatformPC)))
			readString("mineroad.trs");
	} else
		readString(file);
}

SmushFont *SmushPlayer::getFont(int font) {
	char file_font[11];

	if (_sf[font])
		return _sf[font];

	if (_vm->_game.id == GID_FT) {
		if (!((_vm->_game.features & GF_DEMO) && (_vm->_game.platform == Common::kPlatformPC))) {
			const char *ft_fonts[] = {
				"scummfnt.nut",
				"techfnt.nut",
				"titlfnt.nut",
				"specfnt.nut"
			};

			assert(font >= 0 && font < ARRAYSIZE(ft_fonts));

			_sf[font] = new SmushFont(_vm, ft_fonts[font], true, false);
		}
	} else if (_vm->_game.id == GID_DIG) {
		if (!(_vm->_game.features & GF_DEMO)) {
			assert(font >= 0 && font < 4);

			sprintf(file_font, "font%d.nut", font);
			_sf[font] = new SmushFont(_vm, file_font, font != 0, false);
		}
	} else if (_vm->_game.id == GID_CMI) {
		int numFonts = (_vm->_game.features & GF_DEMO) ? 4 : 5;
		assert(font >= 0 && font < numFonts);

		sprintf(file_font, "font%d.nut", font);
		_sf[font] = new SmushFont(_vm, file_font, false, true);
	} else {
		error("SmushPlayer::getFont() Unknown font setup for game");
	}

	assert(_sf[font]);
	return _sf[font];
}

void SmushPlayer::parseNextFrame() {
	Chunk *sub;

	if (_seekPos >= 0) {
		if (_smixer)
			_smixer->stop();

		if (_seekFile.size() > 0) {
			delete _base;
			_base = new FileChunk(_seekFile);

			if (_seekPos > 0) {
				assert(_seekPos > 8);
				// In this case we need to get palette and number of frames
				sub = _base->subBlock();
				checkBlock(*sub, MKID_BE('AHDR'));
				handleAnimHeader(*sub);
				delete sub;

				_middleAudio = true;
				_seekPos -= 8;
			} else {
				// We need this in Full Throttle when entering/leaving
				// the old mine road.
				tryCmpFile(_seekFile.c_str());
			}
			_skipPalette = false;
		} else {
			_skipPalette = true;
		}

		_base->seek(_seekPos, SEEK_SET);
		_frame = _seekFrame;
		_startFrame = _frame;
		_startTime = _vm->_system->getMillis();

		_seekPos = -1;
	}

	assert(_base);
	if (_base->eos()) {
		_vm->_smushVideoShouldFinish = true;
		_endOfFile = true;
		return;
	}

	sub = _base->subBlock();

	switch (sub->getType()) {
	case MKID_BE('AHDR'): // FT INSANE may seek file to the beginning
		handleAnimHeader(*sub);
		break;
	case MKID_BE('FRME'):
		handleFrame(*sub);
		break;
	default:
		error("Unknown Chunk found at %x: %x, %d", _base->pos(), sub->getType(), sub->size());
	}
	delete sub;

	_base->reseek();

	if (_insanity)
		_vm->_sound->processSound();

	_vm->_imuseDigital->flushTracks();
}

void SmushPlayer::setPalette(const byte *palette) {
	memcpy(_pal, palette, 0x300);
	setDirtyColors(0, 255);
}

void SmushPlayer::setPaletteValue(int n, byte r, byte g, byte b) {
	_pal[n * 3 + 0] = r;
	_pal[n * 3 + 1] = g;
	_pal[n * 3 + 2] = b;
	setDirtyColors(n, n);
}

void SmushPlayer::setDirtyColors(int min, int max) {
	if (_palDirtyMin > min)
		_palDirtyMin = min;
	if (_palDirtyMax < max)
		_palDirtyMax = max;
}

void SmushPlayer::warpMouse(int x, int y, int buttons) {
	_warpNeeded = true;
	_warpX = x;
	_warpY = y;
	_warpButtons = buttons;
}

void SmushPlayer::updateScreen() {
#ifdef DUMP_SMUSH_FRAMES
	char fileName[100];
	// change path below for dump png files
	sprintf(fileName, "/path/to/somethere/%s%04d.png", _vm->getBaseName(), _frame);
	FILE *file = fopen(fileName, "wb");
	if (file == NULL)
		error("can't open file for writing png");

	png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
	if (png_ptr == NULL) {
		fclose(file);
		error("can't write png header");
	}
	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL) {
		fclose(file);
		error("can't create png info struct");
	}
	if (setjmp(png_ptr->jmpbuf)) {
		fclose(file);
		error("png jmpbuf error");
	}

	png_init_io(png_ptr, file);

	png_set_IHDR(png_ptr, info_ptr, _width, _height, 8, PNG_COLOR_TYPE_PALETTE, PNG_INTERLACE_NONE,
							PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

	png_colorp palette = (png_colorp)png_malloc(png_ptr, PNG_MAX_PALETTE_LENGTH * sizeof (png_color));
	for (int i = 0; i != 256; ++i) {
		(palette + i)->red = _pal[i * 3 + 0];
		(palette + i)->green = _pal[i * 3 + 1];
		(palette + i)->blue = _pal[i * 3 + 2];
	}

	png_set_PLTE(png_ptr, info_ptr, palette, PNG_MAX_PALETTE_LENGTH);

	png_write_info(png_ptr, info_ptr);
	png_set_flush(png_ptr, 10);

	png_bytep row_pointers[480];
	for (int y = 0 ; y < _height ; y++)
		row_pointers[y] = (png_byte *) (_dst + y * _width);
	png_write_image(png_ptr, row_pointers);
	png_write_end(png_ptr, info_ptr);
	png_free(png_ptr, palette);

	fclose(file);
	png_destroy_write_struct(&png_ptr, &info_ptr);
#endif

	uint32 end_time, start_time = _vm->_system->getMillis();
	_updateNeeded = true;
	end_time = _vm->_system->getMillis();
	debugC(DEBUG_SMUSH, "Smush stats: updateScreen( %03d )", end_time - start_time);
}

void SmushPlayer::insanity(bool flag) {
	_insanity = flag;
}

void SmushPlayer::seekSan(const char *file, int32 pos, int32 contFrame) {
	_seekFile = file ? file : "";
	_seekPos = pos;
	_seekFrame = contFrame;
	_pauseTime = 0;
}

void SmushPlayer::tryCmpFile(const char *filename) {
	_vm->_mixer->stopHandle(_compressedFileSoundHandle);

	_compressedFileMode = false;
	const char *i = strrchr(filename, '.');
	if (i == NULL) {
		error("invalid filename : %s", filename);
	}
#if defined(USE_MAD) || defined(USE_VORBIS)
	char fname[260];
#endif
	Common::File *file = new Common::File();
#ifdef USE_VORBIS
	memcpy(fname, filename, i - filename);
	strcpy(fname + (i - filename), ".ogg");
	if (file->open(fname)) {
		_compressedFileMode = true;
		_vm->_mixer->playInputStream(Audio::Mixer::kSFXSoundType, &_compressedFileSoundHandle, Audio::makeVorbisStream(file, true, 0, 0));
		return;
	}
#endif
#ifdef USE_MAD
	memcpy(fname, filename, i - filename);
	strcpy(fname + (i - filename), ".mp3");
	if (file->open(fname)) {
		_compressedFileMode = true;
		_vm->_mixer->playInputStream(Audio::Mixer::kSFXSoundType, &_compressedFileSoundHandle, Audio::makeMP3Stream(file, true, 0, 0));
		return;
	}
#endif
	delete file;
}

void SmushPlayer::pause() {
	if (!_paused) {
		_paused = true;
		_pauseStartTime = _vm->_system->getMillis();
	}
}

void SmushPlayer::unpause() {
	if (_paused) {
		_paused = false;
		_pauseTime += (_vm->_system->getMillis() - _pauseStartTime);
		_pauseStartTime = 0;
	}
}

void SmushPlayer::play(const char *filename, int32 speed, int32 offset, int32 startFrame) {
	// Verify the specified file exists
	ScummFile f;
	_vm->openFile(f, filename);
	if (!f.isOpen()) {
		warning("SmushPlayer::play() File not found %s", filename);
		return;
	}
	f.close();

	_updateNeeded = false;
	_warpNeeded = false;
	_palDirtyMin = 256;
	_palDirtyMax = -1;

	// Hide mouse
	bool oldMouseState = CursorMan.showMouse(false);

	// Load the video
	_seekFile = filename;
	_seekPos = offset;
	_seekFrame = startFrame;
	_base = 0;

	setupAnim(filename);
	init(speed);

	_startTime = _vm->_system->getMillis();
	_startFrame = startFrame;
	_frame = startFrame;

	_pauseTime = 0;

	int skipped = 0;

	for (;;) {
		uint32 now, elapsed;
		bool skipFrame = false;

		if (_insanity) {
			// Seeking makes a mess of trying to sync the audio to
			// the sound. Synt to time instead.
			now = _vm->_system->getMillis() - _pauseTime;
			elapsed = now - _startTime;
		} else if (_vm->_mixer->isSoundHandleActive(_compressedFileSoundHandle)) {
			// Compressed SMUSH files.
			elapsed = _vm->_mixer->getSoundElapsedTime(_compressedFileSoundHandle);
		} else if (_vm->_mixer->isSoundHandleActive(_IACTchannel)) {
			// Curse of Monkey Island SMUSH files.
			elapsed = _vm->_mixer->getSoundElapsedTime(_IACTchannel);
		} else {
			// For other SMUSH files, we don't necessarily have any
			// one channel to sync against, so we have to use
			// elapsed real time.
			now = _vm->_system->getMillis() - _pauseTime;
			elapsed = now - _startTime;
		}

		if (elapsed >= ((_frame - _startFrame) * 1000) / _speed) {
			if (elapsed >= ((_frame + 1) * 1000) / _speed)
				skipFrame = true;
			else
				skipFrame = false;
			timerCallback();
		}

		if (_warpNeeded) {
			_vm->_system->warpMouse(_warpX, _warpY);
			_warpNeeded = false;
		}
		_vm->parseEvents();
		_vm->processInput();
		if (_palDirtyMax >= _palDirtyMin) {
			byte palette_colors[1024];
			byte *p = palette_colors;

			for (int i = _palDirtyMin; i <= _palDirtyMax; i++) {
				byte *data = _pal + i * 3;

				*p++ = data[0];
				*p++ = data[1];
				*p++ = data[2];
				*p++ = 0;
			}

			_vm->_system->setPalette(palette_colors, _palDirtyMin, _palDirtyMax - _palDirtyMin + 1);

			_palDirtyMax = -1;
			_palDirtyMin = 256;
			skipFrame = false;
		}
		if (skipFrame) {
			if (++skipped > 10) {
				skipFrame = false;
				skipped = 0;
			}
		} else
			skipped = 0;
		if (_updateNeeded) {
			if (!skipFrame) {
				// Workaround for bug #1386333: "FT DEMO: assertion triggered
				// when playing movie". Some frames there are 384 x 224
				int w = MIN(_width, _vm->_screenWidth);
				int h = MIN(_height, _vm->_screenHeight);

				_vm->_system->copyRectToScreen(_dst, _width, 0, 0, w, h);
				_vm->_system->updateScreen();
				_updateNeeded = false;
			}
#ifndef DISABLE_SMUSHREDRAW		// cybercop
#ifdef _WIN32_WCE
			_inTimer = false;
			_inTimerCount = 0;
#endif
#endif
		}
		if (_endOfFile)
			break;
		if (_vm->_quit || _vm->_saveLoadFlag || _vm->_smushVideoShouldFinish) {
			_smixer->stop();
			_vm->_mixer->stopHandle(_compressedFileSoundHandle);
			_vm->_mixer->stopHandle(_IACTchannel);
			_IACTpos = 0;
			break;
		}
		_vm->_system->delayMillis(10);
	}

	release();

	// Reset mouse state
	CursorMan.showMouse(oldMouseState);
}

} // End of namespace Scumm
