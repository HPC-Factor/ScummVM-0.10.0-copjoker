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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/engines/touche/graphics.cpp $
 * $Id: graphics.cpp 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

#include "common/stdafx.h"
#include "common/endian.h"

#include "touche/graphics.h"

namespace Touche {

void Graphics::setupFont(Common::Language language) {
	switch (language) {
	case Common::FR_FRA:
	case Common::DE_DEU:
		_fontOffs = _freGerFontOffs;
		_fontSize = _freGerFontSize;
		_fontData = _freGerFontData;
		break;
	case Common::ES_ESP:
		_fontOffs = _spaFontOffs;
		_fontSize = _spaFontSize;
		_fontData = _spaFontData;
		break;
	case Common::IT_ITA:
	case Common::EN_ANY:
	default:
		_fontOffs = _engFontOffs;
		_fontSize = _engFontSize;
		_fontData = _engFontData;
		break;
	}
}

int Graphics::getStringWidth16(const char *str) {
	int w = 0;
	while (*str) {
		char chr = *str++;
		w += getCharWidth16((uint8)chr);
		if (*str == '\\') {
			break;
		}
	}
	return w;
}

int Graphics::getCharWidth16(uint8 chr) {
	assert(chr >= 32 && chr < 32 + _fontSize);
	const uint8 *chrData = _fontData + _fontOffs[chr - 32];
	return chrData[2];
}

void Graphics::drawString16(uint8 *dst, int dstPitch, uint16 color, int x, int y, const char *str) {
	while (*str) {
		uint8 chr = (uint8)*str++;
		x += drawChar16(dst, dstPitch, chr, x, y, color);
	}
}

int Graphics::drawChar16(uint8 *dst, int dstPitch, uint8 chr, int x, int y, uint16 color) {
	dst += y * dstPitch + x;
	uint8 color1 = color & 0xFF;
	uint8 color2 = color >> 8;
	assert(chr >= 32 && chr < 32 + _fontSize);
	const uint8 *chrData = _fontData + _fontOffs[chr - 32];
	int chrHeight = chrData[1];
	int chrWidth = chrData[2];
	chrData += 3;
	while (chrHeight--) {
		int shiftCount = 0;
		int mask = 0;
		for (int i = 0; i < chrWidth; ++i) {
			if (shiftCount == 0) {
				mask = READ_BE_UINT16(chrData); chrData += 2;
				shiftCount = 8;
			}
			int b = (mask & 0xC000) >> 14;
			mask <<= 2;
			--shiftCount;
			if (b) {
				if (b & 2) {
					dst[i] = color2;
				} else {
					dst[i] = color1;
				}
			}
		}
		dst += dstPitch;
	}
	return chrWidth;
}

void Graphics::fillRect(uint8 *dst, int dstPitch, int x, int y, int w, int h, uint8 color) {
	dst += y * dstPitch + x;
	while (h--) {
		memset(dst, color, w);
		dst += dstPitch;
	}
}

void Graphics::drawRect(uint8 *dst, int dstPitch, int x, int y, int w, int h, uint8 color1, uint8 color2) {
	const int x1 = x;
	const int y1 = y;
	const int x2 = x + w - 1;
	const int y2 = y + h - 1;
	drawLine(dst, dstPitch, x1, y1, x2, y1, color1);
	drawLine(dst, dstPitch, x1, y1, x1, y2, color1);
	drawLine(dst, dstPitch, x2, y1 + 1, x2, y2, color2);
	drawLine(dst, dstPitch, x1 + 1, y2, x2, y2, color2);
}

void Graphics::drawLine(uint8 *dst, int dstPitch, int x1, int y1, int x2, int y2, uint8 color) {
	assert(x1 >= 0 && y1 >= 0 && x2 >= 0 && y2 >= 0);

	dst += y1 * dstPitch + x1;

	int yInc, dy = y2 - y1;
	if (dy < 0) {
		dy = -dy;
		yInc = -dstPitch;
	} else {
		yInc = dstPitch;
	}

	int xInc, dx = x2 - x1;
	if (dx < 0) {
		dx = -dx;
		xInc = -1;
	} else {
		xInc = 1;
	}

	int step = 0;

	if (dx > dy) {
		for (int i = 0; i < dx + 1; ++i) {
			*dst = color;
			dst += xInc;
			step += dy;
			if (step > dx) {
				step -= dx;
				dst += yInc;
			}
		}
	} else {
		for (int i = 0; i < dy + 1; ++i) {
			*dst = color;
			dst += yInc;
			step += dx;
			if (step > 0) {
				step -= dy;
				dst += xInc;
			}
		}
	}
}

void Graphics::copyRect(uint8 *dst, int dstPitch, int dstX, int dstY, const uint8 *src, int srcPitch, int srcX, int srcY, int w, int h, int flags) {
	if (w != 0 && h != 0) {
		dst += dstY * dstPitch + dstX;
		src += srcY * srcPitch + srcX;
		while (h--) {
			for (int i = 0; i < w; ++i) {
				if ((flags & kTransparent) == 0 || src[i] != 0) {
					dst[i] = src[i];
				}
			}
			dst += dstPitch;
			src += srcPitch;
		}
	}
}

void Graphics::copyMask(uint8 *dst, int dstPitch, int dstX, int dstY, const uint8 *src, int srcPitch, int srcX, int srcY, int w, int h, uint8 fillColor) {
	if (w != 0 && h != 0) {
		dst += dstY * dstPitch + dstX;
		src += srcY * srcPitch + srcX;
		while (h--) {
			for (int i = 0; i < w; ++i) {
				if (src[i] != 0) {
					dst[i] = fillColor;
				}
			}
			dst += dstPitch;
			src += srcPitch;
		}
	}
}

} // namespace Touche
