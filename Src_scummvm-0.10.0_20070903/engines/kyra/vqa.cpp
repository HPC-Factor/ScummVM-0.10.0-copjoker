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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/engines/kyra/vqa.cpp $
 * $Id: vqa.cpp 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

// Player for Kyrandia 3 VQA movies, based on the information found at
// http://multimedia.cx/VQA_INFO.TXT
//
// The benchl.vqa movie (or whatever it is) is not supported. It does not have
// a FINF chunk.
//
// The jung2.vqa movie does work, but only thanks to a grotesque hack.

#include "common/stdafx.h"
#include "common/events.h"
#include "common/system.h"
#include "sound/audiostream.h"
#include "sound/mixer.h"
#include "kyra/sound.h"
#include "kyra/screen.h"
#include "kyra/vqa.h"

namespace Kyra {

VQAMovie::VQAMovie(KyraEngine *vm, OSystem *system) {
	_system = system;
	_vm = vm;
	_opened = false;
	_x = _y = _drawPage = -1;
}

VQAMovie::~VQAMovie() {
	close();
}

void VQAMovie::initBuffers() {
	for (int i = 0; i < ARRAYSIZE(_buffers); i++) {
		_buffers[i].data = 0;
		_buffers[i].size = 0;
	}
}

void *VQAMovie::allocBuffer(int num, uint32 size) {
	assert(num >= 0 && num < ARRAYSIZE(_buffers));
	assert(size > 0);

	if (size > _buffers[num].size) {
		/*
		 * We could use realloc() here, but we don't actually need the
		 * old contents of the buffer.
		 */
		delete [] _buffers[num].data;
		_buffers[num].data = new uint8[size];
		_buffers[num].size = size;
	}

	assert(_buffers[num].data);

	return _buffers[num].data;
}

void VQAMovie::freeBuffers() {
	for (int i = 0; i < ARRAYSIZE(_buffers); i++) {
		delete [] _buffers[i].data;
		_buffers[i].data = NULL;
		_buffers[i].size = 0;
	}
}

uint32 VQAMovie::readTag() {
	// Some tags have to be on an even offset, so they are padded with a
	// zero byte. Skip that.

	uint32 tag = _file.readUint32BE();

	if (!(tag & 0xFF000000)) {
		tag = (tag << 8) | _file.readByte();
	}

	return tag;
}

void VQAMovie::decodeSND1(byte *inbuf, uint32 insize, byte *outbuf, uint32 outsize) {
	const int8 WSTable2Bit[] = { -2, -1, 0, 1 };
	const int8 WSTable4Bit[] = {
		-9, -8, -6, -5, -4, -3, -2, -1,
		 0,  1,  2,  3,  4,  5,  6,  8
	};

	byte code;
	int8 count;
	uint16 input;

	int16 curSample = 0x80;

	while (outsize > 0) {
		input = *inbuf++ << 2;
		code = (input >> 8) & 0xff;
		count = (input & 0xff) >> 2;

		switch (code) {
		case 2:
			if (count & 0x20) {
				/* NOTE: count is signed! */
				count <<= 3;
				curSample += (count >> 3);
				*outbuf++ = curSample;
				outsize--;
			} else {
				for (; count >= 0; count--) {
					*outbuf++ = *inbuf++;
					outsize--;
				}
				curSample = *(outbuf - 1);
			}
			break;
		case 1:
			for (; count >= 0; count--) {
				code = *inbuf++;

				curSample += WSTable4Bit[code & 0x0f];
				curSample = CLIP<int16>(curSample, 0, 255);
				*outbuf++ = curSample;

				curSample += WSTable4Bit[code >> 4];
				curSample = CLIP<int16>(curSample, 0, 255);
				*outbuf++ = curSample;

				outsize -= 2;
			}
			break;
		case 0:
			for (; count >= 0; count--) {
				code = *inbuf++;

				curSample += WSTable2Bit[code & 0x03];
				curSample = CLIP<int16>(curSample, 0, 255);
				*outbuf++ = curSample;

				curSample += WSTable2Bit[(code >> 2) & 0x03];
				curSample = CLIP<int16>(curSample, 0, 255);
				*outbuf++ = curSample;

				curSample += WSTable2Bit[(code >> 4) & 0x03];
				curSample = CLIP<int16>(curSample, 0, 255);
				*outbuf++ = curSample;

				curSample += WSTable2Bit[(code >> 6) & 0x03];
				curSample = CLIP<int16>(curSample, 0, 255);
				*outbuf++ = curSample;

				outsize -= 4;
			}
			break;
		default:
			for (; count >= 0; count--) {
				*outbuf++ = curSample;
				outsize--;
			}
		}
	}
}

bool VQAMovie::open(const char *filename) {
	debugC(9, kDebugLevelMovie, "VQAMovie::open('%s')", filename);
	close();

	if (!_file.open(filename))
		return false;

	if (_file.readUint32BE() != MKID_BE('FORM')) {
		warning("VQAMovie::open: Cannot find `FORM' tag");
		return false;
	}

	// For now, we ignore the size of the FORM chunk.
	_file.readUint32BE();

	if (_file.readUint32BE() != MKID_BE('WVQA')) {
		warning("WQAMovie::open: Cannot find `WVQA' tag");
		return false;
	}

	bool foundHeader = false;
	bool foundFrameInfo = false;

	// The information we need is stored in two chunks: VQHD and FINF. We
	// need both of them before we can begin decoding the movie.

	while (!foundHeader || !foundFrameInfo) {
		uint32 tag = readTag();
		uint32 size = _file.readUint32BE();

		switch (tag) {
		case MKID_BE('VQHD'):	// VQA header
			_header.version     = _file.readUint16LE();
			_header.flags       = _file.readUint16LE();
			_header.numFrames   = _file.readUint16LE();
			_header.width       = _file.readUint16LE();
			_header.height      = _file.readUint16LE();
			_header.blockW      = _file.readByte();
			_header.blockH      = _file.readByte();
			_header.frameRate   = _file.readByte();
			_header.cbParts     = _file.readByte();
			_header.colors      = _file.readUint16LE();
			_header.maxBlocks   = _file.readUint16LE();
			_header.unk1        = _file.readUint32LE();
			_header.unk2        = _file.readUint16LE();
			_header.freq        = _file.readUint16LE();
			_header.channels    = _file.readByte();
			_header.bits        = _file.readByte();
			_header.unk3        = _file.readUint32LE();
			_header.unk4        = _file.readUint16LE();
			_header.maxCBFZSize = _file.readUint32LE();
			_header.unk5        = _file.readUint32LE();

			// Kyrandia 3 uses version 1 VQA files, and is the only
			// known game to do so. This version of the format has
			// some implicit default values.

			if (_header.version == 1) {
				if (_header.freq == 0)
					_header.freq = 22050;
				if (_header.channels == 0)
					_header.channels = 1;
				if (_header.bits == 0)
					_header.bits = 8;
			}

			setX((Screen::SCREEN_W - _header.width) / 2);
			setY((Screen::SCREEN_H - _header.height) / 2);

			_frameInfo = new uint32[_header.numFrames];
			_frame = new byte[_header.width * _header.height];

			_codeBookSize = 0xf00 * _header.blockW * _header.blockH;
			_codeBook = new byte[_codeBookSize];
			_partialCodeBook = new byte[_codeBookSize];
			memset(_codeBook, 0, _codeBookSize);
			memset(_partialCodeBook, 0, _codeBookSize);

			_numVectorPointers = (_header.width / _header.blockW) * (_header.height * _header.blockH);
			_vectorPointers = new uint16[_numVectorPointers];
			memset(_vectorPointers, 0, _numVectorPointers * sizeof(uint16));

			_partialCodeBookSize = 0;
			_numPartialCodeBooks = 0;

			if (_header.flags & 1) {
				// This VQA movie has sound. Kyrandia 3 uses
				// 8-bit sound, and so far testing indicates
				// that it's all mono.
				//
				// This is good, because it means we won't have
				// to worry about the confusing parts of the
				// VQA spec, where 8- and 16-bit data have 
				// different signedness and stereo sample
				// layout varies between different games.

				assert(_header.bits == 8);
				assert(_header.channels == 1);

				_stream = Audio::makeAppendableAudioStream(_header.freq, Audio::Mixer::FLAG_UNSIGNED);
			} else {
				_stream = NULL;
			}

			foundHeader = true;
			break;

		case MKID_BE('FINF'):	// Frame info
			if (!foundHeader) {
				warning("VQAMovie::open: Found `FINF' before `VQHD'");
				return false;
			}

			if (size != 4 * (uint32)_header.numFrames) {
				warning("VQAMovie::open: Expected size %d for `FINF' chunk, but got %d", 4 * _header.numFrames, size);
				return false;
			}

			foundFrameInfo = true;

			for (int i = 0; i < _header.numFrames; i++) {
				_frameInfo[i] = 2 * _file.readUint32LE();
			}

			// HACK: This flag is set in jung2.vqa, and its
			// purpose, if it has one, is unknown. It can't be a
			// general purpose flag, because in large movies the
			// frame offsets can be large enough to set this flag,
			// though of course never for the first frame.
			//
			// At least in my copy of Kyrandia 3, _frameInfo[0] is
			// 0x81000098, and the desired index is 0x4716. So the
			// value should be 0x80004716, but I don't want to
			// hard-code it. Instead, scan the file for the offset
			// to the first VQFR chunk.

			if (_frameInfo[0] & 0x01000000) {
				uint32 oldPos = _file.pos();

				while (1) {
					uint32 scanTag = readTag();
					uint32 scanSize = _file.readUint32BE();

					if (_file.eof())
						break;

					if (scanTag == MKID_BE('VQFR')) {
						_frameInfo[0] = (_file.pos() - 8) | 0x80000000;
						break;
					}

					_file.seek(scanSize, SEEK_CUR);
				}

				_file.seek(oldPos);
			}

			break;

		default:
			warning("VQAMovie::open: Unknown tag `%c%c%c%c'", (tag >> 24) & 0xFF, (tag >> 16) & 0xFF, (tag >> 8) & 0xFF, tag & 0xFF);
			_file.seek(size, SEEK_CUR);
			break;
		}
	}

	initBuffers();

	_opened = true;
	return true;
}

void VQAMovie::close() {
	debugC(9, kDebugLevelMovie, "VQAMovie::close()");
	if (_opened) {
		delete [] _frameInfo;
		delete [] _frame;
		delete [] _codeBook;
		delete [] _partialCodeBook;
		delete [] _vectorPointers;

		if (_vm->_mixer->isSoundHandleActive(_sound)) {
			_vm->_mixer->stopHandle(_sound);
		}

		_frameInfo = NULL;
		_frame = NULL;
		_codeBookSize = 0;
		_codeBook = NULL;
		_partialCodeBook = NULL;
		_vectorPointers = NULL;
		_stream = NULL;

		if (_file.isOpen())
			_file.close();

		freeBuffers();

		_opened = false;
	}
}

void VQAMovie::displayFrame(uint frameNum) {
	debugC(9, kDebugLevelMovie, "VQAMovie::displayFrame(%d)", frameNum);
	if (frameNum >= _header.numFrames || !_opened)
		return;

	bool foundSound = _stream ? false : true;
	bool foundFrame = false;
	uint i;

	_file.seek(_frameInfo[frameNum] & 0x7FFFFFFF);

	while (!foundSound || !foundFrame) {
		uint32 tag = readTag();
		uint32 size = _file.readUint32BE();

		if (_file.eof()) {
			// This happens at the last frame. Apparently it has
			// no sound?
			break;
		}

		byte *inbuf, *outbuf;
		uint32 insize, outsize;
		uint32 end;

		switch (tag) {
		case MKID_BE('SND0'):	// Uncompressed sound
			foundSound = true;
			inbuf = new byte[size];
			_file.read(inbuf, size);
			assert(_stream);
			_stream->queueBuffer(inbuf, size);
			break;

		case MKID_BE('SND1'):	// Compressed sound, almost like AUD
			foundSound = true;
			outsize = _file.readUint16LE();
			insize = _file.readUint16LE();

			inbuf = new byte[insize];
			_file.read(inbuf, insize);

			if (insize == outsize) {
				assert(_stream);
				_stream->queueBuffer(inbuf, insize);
			} else {
				outbuf = new byte[outsize];
				decodeSND1(inbuf, insize, outbuf, outsize);
				assert(_stream);
				_stream->queueBuffer(outbuf, outsize);
				delete[] inbuf;
			}
			break;

		case MKID_BE('SND2'):	// Compressed sound
			foundSound = true;
			warning("VQAMovie::displayFrame: `SND2' is not implemented");
			_file.seek(size, SEEK_CUR);
			break;

		case MKID_BE('VQFR'):
			foundFrame = true;
			end = _file.pos() + size - 8;

			while (_file.pos() < end) {
				tag = readTag();
				size = _file.readUint32BE();

				switch (tag) {
				case MKID_BE('CBF0'):	// Full codebook
					_file.read(_codeBook, size);
					break;

				case MKID_BE('CBFZ'):	// Full codebook
					inbuf = (byte *)allocBuffer(0, size);
					_file.read(inbuf, size);
					Screen::decodeFrame4(inbuf, _codeBook, _codeBookSize);
					break;

				case MKID_BE('CBP0'):	// Partial codebook
					_compressedCodeBook = false;
					_file.read(_partialCodeBook + _partialCodeBookSize, size);
					_partialCodeBookSize += size;
					_numPartialCodeBooks++;
					break;

				case MKID_BE('CBPZ'):	// Partial codebook
					_compressedCodeBook = true;
					_file.read(_partialCodeBook + _partialCodeBookSize, size);
					_partialCodeBookSize += size;
					_numPartialCodeBooks++;
					break;

				case MKID_BE('CPL0'):	// Palette
					assert(size <= 3 * 256);
					_file.read(_vm->screen()->_currentPalette, size);
					break;

				case MKID_BE('CPLZ'):	// Palette
					inbuf = (byte *)allocBuffer(0, size);
					_file.read(inbuf, size);
					Screen::decodeFrame4(inbuf, _vm->screen()->_currentPalette, 768);
					break;

				case MKID_BE('VPT0'):	// Frame data
					assert(size / 2 <= _numVectorPointers);

					for (i = 0; i < size / 2; i++)
						_vectorPointers[i] = _file.readUint16LE();
					break;

				case MKID_BE('VPTZ'):	// Frame data
					inbuf = (byte *)allocBuffer(0, size);
					outbuf = (byte *)allocBuffer(1, 2 * _numVectorPointers);

					_file.read(inbuf, size);
					size = Screen::decodeFrame4(inbuf, outbuf, 2 * _numVectorPointers);

					assert(size / 2 <= _numVectorPointers);

					for (i = 0; i < size / 2; i++)
						_vectorPointers[i] = READ_LE_UINT16(outbuf + 2 * i);
					break;

				default:
					warning("VQAMovie::displayFrame: Unknown `VQFR' sub-tag `%c%c%c%c'", (tag >> 24) & 0xFF, (tag >> 16) & 0xFF, (tag >> 8) & 0xFF, tag & 0xFF);
					_file.seek(size, SEEK_CUR);
					break;
				}

			}

			break;

		default:
			warning("VQAMovie::displayFrame: Unknown tag `%c%c%c%c'", (tag >> 24) & 0xFF, (tag >> 16) & 0xFF, (tag >> 8) & 0xFF, tag & 0xFF);
			_file.seek(size, SEEK_CUR);
			break;
		}
	}

	// The frame has been decoded

	if (_frameInfo[frameNum] & 0x80000000) {
		_vm->screen()->setScreenPalette(_vm->screen()->_currentPalette);
	}

	int blockPitch = _header.width / _header.blockW;

	for (int by = 0; by < _header.height / _header.blockH; by++) {
		for (int bx = 0; bx < blockPitch; bx++) {
			byte *dst = _frame + by * _header.width * _header.blockH + bx * _header.blockW;
			int val = _vectorPointers[by * blockPitch + bx];

			if ((val & 0xFF00) == 0xFF00) {
				// Solid color
				for (i = 0; i < _header.blockH; i++) {
					memset(dst, 255 - (val & 0xFF), _header.blockW);
					dst += _header.width;
				}
			} else {
				// Copy data from _vectorPointers. I'm not sure
				// why we don't use the three least significant
				// bits of 'val'.
				byte *src = &_codeBook[(val >> 3) * _header.blockW * _header.blockH];

				for (i = 0; i < _header.blockH; i++) {
					memcpy(dst, src, _header.blockW);
					src += _header.blockW;
					dst += _header.width;
				}
			}
		}
	}

	if (_numPartialCodeBooks == _header.cbParts) {
		if (_compressedCodeBook) {
			Screen::decodeFrame4(_partialCodeBook, _codeBook, _codeBookSize);
		} else {
			memcpy(_codeBook, _partialCodeBook, _partialCodeBookSize);
		}
		_numPartialCodeBooks = 0;
		_partialCodeBookSize = 0;
	}

	_vm->screen()->copyBlockToPage(_drawPage, _x, _y, _header.width, _header.height, _frame);
}

void VQAMovie::play() {
	uint32 startTick;

	if (!_opened)
		return;

	startTick = _system->getMillis();

	// First, handle any sound chunk that appears before the first frame.
	// At least in some versions, it will contain half a second of audio,
	// presumably to lessen the risk of audio underflow.
	//
	// In most movies, we will find a CMDS tag. The purpose of this is
	// currently unknown.
	//
	// In cow1_0.vqa, cow1_1.vqa, jung0.vqa, and jung1.vqa we will find a
	// VQFR tag. A frame before the first frame? Weird. It doesn't seem to
	// be needed, though.

	byte *inbuf, *outbuf;
	uint32 insize, outsize;

	if (_stream) {
		while (_file.pos() < (_frameInfo[0] & 0x7FFFFFFF)) {
			uint32 tag = readTag();
			uint32 size = _file.readUint32BE();

			if (_file.eof()) {
				warning("VQAMovie::play: Unexpected EOF");
				break;
			}

			switch (tag) {
			case MKID_BE('SND0'):	// Uncompressed sound
				inbuf = new byte[size];
				_file.read(inbuf, size);
				_stream->queueBuffer(inbuf, size);
				break;

			case MKID_BE('SND1'):	// Compressed sound
				outsize = _file.readUint16LE();
				insize = _file.readUint16LE();

				inbuf = new byte[insize];
				_file.read(inbuf, insize);

				if (insize == outsize) {
					_stream->queueBuffer(inbuf, insize);
				} else {
					outbuf = new byte[outsize];
					decodeSND1(inbuf, insize, outbuf, outsize);
					_stream->queueBuffer(outbuf, outsize);
					delete[] inbuf;
				}
				break;

			case MKID_BE('SND2'):	// Compressed sound
				warning("VQAMovie::play: `SND2' is not implemented");
				_file.seek(size, SEEK_CUR);
				break;

			case MKID_BE('CMDS'):	// Unused tag, always empty in kyra3
				debugC(9, kDebugLevelMovie, "VQAMovie::play: skipping CMDS tag");
				_file.seek(size, SEEK_CUR);
				break;

			default:
				warning("VQAMovie::play: Unknown tag `%c%c%c%c'", (tag >> 24) & 0xFF, (tag >> 16) & 0xFF, (tag >> 8) & 0xFF, tag & 0xFF);
				_file.seek(size, SEEK_CUR);
				break;
			}
		}
	}

	_vm->_mixer->playInputStream(Audio::Mixer::kSFXSoundType, &_sound, _stream);

	for (uint i = 0; i < _header.numFrames; i++) {
		displayFrame(i);

		// TODO: Implement frame skipping?

		while (1) {
			uint32 elapsedTime;

			if (_vm->_mixer->isSoundHandleActive(_sound)) {
				elapsedTime = _vm->_mixer->getSoundElapsedTime(_sound);
			} else {
				elapsedTime = _system->getMillis() - startTick;
			}

			if (elapsedTime >= (i * 1000) / _header.frameRate)
				break;

			Common::Event event;

			Common::EventManager *eventMan = _system->getEventManager();
			while (eventMan->pollEvent(event)) {
				switch (event.type) {
				case Common::EVENT_KEYDOWN:
					if (event.kbd.ascii == 27)
						return;
					break;
				case Common::EVENT_QUIT:
					_vm->quitGame();
					return;
				default:
					break;
				}
			}

			_system->delayMillis(10);
		}

		_vm->screen()->updateScreen();
	}

	// TODO: Wait for the sound to finish?
}

} // end of namespace Kyra
