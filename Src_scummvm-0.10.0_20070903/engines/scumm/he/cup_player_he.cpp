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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/engines/scumm/he/cup_player_he.cpp $
 * $Id: cup_player_he.cpp 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

#include "common/stdafx.h"
#include "common/system.h"
#include "sound/audiostream.h"
#include "sound/mixer.h"
#include "scumm/scumm.h"
#include "scumm/util.h"
#include "scumm/he/intern_he.h"
#include "scumm/he/cup_player_he.h"

namespace Scumm {

CUP_Player::CUP_Player(OSystem *sys, ScummEngine_vCUPhe *vm, Audio::Mixer *mixer)
	: _vm(vm), _mixer(mixer), _system(sys) {
}

bool CUP_Player::open(const char *filename) {
	bool opened = false;
	debug(1, "opening '%s'", filename);
	if (_fileStream.open(filename)) {
		uint32 tag = _fileStream.readUint32BE();
		_fileStream.readUint32BE();
		if (tag == MKID_BE('BEAN')) {
			_playbackRate = kDefaultPlaybackRate;
			_width = kDefaultVideoWidth;
			_height = kDefaultVideoHeight;

			memset(_paletteData, 0, sizeof(_paletteData));
			_paletteChanged = false;
			_offscreenBuffer = 0;

			_inLzssBufData = 0;
			_inLzssBufSize = 0;
			_outLzssBufData = 0;
			_outLzssBufSize = 0;

			_dataSize = 0;

			_sfxCount = 0;
			_sfxBuffer = 0;
			for (int i = 0; i < kSfxChannels; ++i) {
				_sfxChannels[i].sfxNum = -1;
			}
			memset(_sfxQueue, 0, sizeof(_sfxQueue));
			_sfxQueuePos = 0;
			_lastSfxChannel = -1;

			_offscreenBuffer = (uint8 *)malloc(_width * _height);
			memset(_offscreenBuffer, 0, _width * _height);

			opened = true;
		}
	}
	return opened;
}

void CUP_Player::close() {
	_fileStream.close();
	free(_offscreenBuffer);
	_offscreenBuffer = 0;
	free(_inLzssBufData);
	_inLzssBufData = 0;
	free(_outLzssBufData);
	_outLzssBufData = 0;
	free(_sfxBuffer);
	_sfxBuffer = 0;
}

void CUP_Player::play() {
	while (parseNextHeaderTag(_fileStream)) {
		if (_fileStream.ioFailed()) {
			return;
		}
	}
	debug(1, "rate %d width %d height %d", _playbackRate, _width, _height);

	int ticks = _system->getMillis();
	while (_dataSize != 0 && !_vm->_quit) {
		while (parseNextBlockTag(_fileStream)) {
			if (_fileStream.ioFailed()) {
				return;
			}
		}
		int diff = _system->getMillis() - ticks;
		if (diff >= 0 && diff <= _playbackRate) {
			_system->delayMillis(_playbackRate - diff);
		} else {
			_system->delayMillis(1);
		}
		updateSfx();
		updateScreen();
		_vm->parseEvents();
		ticks = _system->getMillis();
	}
}

void CUP_Player::copyRectToScreen(const Common::Rect &r) {
	const uint8 *src = _offscreenBuffer + r.top * _width + r.left;
	_system->copyRectToScreen(src, _width, r.left, r.top, r.width() + 1, r.height() + 1);
}

void CUP_Player::updateScreen() {
	if (_paletteChanged) {
		_system->setPalette(_paletteData, 0, 256);
		_paletteChanged = false;
	}
	_system->updateScreen();
}

void CUP_Player::updateSfx() {
	int lastSfxChannel = _lastSfxChannel;
	for (int i = 0; i < _sfxQueuePos; ++i) {
		const CUP_Sfx *sfx = &_sfxQueue[i];
		if (sfx->num == -1) {
			debug(1, "Stopping sound channel %d", _lastSfxChannel);
			if (_lastSfxChannel != -1) {
				_mixer->stopHandle(_sfxChannels[_lastSfxChannel].handle);
			}
			continue;
		}
		if ((sfx->flags & kSfxFlagRestart) != 0) {
			for (int ch = 0; ch < kSfxChannels; ++ch) {
				if (_mixer->isSoundHandleActive(_sfxChannels[ch].handle) && _sfxChannels[ch].sfxNum == sfx->num) {
					_mixer->stopHandle(_sfxChannels[ch].handle);
					break;
				}
			}
		}
		CUP_SfxChannel *sfxChannel = 0;
		for (int ch = 0; ch < kSfxChannels; ++ch) {
			if (!_mixer->isSoundHandleActive(_sfxChannels[ch].handle)) {
				lastSfxChannel = ch;
				sfxChannel = &_sfxChannels[ch];
				sfxChannel->sfxNum = sfx->num;
				sfxChannel->flags = sfx->flags;
				break;
			}
		}
		if (sfxChannel) {
			debug(1, "Start sound %d channel %d flags 0x%X", sfx->num, lastSfxChannel, sfx->flags);
			int sfxIndex = sfxChannel->sfxNum - 1;
			assert(sfxIndex >= 0 && sfxIndex < _sfxCount);
			uint32 offset = READ_LE_UINT32(_sfxBuffer + sfxIndex * 4) - 8;
			uint8 *soundData = _sfxBuffer + offset;
			if (READ_BE_UINT32(soundData) == MKID_BE('DATA')) {
				uint32 soundSize = READ_BE_UINT32(soundData + 4);
				uint32 flags = Audio::Mixer::FLAG_UNSIGNED;
				uint32 loopEnd = 0;
				if (sfx->flags & kSfxFlagLoop) {
					flags |= Audio::Mixer::FLAG_LOOP;
					loopEnd = soundSize - 8;
				}
				_mixer->playInputStream(Audio::Mixer::kSFXSoundType, &sfxChannel->handle,
						Audio::makeLinearInputStream(soundData + 8, soundSize - 8, 11025, flags, 0, loopEnd));
			}
		} else {
			warning("Unable to find a free channel to play sound %d", sfx->num);
		}
	}
	_lastSfxChannel = lastSfxChannel;
	_sfxQueuePos = 0;
}

void CUP_Player::waitForSfxChannel(int channel) {
	assert(channel >= 0 && channel < kSfxChannels);
	CUP_SfxChannel *sfxChannel = &_sfxChannels[channel];
	debug(1, "waitForSfxChannel %d", channel);
	if ((sfxChannel->flags & kSfxFlagLoop) == 0) {
		while (_mixer->isSoundHandleActive(sfxChannel->handle) && !_vm->_quit) {
			_vm->parseEvents();
			_system->delayMillis(10);
		}
	}
}

bool CUP_Player::parseNextHeaderTag(Common::SeekableReadStream &dataStream) {
	uint32 tag = dataStream.readUint32BE();
	uint32 size = dataStream.readUint32BE() - 8;
	uint32 next = dataStream.pos() + size;
	debug(1, "New header tag %s %d dataSize %d", tag2str(tag), size, _dataSize);
	switch (tag) {
	case MKID_BE('HEAD'):
		handleHEAD(dataStream, size);
		break;
	case MKID_BE('SFXB'):
		handleSFXB(dataStream, size);
		break;
	case MKID_BE('RGBS'):
		handleRGBS(dataStream, size);
		break;
	case MKID_BE('DATA'):
		_dataSize = size;
		return false;
	case MKID_BE('GFXB'):
		// this is never triggered
	default:
		warning("Unhandled tag %s", tag2str(tag));
		break;
	}
	dataStream.seek(next);
	return true;
}

bool CUP_Player::parseNextBlockTag(Common::SeekableReadStream &dataStream) {
	uint32 tag = dataStream.readUint32BE();
	uint32 size = dataStream.readUint32BE() - 8;
	uint32 next = dataStream.pos() + size;
	debug(1, "New block tag %s %d dataSize %d", tag2str(tag), size, _dataSize);
	switch (tag) {
	case MKID_BE('FRAM'):
		handleFRAM(dataStream, size);
		break;
	case MKID_BE('LZSS'):
		if (handleLZSS(dataStream, size) && _outLzssBufSize != 0) {
			Common::MemoryReadStream memoryStream(_outLzssBufData, _outLzssBufSize);
			parseNextBlockTag(memoryStream);
		}
		break;
	case MKID_BE('RATE'):
		handleRATE(dataStream, size);
		break;
	case MKID_BE('RGBS'):
		handleRGBS(dataStream, size);
		break;
	case MKID_BE('SNDE'):
		handleSNDE(dataStream, size);
		break;
	case MKID_BE('TOIL'):
		handleTOIL(dataStream, size);
		break;
	case MKID_BE('SRLE'):
		handleSRLE(dataStream, size);
		break;
	case MKID_BE('BLOK'):
		_dataSize -= size + 8;
		return false;
	case MKID_BE('WRLE'):
		// this is never triggered
	default:
		warning("Unhandled tag %s", tag2str(tag));
		break;
	}
	dataStream.seek(next);
	return true;
}

void CUP_Player::handleHEAD(Common::SeekableReadStream &dataStream, uint32 dataSize) {
	_playbackRate = dataStream.readUint16LE();
	_width = dataStream.readUint16LE();
	_height = dataStream.readUint16LE();
}

void CUP_Player::handleSFXB(Common::SeekableReadStream &dataStream, uint32 dataSize) {
	if (dataSize > 16) { // WRAP and OFFS chunks
		uint32 tag = dataStream.readUint32BE();
		uint32 size = dataStream.readUint32BE();
		if (tag == MKID_BE('WRAP')) {
			tag = dataStream.readUint32BE();
			size = dataStream.readUint32BE();
			if (tag == MKID_BE('OFFS')) {
				_sfxCount = (size - 8) / 4;
				_sfxBuffer = (uint8 *)malloc(dataSize - 16);
				if (_sfxBuffer) {
					dataStream.read(_sfxBuffer, dataSize - 16);
				}
			}
		}
	}
}

void CUP_Player::handleRGBS(Common::SeekableReadStream &dataStream, uint32 dataSize) {
	for (int i = 0; i < 256; i++) {
		dataStream.read(&_paletteData[i * 4], 3);
	}
	_paletteChanged = true;
}

static void decodeTRLE(uint8 *dst, int dstPitch, Common::Rect &dstRect, Common::SeekableReadStream &dataStream) {
	dst += dstRect.top * dstPitch + dstRect.left;
	int h = dstRect.bottom - dstRect.top + 1;
	int w = dstRect.right - dstRect.left + 1;
	while (h--) {
		int lineSize = dataStream.readUint16LE();
		int nextLineOffset = dataStream.pos() + lineSize;
		uint8 *dstNextLine = dst + dstPitch;
		if (lineSize != 0) {
			uint8 *dstEnd = dst + w;
			while (dst < dstEnd) {
				int code = dataStream.readByte();
				if (code & 1) { // skip
					code >>= 1;
					dst += code;
				} else if (code & 2) { // set
					code = (code >> 2) + 1;
					const int sz = MIN<int>(code, dstEnd - dst);
					memset(dst, dataStream.readByte(), sz);
					dst += sz;
				} else { // copy
					code = (code >> 2) + 1;
					const int sz = MIN<int>(code, dstEnd - dst);
					dataStream.read(dst, sz);
					dst += sz;
				}
			}
		}
		dataStream.seek(nextLineOffset);
		dst = dstNextLine;
	}
}


void CUP_Player::handleFRAM(Common::SeekableReadStream &dataStream, uint32 dataSize) {
	const uint8 flags = dataStream.readByte();
	int type = 256;
	if (flags & 1) {
		type = dataStream.readByte();
	}
	Common::Rect r;
	if (flags & 2) {
		r.left   = dataStream.readUint16LE();
		r.top    = dataStream.readUint16LE();
		r.right  = dataStream.readUint16LE();
		r.bottom = dataStream.readUint16LE();
	}
	if (flags & 0x80) {
		if (type == 256) {
			decodeTRLE(_offscreenBuffer, _width, r, dataStream);
			copyRectToScreen(r);
		} else {
			warning("Unhandled FRAM type %d", type); // this is never triggered
		}
	}
}

static void decodeSRLE(uint8 *dst, const uint8 *colorMap, Common::SeekableReadStream &dataStream, int unpackedSize) {
	while (unpackedSize > 0) {
		int size, code = dataStream.readByte();
		if ((code & 1) == 0) {
			if ((code & 2) == 0) {
				size = (code >> 2) + 1;
				dst += size;
				unpackedSize -= size;
			} else {
				if ((code & 4) == 0) {
					*dst++ = colorMap[code >> 3];
					--unpackedSize;
				} else {
					code >>= 3;
					if (code == 0) {
						size = 1 + dataStream.readByte();
					} else {
						size = code;
					}
					memset(dst, dataStream.readByte(), MIN(unpackedSize, size));
					dst += size;
					unpackedSize -= size;
				}
			}
		} else {
			code >>= 1;
			if (code == 0) {
				code = 1 + dataStream.readUint16LE();
			}
			dst += code;
			unpackedSize -= code;
		}
	}
}

void CUP_Player::handleSRLE(Common::SeekableReadStream &dataStream, uint32 dataSize) {
	Common::Rect r;
	r.left   = dataStream.readUint16LE();
	r.top    = dataStream.readUint16LE();
	r.right  = dataStream.readUint16LE();
	r.bottom = dataStream.readUint16LE();
	uint8 colorMap[32];
	dataStream.read(colorMap, 32);
	int unpackedSize = dataStream.readUint32LE();
	decodeSRLE(_offscreenBuffer, colorMap, dataStream, unpackedSize);
	copyRectToScreen(r);
}

static void decodeLZSS(uint8 *dst, const uint8 *src1, const uint8 *src2, const uint8 *src3) {
	uint8 wnd[4096];
	int index = 1;
	while (1) {
		int code = *src1++;
		for (int b = 0; b < 8; ++b) {
			if (code & (1 << b)) {
				*dst++ = wnd[index] = *src2++;
				++index;
				index &= 0xFFF;
			} else {
				int cmd = READ_LE_UINT16(src3); src3 += 2;
				int count = (cmd >> 0xC) + 2;
				int offs = cmd & 0xFFF;
				if (offs == 0) {
					return;
				}
				while (count--) {
					*dst++ = wnd[index] = wnd[offs];
					++index;
					index &= 0xFFF;
					++offs;
					offs &= 0xFFF;
				}
			}
		}
	}
}

bool CUP_Player::handleLZSS(Common::SeekableReadStream &dataStream, uint32 dataSize) {
	uint32 tag = dataStream.readUint32BE();
	uint32 size = dataStream.readUint32BE();
	if (tag == MKID_BE('LZHD')) {
		uint32 compressionType = dataStream.readUint32LE();
		uint32 compressionSize = dataStream.readUint32LE();
		tag = dataStream.readUint32BE();
		size = dataStream.readUint32BE();
		if (tag == MKID_BE('DATA') && compressionType == 0x2000) {
			if (_inLzssBufSize < size - 16) {
				free(_inLzssBufData);
				_inLzssBufSize = size - 16;
				_inLzssBufData = (uint8 *)malloc(_inLzssBufSize);
			}
			if (_outLzssBufSize < compressionSize) {
				free(_outLzssBufData);
				_outLzssBufSize = compressionSize;
				_outLzssBufData = (uint8 *)malloc(_outLzssBufSize);
			}
			if (_inLzssBufData && _outLzssBufData) {
				uint32 offset1 = dataStream.readUint32LE() - 8;
				uint32 offset2 = dataStream.readUint32LE() - 8;
				dataStream.read(_inLzssBufData, size - 16);
				decodeLZSS(_outLzssBufData, _inLzssBufData, _inLzssBufData + offset1, _inLzssBufData + offset2);
				return true;
			}
		}
	}
	return false;
}

void CUP_Player::handleRATE(Common::SeekableReadStream &dataStream, uint32 dataSize) {
	const int rate = dataStream.readSint16LE();
	_playbackRate = CLIP(rate, 1, 4000);
}

void CUP_Player::handleSNDE(Common::SeekableReadStream &dataStream, uint32 dataSize) {
	assert(_sfxQueuePos < kSfxQueueSize);
	CUP_Sfx *sfx = &_sfxQueue[_sfxQueuePos];
	sfx->flags = dataStream.readUint32LE();
	sfx->num = dataStream.readUint16LE();
	dataStream.skip(2);
	uint16 loop = dataStream.readUint16LE();
	assert((loop & 0x8000) != 0); // this is never triggered
	++_sfxQueuePos;
}

void CUP_Player::handleTOIL(Common::SeekableReadStream &dataStream, uint32 dataSize) {
	int codesCount = dataStream.readUint16LE();
	while (codesCount != 0) {
		int codeSize = dataStream.readByte();
		if (codeSize == 0) {
			codeSize = dataStream.readUint16LE();
		}
		int code = dataStream.readByte();
		if (code == 0) {
			code = dataStream.readUint16LE();
		}
		switch (code) {
		case 1:
			for (int i = 0; i < kSfxChannels; ++i) {
				waitForSfxChannel(i);
			}
			_vm->_quit = true;
			break;
		case 7: {
				int channelSync = dataStream.readUint32LE();
				waitForSfxChannel(channelSync);
			}
			break;
		case 2: // display copyright/information messagebox
		case 3: // no-op in the original
		case 4: // restart playback
		case 5: // disable normal screen update
		case 6: // enable double buffer rendering
			// these are never triggered
		default:
			warning("Unhandled TOIL code=%d", code);
			break;
		}
		--codesCount;
	}
}

} // End of namespace Scumm
