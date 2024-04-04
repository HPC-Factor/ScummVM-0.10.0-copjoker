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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/engines/kyra/wsamovie.cpp $
 * $Id: wsamovie.cpp 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

#include "common/stdafx.h"
#include "common/endian.h"
#include "common/system.h"

#include "kyra/kyra.h"
#include "kyra/screen.h"
#include "kyra/wsamovie.h"


namespace Kyra {
WSAMovieV1::WSAMovieV1(KyraEngine *vm) : Movie(vm) {}
WSAMovieV1::~WSAMovieV1() { close(); }

int WSAMovieV1::open(const char *filename, int offscreenDecode, uint8 *palBuf) {
	debugC(9, kDebugLevelMovie, "WSAMovieV1::open('%s', %d, %p)", filename, offscreenDecode, (const void *)palBuf);
	close();

	uint32 flags = 0;
	uint32 fileSize;
	uint8 *p = _vm->resource()->fileData(filename, &fileSize);
	if (!p)
		return 0;
	
	const uint8 *wsaData = p;
	_numFrames = READ_LE_UINT16(wsaData); wsaData += 2;
	_width = READ_LE_UINT16(wsaData); wsaData += 2;
	_height = READ_LE_UINT16(wsaData); wsaData += 2;
	_deltaBufferSize = READ_LE_UINT16(wsaData); wsaData += 2;
	_offscreenBuffer = NULL;
	_flags = 0;
	if (_vm->gameFlags().useAltShapeHeader) {
		flags = READ_LE_UINT16(wsaData); 
		wsaData += 2;
	}
	
	uint32 offsPal = 0;
	if (flags & 1) {
		offsPal = 0x300;
		_flags |= WF_HAS_PALETTE;
		if (palBuf)
			memcpy(palBuf, wsaData + (_numFrames + 2) * 4, 0x300);
	}
	
	if (offscreenDecode) {
		_flags |= WF_OFFSCREEN_DECODE;
		const int offscreenBufferSize = _width * _height;
		_offscreenBuffer = new uint8[offscreenBufferSize];
		memset(_offscreenBuffer, 0, offscreenBufferSize);
	}

	if (_numFrames & 0x8000) {
		// This is used in the Amiga version, the wsa playing code
		// doesn't include any handling of it though, so we disable
		// this warning for now.
		//warning("Unhandled wsa flags 0x80");
		_flags |= 0x80;
		_numFrames &= 0x7FFF;
	}
	_currentFrame = _numFrames;

	_deltaBuffer = new uint8[_deltaBufferSize];
	memset(_deltaBuffer, 0, _deltaBufferSize);
	
	// read frame offsets
	_frameOffsTable = new uint32[_numFrames + 2];
	_frameOffsTable[0] = 0;
	uint32 frameDataOffs = READ_LE_UINT32(wsaData); wsaData += 4;
	bool firstFrame = true;

	if (frameDataOffs == 0) {
		firstFrame = false;
		frameDataOffs = READ_LE_UINT32(wsaData);
		_flags |= WF_NO_FIRST_FRAME;
	}

	for (int i = 1; i < _numFrames + 2; ++i) {
		_frameOffsTable[i] = READ_LE_UINT32(wsaData) - frameDataOffs;
		wsaData += 4;
	}
	
	// skip palette
	wsaData += offsPal;
	
	// read frame data
	const int frameDataSize = p + fileSize - wsaData;
	_frameData = new uint8[frameDataSize];
	memcpy(_frameData, wsaData, frameDataSize);
	
	// decode first frame
	if (firstFrame)
		Screen::decodeFrame4(_frameData, _deltaBuffer, _deltaBufferSize);
	
	delete [] p;
	_opened = true;
	
	return _numFrames;
}

void WSAMovieV1::close() {
	debugC(9, kDebugLevelMovie, "WSAMovieV1::close()");
	if (_opened) {
		delete [] _deltaBuffer;
		delete [] _offscreenBuffer;
		delete [] _frameOffsTable;
		delete [] _frameData;
		_opened = false;
	}
}

void WSAMovieV1::displayFrame(int frameNum) {
	debugC(9, kDebugLevelMovie, "WSAMovieV1::displayFrame(%d)", frameNum);
	if (frameNum >= _numFrames || !_opened)
		return;

	uint8 *dst;
	if (_flags & WF_OFFSCREEN_DECODE)
		dst = _offscreenBuffer;
	else
		dst = _vm->screen()->getPageRect(_drawPage, _x, _y, _width, _height);
		
	if (_currentFrame == _numFrames) {
		if (!(_flags & WF_NO_FIRST_FRAME)) {
			if (_flags & WF_OFFSCREEN_DECODE)
				Screen::decodeFrameDelta(dst, _deltaBuffer);
			else
				Screen::decodeFrameDeltaPage(dst, _deltaBuffer, _width, (_flags & WF_XOR) == 0);
		}
		_currentFrame = 0;
	}

	// try to reduce the number of needed frame operations
	int diffCount = ABS(_currentFrame - frameNum);
	int frameStep = 1;
	int frameCount;
	if (_currentFrame < frameNum) {
		frameCount = _numFrames - frameNum + _currentFrame;
		if (diffCount > frameCount)
			frameStep = -1;
		else
			frameCount = diffCount;
	} else {
		frameCount = _numFrames - _currentFrame + frameNum;
		if (frameCount >= diffCount) {
			frameStep = -1;
			frameCount = diffCount;
		}
	}
	
	// process
	if (frameStep > 0) {
		uint16 cf = _currentFrame;
		while (frameCount--) {
			cf += frameStep;
			processFrame(cf, dst);
			if (cf == _numFrames)
				cf = 0;
		}
	} else {
		uint16 cf = _currentFrame;
		while (frameCount--) {
			if (cf == 0)
				cf = _numFrames;
			processFrame(cf, dst);
			cf += frameStep;
		}
	}
	
	// display
	_currentFrame = frameNum;
	if (_flags & WF_OFFSCREEN_DECODE)
		_vm->screen()->copyBlockToPage(_drawPage, _x, _y, _width, _height, _offscreenBuffer);
}

void WSAMovieV1::processFrame(int frameNum, uint8 *dst) {
	debugC(9, kDebugLevelMovie, "WSAMovieV1::processFrame(%d, %p)", frameNum, (const void *)dst);
	if (!_opened)
		return;
	assert(frameNum <= _numFrames);
	const uint8 *src = _frameData + _frameOffsTable[frameNum];
	Screen::decodeFrame4(src, _deltaBuffer, _deltaBufferSize);
	if (_flags & WF_OFFSCREEN_DECODE)
		Screen::decodeFrameDelta(dst, _deltaBuffer);
	else
		Screen::decodeFrameDeltaPage(dst, _deltaBuffer, _width, false);
}

#pragma mark -

WSAMovieAmiga::WSAMovieAmiga(KyraEngine *vm) : WSAMovieV1(vm), _buffer(0) {}

int WSAMovieAmiga::open(const char *filename, int offscreenDecode, uint8 *palBuf) {
	debugC(9, kDebugLevelMovie, "WSAMovieAmiga::open('%s', %d, %p)", filename, offscreenDecode, (const void *)palBuf);
	int res = WSAMovieV1::open(filename, offscreenDecode, palBuf);

	if (!res)
		return 0;

	_buffer = new uint8[_width * _height];
	assert(_buffer);
	return res;	
}

void WSAMovieAmiga::close() {
	debugC(9, kDebugLevelMovie, "WSAMovieAmiga::close()");
	if (_opened) {
		delete [] _buffer;
		_buffer = 0;
	}
	WSAMovieV1::close();
}

void WSAMovieAmiga::displayFrame(int frameNum) {
	debugC(9, kDebugLevelMovie, "WSAMovieAmiga::displayFrame(%d)", frameNum);
	if (frameNum >= _numFrames || !_opened)
		return;

	uint8 *dst;
	dst = _buffer;
	memset(_buffer, 0, _width*_height);

	if (_currentFrame == _numFrames) {
		if (!(_flags & WF_NO_FIRST_FRAME)) {
			Screen::decodeFrameDelta(dst, _deltaBuffer, true);
			Screen::convertAmigaGfx(dst, _width, _height);

			if (_flags & WF_OFFSCREEN_DECODE) {
				dst = _offscreenBuffer;
				const uint8 *src = _buffer;
				int size = _width * _height;

				for (int i = 0; i < size; ++i)
					*dst++ ^= *src++;

				dst = _buffer;
			} else {
				_vm->screen()->copyBlockToPage(_drawPage, _x, _y, _width, _height, _buffer);
			}
		}
		_currentFrame = 0;
	}

	// try to reduce the number of needed frame operations
	int diffCount = ABS(_currentFrame - frameNum);
	int frameStep = 1;
	int frameCount;
	if (_currentFrame < frameNum) {
		frameCount = _numFrames - frameNum + _currentFrame;
		if (diffCount > frameCount)
			frameStep = -1;
		else
			frameCount = diffCount;
	} else {
		frameCount = _numFrames - _currentFrame + frameNum;
		if (frameCount >= diffCount) {
			frameStep = -1;
			frameCount = diffCount;
		}
	}
	
	// process
	if (frameStep > 0) {
		uint16 cf = _currentFrame;
		while (frameCount--) {
			cf += frameStep;
			processFrame(cf, dst);
			if (cf == _numFrames)
				cf = 0;
		}
	} else {
		uint16 cf = _currentFrame;
		while (frameCount--) {
			if (cf == 0)
				cf = _numFrames;
			processFrame(cf, dst);
			cf += frameStep;
		}
	}
	
	// display
	_currentFrame = frameNum;
	if (_flags & WF_OFFSCREEN_DECODE)
		_vm->screen()->copyBlockToPage(_drawPage, _x, _y, _width, _height, _offscreenBuffer);
}

void WSAMovieAmiga::processFrame(int frameNum, uint8 *dst) {
	debugC(9, kDebugLevelMovie, "WSAMovieAmiga::processFrame(%d, %p)", frameNum, (const void *)dst);
	if (!_opened)
		return;
	assert(frameNum <= _numFrames);

	memset(dst, 0, _width*_height);

	const uint8 *src = _frameData + _frameOffsTable[frameNum];
	Screen::decodeFrame4(src, _deltaBuffer, _deltaBufferSize);
	Screen::decodeFrameDelta(dst, _deltaBuffer, true);
	Screen::convertAmigaGfx(dst, _width, _height);

	src = dst;
	dst = 0;
	int dstPitch = 0;
	if (_flags & WF_OFFSCREEN_DECODE) {
		dst = _offscreenBuffer;
		dstPitch = _width;
	} else {
		dst = _vm->screen()->getPageRect(_drawPage, _x, _y, _width, _height);
		dstPitch = Screen::SCREEN_W;
	}

	for (int y = 0; y < _height; ++y) {
		for (int x = 0; x < _width; ++x)
			*dst++ ^= *src++;			
		dst += dstPitch - _width;
	}
}

#pragma mark -

WSAMovieV2::WSAMovieV2(KyraEngine *vm) : WSAMovieV1(vm), _xAdd(0), _yAdd(0) {}

int WSAMovieV2::open(const char *filename, int unk1, uint8 *palBuf) {
	debugC(9, kDebugLevelMovie, "WSAMovieV2::open('%s', %d, %p)", filename, unk1, (const void *)palBuf);
	close();

	uint32 flags = 0;
	uint32 fileSize;
	uint8 *p = _vm->resource()->fileData(filename, &fileSize);
	if (!p) {
		warning("couldn't load wsa file: '%s'", filename);
		return 0;
	}
	
	const uint8 *wsaData = p;
	_numFrames = READ_LE_UINT16(wsaData); wsaData += 2;
	_xAdd = (int16)(READ_LE_UINT16(wsaData)); wsaData += 2;
	_yAdd = (int16)(READ_LE_UINT16(wsaData)); wsaData += 2;
	_width = READ_LE_UINT16(wsaData); wsaData += 2;
	_height = READ_LE_UINT16(wsaData); wsaData += 2;
	_deltaBufferSize = READ_LE_UINT16(wsaData); wsaData += 2;
	_offscreenBuffer = NULL;
	_flags = 0;
	flags = READ_LE_UINT16(wsaData); wsaData += 2;
	
	uint32 offsPal = 0;
	if (flags & 1) {
		offsPal = 0x300;
		_flags |= WF_HAS_PALETTE;
		if (palBuf)
			memcpy(palBuf, wsaData + 8 + ((_numFrames << 2) & 0xFFFF), 0x300);
	}
	
	if (flags & 2)
		_flags |= WF_XOR;
	
	if (!(unk1 & 2)) {
		_flags |= WF_OFFSCREEN_DECODE;
		const int offscreenBufferSize = _width * _height;
		_offscreenBuffer = new uint8[offscreenBufferSize];
		memset(_offscreenBuffer, 0, offscreenBufferSize);
	}

	if (_numFrames & 0x8000) {
		warning("Unhandled wsa flags 0x80");
		_flags |= 0x80;
		_numFrames &= 0x7FFF;
	}
	_currentFrame = _numFrames;

	_deltaBuffer = new uint8[_deltaBufferSize];
	memset(_deltaBuffer, 0, _deltaBufferSize);
	
	// read frame offsets
	_frameOffsTable = new uint32[_numFrames + 2];
	_frameOffsTable[0] = 0;
	uint32 frameDataOffs = READ_LE_UINT32(wsaData); wsaData += 4;
	for (int i = 1; i < _numFrames + 2; ++i) {
		_frameOffsTable[i] = READ_LE_UINT32(wsaData) - frameDataOffs;
		wsaData += 4;
	}
	
	// skip palette
	wsaData += offsPal;
	
	// read frame data
	const int frameDataSize = p + fileSize - wsaData;
	_frameData = new uint8[frameDataSize];
	memcpy(_frameData, wsaData, frameDataSize);
	
	// decode first frame
	Screen::decodeFrame4(_frameData, _deltaBuffer, _deltaBufferSize);
	
	delete [] p;
	_opened = true;
	
	return _numFrames;
}

} // end of namespace Kyra

