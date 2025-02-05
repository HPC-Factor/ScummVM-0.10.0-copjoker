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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/graphics/dxa_player.cpp $
 * $Id: dxa_player.cpp 27372 2007-06-12 06:47:40Z Kirben $
 *
 */

#include "common/stdafx.h"
#include "common/endian.h"
#include "graphics/dxa_player.h"
#include "common/util.h"

#ifdef USE_ZLIB
#include <zlib.h>
#endif

namespace Graphics {

DXAPlayer::DXAPlayer() {
	_fd = 0;

	_frameBuffer1 = 0;
	_frameBuffer2 = 0;
	_scaledBuffer = 0;
	_drawBuffer = 0;

	_width = 0;
	_height = 0;

	_frameSize = 0;
	_framesCount = 0;
	_frameNum = 0;
	_framesPerSec = 0;
	_frameSkipped = 0;
	_frameTicks = 0;

	_scaleMode = S_NONE;
}

DXAPlayer::~DXAPlayer() {
}

int DXAPlayer::getWidth() {
	if (!_fd)
		return 0;
	return _width;
}

int DXAPlayer::getHeight() {
	if (!_fd)
		return 0;
	return _height;
}

int DXAPlayer::getCurFrame() {
	if (!_fd)
		return -1;
	return _frameNum;
}

int DXAPlayer::getFrameCount() {
	if (!_fd)
		return 0;
	return _framesCount;
}

bool DXAPlayer::loadFile(const char *filename) {
	uint32 tag;
	int32 frameRate;

	Common::File *file = new Common::File();
	if (!file->open(filename)) {
		return 0;
	}

	_fd = file;

	tag = _fd->readUint32BE();
	assert(tag == MKID_BE('DEXA'));

	uint8 flags = _fd->readByte();
	_framesCount = _fd->readUint16BE();
	frameRate = _fd->readUint32BE();

	if (frameRate > 0)
		_framesPerSec = 1000 / frameRate;
	else if (frameRate < 0)
		_framesPerSec = 100000 / (-frameRate);
	else
		_framesPerSec = 10;

	if (frameRate < 0)
		_frameTicks = -frameRate / 100;
	else
		_frameTicks = frameRate;

	_width = _fd->readUint16BE();
	_height = _fd->readUint16BE();

	if (flags & 0x80) {
		_scaleMode = S_INTERLACED;
		_curHeight = _height / 2;
	} else if (flags & 0x40) {
		_scaleMode = S_DOUBLE;
		_curHeight = _height / 2;
	} else {
		_scaleMode = S_NONE;
		_curHeight = _height;
	}

	debug(2, "flags 0x0%x framesCount %d width %d height %d rate %d ticks %d", flags, _framesCount, _width, _height, _framesPerSec, _frameTicks);

	_frameSize = _width * _height;
	_frameBuffer1 = (uint8 *)malloc(_frameSize);
	_frameBuffer2 = (uint8 *)malloc(_frameSize);
	if (!_frameBuffer1 || !_frameBuffer2)
		error("DXAPlayer: Error allocating frame buffers (size %d)", _frameSize);

	_scaledBuffer = 0;
	if (_scaleMode != S_NONE) {
		_scaledBuffer = (uint8 *)malloc(_frameSize);
		if (!_scaledBuffer)
			error("Error allocating scale buffer (size %d)", _frameSize);
	}

	_frameNum = 0;
	_frameSkipped = 0;

	return true;
}

void DXAPlayer::closeFile() {
	if (!_fd)
		return;

	delete _fd;
	free(_frameBuffer1);
	free(_frameBuffer2);
	free(_scaledBuffer);
}

void DXAPlayer::copyFrameToBuffer(byte *dst, uint x, uint y, uint pitch) {
	uint h = _height;
	uint w = _width;

	byte *src = _drawBuffer;
	dst += y * pitch + x;

	do {
		memcpy(dst, src, w);
		dst += pitch;
		src += _width;
	} while (--h);
}

void DXAPlayer::decodeZlib(byte *data, int size, int totalSize) {
#ifdef USE_ZLIB
	byte *temp = (byte *)malloc(size);
	if (temp) {
		memcpy(temp, data, size);

		z_stream d_stream;
		d_stream.zalloc = (alloc_func)0;
		d_stream.zfree = (free_func)0;
		d_stream.opaque = (voidpf)0;
		d_stream.next_in = temp;
		d_stream.avail_in = size;
		d_stream.total_in = size;
		d_stream.next_out = data;
		d_stream.avail_out = totalSize;
		inflateInit(&d_stream);
		inflate(&d_stream, Z_FINISH);
		inflateEnd(&d_stream);
		free(temp);
	}
#endif
}

#define BLOCKW 4
#define BLOCKH 4

void DXAPlayer::decode12(byte *data, int size, int totalSize) {
#ifdef USE_ZLIB
	/* decompress the input data */
	decodeZlib(data, size, totalSize);

	byte *dat = data;
	byte *frame2 = (byte *)malloc(totalSize);

	memcpy(frame2, _frameBuffer1, totalSize);

	for (int by = 0; by < _height; by += BLOCKH) {
		for (int bx = 0; bx < _width; bx += BLOCKW) {
			byte type = *dat++;
			byte *b2 = frame2 + bx + by * _width;

			switch (type) {
			case 0:
				break;
			case 10:
			case 11:
			case 12:
			case 13:
			case 14:
			case 15:
			case 1:	{
				unsigned short diffMap;
				if (type >= 10 && type <= 15) {
					static const struct { uint8 sh1, sh2; } shiftTbl[6] = {
						{0, 0},	{8, 0},	{8, 8},	{8, 4},	{4, 0},	{4, 4}
					};
					diffMap = ((*dat & 0xF0) << shiftTbl[type-10].sh1) |
						  ((*dat & 0x0F) << shiftTbl[type-10].sh2);
					dat++;
				} else {
					diffMap = *(unsigned short*)dat;
					dat += 2;
				}

				for (int yc = 0; yc < BLOCKH; yc++) {
					for (int xc = 0; xc < BLOCKW; xc++) {
						if (diffMap & 0x8000) {
							b2[xc] = *dat++;
						}
						diffMap <<= 1;
					}
					b2 += _width;
				}
				break;
			}
			case 2:	{
				byte color = *dat++;

				for (int yc = 0; yc < BLOCKH; yc++) {
					for (int xc = 0; xc < BLOCKW; xc++) {
						b2[xc] = color;
					}
					b2 += _width;
				}
				break;
			}
			case 3:	{
				for (int yc = 0; yc < BLOCKH; yc++) {
					for (int xc = 0; xc < BLOCKW; xc++) {
						b2[xc] = *dat++;
					}
					b2 += _width;
				}
				break;
			}
			case 4:	{
				byte mbyte = *dat++;
				int mx = (mbyte >> 4) & 0x07;
				if (mbyte & 0x80)
					mx = -mx;
				int my = mbyte & 0x07;
				if (mbyte & 0x08)
					my = -my;
				byte *b1 = _frameBuffer1 + (bx+mx) + (by+my) * _width;
				for (int yc = 0; yc < BLOCKH; yc++) {
					memcpy(b2, b1, BLOCKW);
					b1 += _width;
					b2 += _width;
				}
				break;
			}
			case 5:
				break;
			default:
				error("decode12: Unknown type %d", type);
			}
		}
	}

	memcpy(data, frame2, totalSize);
	free(frame2);
#endif
}

void DXAPlayer::decode13(byte *data, int size, int totalSize) {
#ifdef USE_ZLIB
	uint8 *codeBuf, *dataBuf, *motBuf, *maskBuf;

	/* decompress the input data */
	decodeZlib(data, size, totalSize);

	uint8 *frame2 = (uint8*)malloc(totalSize);
	memcpy(frame2, _frameBuffer1, totalSize);

	int codeSize = _width * _curHeight / 16;
	int dataSize, motSize, maskSize;

	dataSize = READ_BE_UINT32(&data[0]);
	motSize  = READ_BE_UINT32(&data[4]);
	maskSize = READ_BE_UINT32(&data[8]);

	codeBuf = &data[12];
	dataBuf = &codeBuf[codeSize];
	motBuf = &dataBuf[dataSize];
	maskBuf = &motBuf[motSize];

	for (int by = 0; by < _curHeight; by += BLOCKH) {
		for (int bx = 0; bx < _width; bx += BLOCKW) {
			uint8 type = *codeBuf++;
			uint8 *b2 = (uint8*)frame2 + bx + by * _width;

			switch (type) {
			case 0:
				break;

			case 1: {
				uint16 diffMap = READ_BE_UINT16(maskBuf);
				maskBuf += 2;

				for (int yc = 0; yc < BLOCKH; yc++) {
					for (int xc = 0; xc < BLOCKW; xc++) {
						if (diffMap & 0x8000) {
							b2[xc] = *dataBuf++;
						}
						diffMap <<= 1;
					}
					b2 += _width;
				}
				break;
			}
			case 2: {
				uint8 color = *dataBuf++;

				for (int yc = 0; yc < BLOCKH; yc++) {
					for (int xc = 0; xc < BLOCKW; xc++) {
						b2[xc] = color;
					}
					b2 += _width;
				}
				break;
			}
			case 3: {
				for (int yc = 0; yc < BLOCKH; yc++) {
					for (int xc = 0; xc < BLOCKW; xc++) {
						b2[xc] = *dataBuf++;
					}
					b2 += _width;
				}
				break;
			}
			case 4: {
				uint8 mbyte = *motBuf++;

				int mx = (mbyte >> 4) & 0x07;
				if (mbyte & 0x80)
					mx = -mx;
				int my = mbyte & 0x07;
				if (mbyte & 0x08)
					my = -my;

				uint8 *b1 = (uint8*)_frameBuffer1 + (bx+mx) + (by+my) * _width;
				for (int yc = 0; yc < BLOCKH; yc++) {
					memcpy(b2, b1, BLOCKW);
					b1 += _width;
					b2 += _width;
				}
				break;
			}
			case 8: {
				static const int subX[4] = {0, 2, 0, 2};
				static const int subY[4] = {0, 0, 2, 2};

				uint8 subMask = *maskBuf++;

				for (int subBlock = 0; subBlock < 4; subBlock++) {
					int sx = bx + subX[subBlock], sy = by + subY[subBlock];
					b2 = (uint8*)frame2 + sx + sy * _width;
					switch (subMask & 0xC0) {
					// 00: skip
					case 0x00:
						break;
					// 01: solid color
					case 0x40: {
						uint8 subColor = *dataBuf++;
						for (int yc = 0; yc < BLOCKH / 2; yc++) {
							for (int xc = 0; xc < BLOCKW / 2; xc++) {
								b2[xc] = subColor;
							}
							b2 += _width;
						}
						break;
					}
					// 02: motion vector
					case 0x80: {
						uint8 mbyte = *motBuf++;

						int mx = (mbyte >> 4) & 0x07;
						if (mbyte & 0x80)
							mx = -mx;

						int my = mbyte & 0x07;
						if (mbyte & 0x08)
							my = -my;

						uint8 *b1 = (uint8*)_frameBuffer1 + (sx+mx) + (sy+my) * _width;
						for (int yc = 0; yc < BLOCKH / 2; yc++) {
							memcpy(b2, b1, BLOCKW / 2);
							b1 += _width;
							b2 += _width;
						}
						break;
					}
					// 03: raw
					case 0xC0:
						for (int yc = 0; yc < BLOCKH / 2; yc++) {
							for (int xc = 0; xc < BLOCKW / 2; xc++) {
								b2[xc] = *dataBuf++;
							}
							b2 += _width;
						}
						break;
					}
					subMask <<= 2;
				}
				break;
			}
			case 32:
			case 33:
			case 34: {
				int count = type - 30;
				uint8 pixels[4];

				memcpy(pixels, dataBuf, count);
				dataBuf += count;

				if (count == 2) {
					uint16 code = READ_BE_UINT16(maskBuf);
					maskBuf += 2;
					for (int yc = 0; yc < BLOCKH; yc++) {
						for (int xc = 0; xc < BLOCKW; xc++) {
							b2[xc] = pixels[code & 1];
							code >>= 1;
						}
						b2 += _width;
					}
				} else {
					uint32 code = READ_BE_UINT32(maskBuf);
					maskBuf += 4;
					for (int yc = 0; yc < BLOCKH; yc++) {
						for (int xc = 0; xc < BLOCKW; xc++) {
							b2[xc] = pixels[code & 3];
							code >>= 2;
						}
						b2 += _width;
					}
				}
				break;
			}
			default:
				error("decode13: Unknown type %d", type);
			}
		}
	}

	memcpy(data, frame2, totalSize);
	free(frame2);
#endif
}

void DXAPlayer::decodeNextFrame() {
	uint32 tag;

	tag = _fd->readUint32BE();
	if (tag == MKID_BE('CMAP')) {
		byte rgb[768];

		_fd->read(rgb, ARRAYSIZE(rgb));
		setPalette(rgb);
	}

	tag = _fd->readUint32BE();
	if (tag == MKID_BE('FRAM')) {
		byte type = _fd->readByte();
		uint32 size = _fd->readUint32BE();

		_fd->read(_frameBuffer2, size);

		switch (type) {
		case 2:
		case 3:
			decodeZlib(_frameBuffer2, size, _frameSize);
			break;
		case 12:
			decode12(_frameBuffer2, size, _frameSize);
			break;
		case 13:
			decode13(_frameBuffer2, size, _frameSize);
			break;
		default:
			error("decodeFrame: Unknown compression type %d", type);
		}
		if (type == 2 || type == 4 || type == 12 || type == 13) {
			memcpy(_frameBuffer1, _frameBuffer2, _frameSize);
		} else {
			for (int j = 0; j < _curHeight; ++j) {
				for (int i = 0; i < _width; ++i) {
					const int offs = j * _width + i;
					_frameBuffer1[offs] ^= _frameBuffer2[offs];
				}
			}
		}
	}

	switch (_scaleMode) {
	case S_INTERLACED:
		for (int cy = 0; cy < _curHeight; cy++) {
			memcpy(&_scaledBuffer[2 * cy * _width], &_frameBuffer1[cy * _width], _width);
			memset(&_scaledBuffer[((2 * cy) + 1) * _width], 0, _width);
		}
		_drawBuffer = _scaledBuffer;
		break;
	case S_DOUBLE:
		for (int cy = 0; cy < _curHeight; cy++) {
			memcpy(&_scaledBuffer[2 * cy * _width], &_frameBuffer1[cy * _width], _width);
			memcpy(&_scaledBuffer[((2 * cy) + 1) * _width], &_frameBuffer1[cy * _width], _width);
		}
		_drawBuffer = _scaledBuffer;
		break;
	case S_NONE:
		_drawBuffer = _frameBuffer1;
		break;
	}
}

} // End of namespace Graphics
