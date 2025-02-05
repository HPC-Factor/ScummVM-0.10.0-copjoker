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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/engines/agos/vga_e2.cpp $
 * $Id: vga_e2.cpp 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

// Video script opcodes for Simon1/Simon2
#include "common/stdafx.h"

#include "agos/agos.h"
#include "agos/intern.h"

#include "common/system.h"

namespace AGOS {

void AGOSEngine_Elvira2::setupVideoOpcodes(VgaOpcodeProc *op) {
	AGOSEngine::setupVideoOpcodes(op);

	op[17] = &AGOSEngine::vc17_waitEnd;
	op[19] = &AGOSEngine::vc19_loop;
	op[22] = &AGOSEngine::vc22_setPaletteOld;
	op[28] = &AGOSEngine::vc28_playSFX;
	op[32] = &AGOSEngine::vc32_saveScreen;
	op[37] = &AGOSEngine::vc37_pokePalette;
	op[45] = &AGOSEngine::vc45_setWindowPalette;
	op[46] = &AGOSEngine::vc46_setPaletteSlot1;
	op[47] = &AGOSEngine::vc47_setPaletteSlot2;
	op[48] = &AGOSEngine::vc48_setPaletteSlot3;
	op[53] = &AGOSEngine::vc53_dissolveIn;
	op[54] = &AGOSEngine::vc54_dissolveOut;
	op[57] = &AGOSEngine::vc57_blackPalette;
	op[56] = &AGOSEngine::vc56_fullScreen;
	op[58] = &AGOSEngine::vc58_checkCodeWheel;
	op[59] = &AGOSEngine::vc59_ifEGA;
}

void AGOSEngine::vc43_ifBitSet() {
	if (!getBitFlag(vcReadNextWord())) {
		vcSkipNextInstruction();
	}
}

void AGOSEngine::vc44_ifBitClear() {
	if (getBitFlag(vcReadNextWord())) {
		vcSkipNextInstruction();
	}
}

void AGOSEngine::vc45_setWindowPalette() {
	uint num = vcReadNextWord();
	uint color = vcReadNextWord();

	if (num == 4) {
		const uint16 *vlut = &_videoWindows[num * 4];
		uint16 *dst = (uint16 *)_window4BackScn;
		uint width = vlut[2] * 16 / 2;
		uint height = vlut[3];

		for (uint h = 0; h < height; h++) {
			for (uint w = 0; w < width; w++) {
				dst[w] &= 0xF0F;
				dst[w] |= color * 16;
			}
			dst += width;
		}
	} else {
		const uint16 *vlut = &_videoWindows[num * 4];
		uint16 *dst = (uint16 *)getFrontBuf() + vlut[0] * 8 + vlut[1] * _dxSurfacePitch / 2;
		uint width = vlut[2] * 16 / 2;
		uint height = vlut[3];

		if (getGameType() == GType_ELVIRA2 && num == 7) {
			dst -= 4;
			width += 4;
		}

		for (uint h = 0; h < height; h++) {
			for (uint w = 0; w < width; w++) {
				dst[w] &= 0xF0F;
				dst[w] |= color * 16;
			}
			dst += _dxSurfacePitch / 2;
		}
	}
}

void AGOSEngine::setPaletteSlot(uint srcOffs, uint dstOffs) {
	byte *offs, *palptr, *src;
	uint16 num;

	palptr = _displayPalette + dstOffs * 64;
	offs = _curVgaFile1 + READ_BE_UINT16(_curVgaFile1 + 6);
	src = offs + srcOffs * 32;
	num = 16;

	do {
		uint16 color = READ_BE_UINT16(src);
		palptr[0] = ((color & 0xf00) >> 8) * 32;
		palptr[1] = ((color & 0x0f0) >> 4) * 32;
		palptr[2] = ((color & 0x00f) >> 0) * 32;
		palptr[3] = 0;

		palptr += 4;
		src += 2;
	} while (--num);

	_paletteFlag = 2;
}

void AGOSEngine::vc46_setPaletteSlot1() {
	uint srcOffs = vcReadNextWord();
	setPaletteSlot(srcOffs, 1);
}

void AGOSEngine::vc47_setPaletteSlot2() {
	uint srcOffs = vcReadNextWord();
	setPaletteSlot(srcOffs, 2);
}

void AGOSEngine::vc48_setPaletteSlot3() {
	uint srcOffs = vcReadNextWord();
	setPaletteSlot(srcOffs, 3);
}

void AGOSEngine::vc49_setBit() {
	uint16 bit = vcReadNextWord();
	if (getGameType() == GType_FF && bit == 82) {
		_variableArrayPtr = _variableArray2;
	}
	setBitFlag(bit, true);
}

void AGOSEngine::vc50_clearBit() {
	uint16 bit = vcReadNextWord();
	if (getGameType() == GType_FF && bit == 82) {
		_variableArrayPtr = _variableArray;
	}
	setBitFlag(bit, false);
}

void AGOSEngine::vc51_enableBox() {
	enableBox(vcReadNextWord());
}

void AGOSEngine::vc52_playSound() {
	bool ambient = false;

	uint16 sound = vcReadNextWord();
	if (sound >= 0x8000) {
		ambient = true;
		sound = -sound;
	}

	if (getGameType() == GType_FF || getGameType() == GType_PP) {
		int16 pan = vcReadNextWord();
		int16 vol = vcReadNextWord();

		if (ambient)
			loadSound(sound, pan, vol, 2);
		else
			loadSound(sound, pan, vol, 1);
	} else if (getGameType() == GType_SIMON2) {
		if (ambient)
			_sound->playAmbient(sound);
		else
			_sound->playEffects(sound);
	} else if (getFeatures() & GF_TALKIE) {
		_sound->playEffects(sound);
	} else if (getGameId() == GID_SIMON1DOS) {
		playSting(sound);
	} else {
		loadSound(sound);
	}
}

void AGOSEngine::vc53_dissolveIn() {
	uint16 num = vcReadNextWord();
	uint16 speed = vcReadNextWord() + 1;

	byte *src, *dst, *srcOffs, *srcOffs2, *dstOffs, *dstOffs2;
	uint8 color = 0;

	// Only uses Video Window 4
	num = 4;

	uint16 dissolveX = _videoWindows[num * 4 + 2] * 8;
	uint16 dissolveY = (_videoWindows[num * 4 + 3] + 1) / 2;
	uint16 dissolveCheck = dissolveY * dissolveX * 4;
	uint16 dissolveDelay = dissolveCheck * 2 / speed;
	uint16 dissolveCount = dissolveCheck * 2 / speed;

	int16 xoffs = _videoWindows[num * 4 + 0] * 16;
	int16 yoffs = _videoWindows[num * 4 + 1];
	byte *dstPtr = getFrontBuf() + xoffs + yoffs * _screenWidth;

	uint16 count = dissolveCheck * 2;
	while (count--) {
		yoffs = _rnd.getRandomNumber(dissolveY);
		dst = dstPtr + yoffs * _screenWidth;
		src = _window4BackScn + yoffs * 224;

		xoffs = _rnd.getRandomNumber(dissolveX);
		dst += xoffs;
		src += xoffs;

		*dst &= color;
		*dst |= *src & 0xF;

		dstOffs = dst;
		srcOffs = src;

		xoffs = dissolveX * 2 - 1 - (xoffs * 2);
		dst += xoffs;
		src += xoffs;

		*dst &= color;
		*dst |= *src & 0xF;

		srcOffs2 = src;
		dstOffs2 = dst;

		yoffs = (dissolveY - 1) * 2 - (yoffs * 2);
		src = srcOffs + yoffs * 224;
		dst = dstOffs + yoffs * _screenWidth;

		color = 0xF0;
		*dst &= color;
		*dst |= *src & 0xF;

		dst = dstOffs2 + yoffs * _screenWidth;;
		src = srcOffs2 + yoffs * 224;

		*dst &= color;
		*dst |= *src & 0xF;

		dissolveCount--;
		if (!dissolveCount) {
			if (count >= dissolveCheck)
				dissolveDelay++;

			dissolveCount = dissolveDelay;
			_system->copyRectToScreen(getFrontBuf(), _screenWidth, 0, 0, _screenWidth, _screenHeight);
			_system->updateScreen();
			delay(0);
		}
	}
}

void AGOSEngine::vc54_dissolveOut() {
	uint16 num = vcReadNextWord();
	uint16 color = vcReadNextWord();
	uint16 speed = vcReadNextWord() + 1;

	byte *dst, *dstOffs;

	uint16 dissolveX = _videoWindows[num * 4 + 2] * 8;
	uint16 dissolveY = (_videoWindows[num * 4 + 3] + 1) / 2;
	uint16 dissolveCheck = dissolveY * dissolveX * 4;
	uint16 dissolveDelay = dissolveCheck * 2 / speed;
	uint16 dissolveCount = dissolveCheck * 2 / speed;

	int16 xoffs = _videoWindows[num * 4 + 0] * 16;
	int16 yoffs = _videoWindows[num * 4 + 1];
	byte *dstPtr = getFrontBuf() + xoffs + yoffs * _screenWidth;
	color |= dstPtr[0] & 0xF0;

	uint16 count = dissolveCheck * 2;
	while (count--) {
		yoffs = _rnd.getRandomNumber(dissolveY);
		xoffs = _rnd.getRandomNumber(dissolveX);
		dst = dstPtr + xoffs + yoffs * _screenWidth;
		*dst = color;

		dstOffs = dst;

		xoffs = dissolveX * 2 - 1 - (xoffs * 2);
		dst += xoffs;
		*dst = color;

		yoffs = (dissolveY - 1) * 2 - (yoffs * 2);
		dst = dstOffs + yoffs * _screenWidth;
		*dst = color;

		dst += xoffs;
		*dst = color;

		dissolveCount--;
		if (!dissolveCount) {
			if (count >= dissolveCheck)
				dissolveDelay++;

			dissolveCount = dissolveDelay;
			_system->copyRectToScreen(getFrontBuf(), _screenWidth, 0, 0, _screenWidth, _screenHeight);
			_system->updateScreen();
			delay(0);
		}
	}
}

void AGOSEngine::vc55_moveBox() {
	HitArea *ha = _hitAreas;
	uint count = ARRAYSIZE(_hitAreas);
	uint16 id = vcReadNextWord();
	int16 x = vcReadNextWord();
	int16 y = vcReadNextWord();

	for (;;) {
		if (ha->id == id) {
			ha->x += x;
			ha->y += y;
			break;
		}
		ha++;
		if (!--count)
			break;
	}

	_needHitAreaRecalc++;
}

void AGOSEngine::vc56_fullScreen() {
	byte *src = _curVgaFile2 + 32;
	byte *dst = getFrontBuf();

	memcpy(dst, src + 768, _screenHeight * _screenWidth);

	//fullFade();

	uint8 palette[1024];
	for (int i = 0; i < 256; i++) {
		palette[i * 4 + 0] = *src++ * 4;
		palette[i * 4 + 1] = *src++ * 4;
		palette[i * 4 + 2] = *src++ * 4;
		palette[i * 4 + 3] = 0;
	}

	_system->setPalette(palette, 0, 256);
}

void AGOSEngine::vc57_blackPalette() {
	uint8 palette[1024];
	memset(palette, 0, sizeof(palette));
	_system->setPalette(palette, 0, 256);
}

void AGOSEngine::vc58_checkCodeWheel() {
	_variableArray[0] = 0;
}

void AGOSEngine::vc59_ifEGA() {
	// Skip if not EGA
	vcSkipNextInstruction();
}

} // End of namespace AGOS
