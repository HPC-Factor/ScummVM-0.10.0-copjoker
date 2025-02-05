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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/engines/agos/res_ami.cpp $
 * $Id: res_ami.cpp 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

// Conversion routines for planar graphics in Amiga versions
#include "common/stdafx.h"

#include "agos/agos.h"
#include "agos/intern.h"

namespace AGOS {

enum {
	kMaxColorDepth = 5
};

static void uncompressPlane(const byte *plane, byte *outptr, int length) {
	while (length != 0) {
		int wordlen;
		signed char x = *plane++;
		if (x >= 0) {
			wordlen = MIN<int>(x + 1, length);
			uint16 w = READ_UINT16(plane); plane += 2;
			for (int i = 0; i < wordlen; ++i) {
				WRITE_UINT16(outptr, w); outptr += 2;
			}
		} else {
			wordlen = MIN<int>(-x, length);
			memcpy(outptr, plane, wordlen * 2);
			outptr += wordlen * 2;
			plane += wordlen * 2;
		}
		length -= wordlen;
	}
}

static void bitplaneToChunky(uint16 *w, uint8 colorDepth, uint8 *&dst) {
	for (int j = 0; j < 8; j++) {
		byte color1 = 0;
		byte color2 = 0;
		for (int p = 0; p < colorDepth; ++p) {
			if (w[p] & 0x8000) {
				color1 |= 1 << p;
			}
			if (w[p] & 0x4000) {
				color2 |= 1 << p;
			}
			w[p] <<= 2;
		}
		if (colorDepth > 4) {
			*dst++ = color1;
			*dst++ = color2;
		} else {
			*dst++ = (color1 << 4) | color2;
		}
	}
}

static void bitplaneToChunkyText(uint16 *w, uint8 colorDepth, uint8 *&dst) {
	for (int j = 0; j < 16; j++) {
		byte color = 0;
		for (int p = 0; p < colorDepth; ++p) {
			if (w[p] & 0x8000) {
				color |= 1 << p;
			}
			w[p] <<= 1;
		}
		if (color)
			color |= 0xC0;
		*dst++ = color;
	}
}

static void convertCompressedImage(const byte *src, byte *dst, uint8 colorDepth, int height, int width) {
	const byte *plane[kMaxColorDepth];
	byte *uncptr[kMaxColorDepth];
	int length, i, j;

	byte *uncbfrout = (byte *)malloc(width * height);

	length = (width + 15) / 16 * height;

	for (i = 0; i < colorDepth; ++i) {
		plane[i] = src + READ_BE_UINT16(src + i * 4) + READ_BE_UINT16(src + i * 4 + 2);
		uncptr[i] = (uint8 *)malloc(length * 2);
		uncompressPlane(plane[i], uncptr[i], length);
		plane[i] = uncptr[i];
	}

	byte *uncbfroutptr = uncbfrout;
	for (i = 0; i < length; ++i) {
		uint16 w[kMaxColorDepth];
		for (j = 0; j < colorDepth; ++j) {
			w[j] = READ_BE_UINT16(plane[j]); plane[j] += 2;
		}
		bitplaneToChunky(w, colorDepth, uncbfroutptr);
	}

	uncbfroutptr = uncbfrout;
	const int chunkSize = colorDepth > 4 ? 16 : 8;
	for (i = 0; i < width / 16; ++i) {
		for (j = 0; j < height; ++j) {
			memcpy(dst + width * chunkSize / 16 * j + chunkSize * i, uncbfroutptr, chunkSize);
			uncbfroutptr += chunkSize;
		}
	}

	free(uncbfrout);
	for (i = 0; i < colorDepth; ++i) {
		free(uncptr[i]);
  	}
}

byte *AGOSEngine::convertImage(VC10_state *state, bool compressed) {
	int length, i, j;

	uint8 colorDepth = 4;
	if (getGameType() == GType_SIMON1) {
		if (((_lockWord & 0x20) && !state->palette) || ((getFeatures() & GF_32COLOR) &&
			state->palette != 0xC0)) {
			colorDepth = 5;
		}
	}

	const byte *src = state->srcPtr;
	int width = state->width * 16;
	int height = state->height;

	free(_planarBuf);
	_planarBuf = (byte *)malloc(width * height);
	byte *dst = _planarBuf;

	if (compressed) {
		convertCompressedImage(src, dst, colorDepth, height, width);
	} else {
		length = (width + 15) / 16 * height;
		for (i = 0; i < length; i++) {
			uint16 w[kMaxColorDepth];
			if (getGameType() == GType_SIMON1 && colorDepth == 4) {
				for (j = 0; j < colorDepth; ++j) {
					w[j] = READ_BE_UINT16(src + j * length * 2);
				}
				if (state->palette == 0xC0) {
					bitplaneToChunkyText(w, colorDepth, dst);
				} else {
					bitplaneToChunky(w, colorDepth, dst);
				}
				src += 2;
			} else {
				for (j = 0; j < colorDepth; ++j) {
					w[j] = READ_BE_UINT16(src); src += 2;
				}
				bitplaneToChunky(w, colorDepth, dst);
			}
		}
	}

	return _planarBuf;
}

} // End of namespace AGOS
