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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/engines/agos/vga_ff.cpp $
 * $Id: vga_ff.cpp 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

#include "common/stdafx.h"

#include "agos/agos.h"
#include "agos/intern.h"

namespace AGOS {

void AGOSEngine_Feeble::setupVideoOpcodes(VgaOpcodeProc *op) {
	AGOSEngine_Simon2::setupVideoOpcodes(op);

	op[75] = &AGOSEngine::vc75_setScale;
	op[76] = &AGOSEngine::vc76_setScaleXOffs;
	op[77] = &AGOSEngine::vc77_setScaleYOffs;
	op[78] = &AGOSEngine::vc78_computeXY;
	op[79] = &AGOSEngine::vc79_computePosNum;
	op[80] = &AGOSEngine::vc80_setOverlayImage;
	op[81] = &AGOSEngine::vc81_setRandom;
	op[82] = &AGOSEngine::vc82_getPathValue;
	op[83] = &AGOSEngine::vc83_playSoundLoop;
	op[84] = &AGOSEngine::vc84_stopSoundLoop;
}

int AGOSEngine::getScale(int16 y, int16 x) {
	int16 z;

	if (y > _baseY) {
		return((int16)(x * (1 + ((y - _baseY) * _scale))));
	} else {	
		if (x == 0)
			return(0);
		if (x < 0) {
			z = ((int16)((x * (1 - ((_baseY - y)* _scale))) - 0.5));
			if (z >- 2)
				return(-2);
			return(z);
		}

		z = ((int16)((x * (1 - ((_baseY - y) * _scale))) + 0.5));
		if (z < 2)
			return(2);

		return(z);
	}
}

void AGOSEngine::vc75_setScale() {
	_baseY = vcReadNextWord();
	_scale = (float)vcReadNextWord() / 1000000.;
}

void AGOSEngine::vc76_setScaleXOffs() {
	if (getGameType() == GType_PP && getBitFlag(120)) {
		VgaSprite *vsp1, *vsp2;
		uint16 old_file_1, tmp1, tmp2;

		old_file_1 = _vgaCurSpriteId;

		_vgaCurSpriteId = vcReadVar(vcReadNextWord());
		 vsp1 = findCurSprite();
		_vgaCurSpriteId = vcReadVar(vcReadNextWord());
		 vsp2 = findCurSprite();

		tmp1 = vsp1->x;
		tmp2 = vsp2->x;
		vsp1->x = tmp2;
		vsp2->x = tmp1;
		tmp1 = vsp1->y;
		tmp2 = vsp2->y;
		vsp1->y = tmp2;
		vsp2->y = tmp1;

		_vgaCurSpriteId = old_file_1;
		_vcPtr += 2;
	} else {
		VgaSprite *vsp = findCurSprite();

		vsp->image = vcReadNextWord();
		int16 x = vcReadNextWord();
		uint16 var = vcReadNextWord();

		vsp->x += getScale(vsp->y, x);
		_variableArrayPtr[var] = vsp->x;

		checkScrollX(x, vsp->x);

		vsp->flags = kDFScaled;
	}
}

void AGOSEngine::vc77_setScaleYOffs() {
	VgaSprite *vsp = findCurSprite();

	vsp->image = vcReadNextWord();
	int16 y = vcReadNextWord();
	uint16 var = vcReadNextWord();

	vsp->y += getScale(vsp->y, y);
	_variableArrayPtr[var] = vsp->y;

	if (y != 0) 
		checkScrollY(y, vsp->y);

	vsp->flags = kDFScaled;
}

void AGOSEngine::vc78_computeXY() {
	VgaSprite *vsp = findCurSprite();

	uint16 a = (uint16)_variableArrayPtr[12];
	uint16 b = (uint16)_variableArrayPtr[13];

	const uint16 *p = _pathFindArray[a - 1];
	p += b * 2;

	uint16 posx = readUint16Wrapper(p);
	_variableArrayPtr[15] = posx;
	vsp->x = posx;

	uint16 posy = readUint16Wrapper(p + 1);
	_variableArrayPtr[16] = posy;
	vsp->y = posy;

	if (getGameType() == GType_FF) {
		setBitFlag(85, false);
		if (getBitFlag(74)) {
			centreScroll();
		}
	}
}

void AGOSEngine::vc79_computePosNum() {
	uint a = (uint16)_variableArrayPtr[12];
	const uint16 *p = _pathFindArray[a - 1];
	uint pos = 0;

	int16 y = _variableArrayPtr[16];
	while (y >= (int16)readUint16Wrapper(p + 1)) {
		p += 2;
		pos++;
	}

	_variableArrayPtr[13] = pos;
}

void AGOSEngine::vc80_setOverlayImage() {
	VgaSprite *vsp = findCurSprite();

	vsp->image = vcReadVarOrWord();

	vsp->x += vcReadNextWord();
	vsp->y += vcReadNextWord();
	vsp->flags = kDFOverlayed;

	_vgaSpriteChanged++;
}

void AGOSEngine::vc81_setRandom() {
	uint16 var = vcReadNextWord();
	uint16 value = vcReadNextWord();

	_variableArray[var] = _rnd.getRandomNumber(value - 1);
}

void AGOSEngine::vc82_getPathValue() {
	uint8 val;

	uint16 var = vcReadNextWord();

	if (getGameType() == GType_FF && getBitFlag(82)) {
		val = _pathValues1[_GPVCount1++];
	} else {
		val = _pathValues[_GPVCount++];
	}

	vcWriteVar(var, val);
}

void AGOSEngine::vc83_playSoundLoop() {
	uint16 sound = vcReadNextWord();
	int16 vol = vcReadNextWord();
	int16 pan = vcReadNextWord();

	loadSound(sound, pan, vol, 3);
}

void AGOSEngine::vc84_stopSoundLoop() {
	_sound->stopSfx5();
}

// Scrolling functions for Feeble Files
void AGOSEngine::checkScrollX(int16 x, int16 xpos) {
	if (_scrollXMax == 0 || x == 0)
		return;

	if ((getGameType() == GType_FF) && (getBitFlag(80) || getBitFlag(82)))
		return;

	int16 tmp;
	if (x > 0) {
		if (_scrollCount != 0) {
			if (_scrollCount >= 0)
				return;
			_scrollCount = 0;
		} else {
			if (_scrollFlag != 0)
				return;
		}

		if (xpos - _scrollX >= 480) {
			_scrollCount = 320;
			tmp = _scrollXMax - _scrollX;
			if (tmp < 320)
				_scrollCount = tmp;
		}
	} else {
		if (_scrollCount != 0) {
			if (_scrollCount < 0)
				return;
			_scrollCount = 0;
		} else {
			if (_scrollFlag != 0)
				return;
		}

		if (xpos - _scrollX < 161) {
			_scrollCount = -320;
			if (_scrollX < 320)
				_scrollCount = -_scrollX;
		}
	}
}

void AGOSEngine::checkScrollY(int16 y, int16 ypos) {
	if (_scrollYMax == 0)
		return;

	if (getGameType() == GType_FF && getBitFlag(80))
		return;

	int16 tmp;
	if (y >= 0) {
		if (_scrollCount != 0) {
			if (_scrollCount >= 0)
				return;
		} else {
			if (_scrollFlag != 0)
				return;
		}

		if (ypos - _scrollY >= 440) {
			_scrollCount = 240;
			tmp = _scrollYMax - _scrollY;
			if (tmp < 240)
				_scrollCount = tmp;
		}
	} else {
		if (_scrollCount != 0) {
			if (_scrollCount < 0)
				return;
		} else {
			if (_scrollFlag != 0)
				return;
		}

		if (ypos - _scrollY < 100) {
			_scrollCount = -240;
			if (_scrollY < 240)
				_scrollCount = -_scrollY;
		}
	}
}

void AGOSEngine::centreScroll() {
	int16 x, y, tmp;

	if (_scrollXMax != 0) {
		_scrollCount = 0;
		x = _variableArray[15] - _scrollX;
		if (x < 17 || (getBitFlag(85) && x < 320)) {
			x -= 320;
			if (_scrollX < -x)
				x = -_scrollX;
			_scrollCount = x;
		} else if ((getBitFlag(85) && x >= 320) || x >= 624) {
			x -= 320;
			tmp = _scrollXMax - _scrollX;
			if (tmp < x)
				x = tmp;
			_scrollCount = x;
		}
	} else if (_scrollYMax != 0) {
		_scrollCount = 0;
		y = _variableArray[16] - _scrollY;
		if (y < 30) {
			y -= 240;
			if (_scrollY < -y)
				y = -_scrollY;
			_scrollCount = y;
		} else if (y >= 460) {
			y -= 240;
			tmp = _scrollYMax - _scrollY;
			if (tmp < y)
				y = tmp;
			_scrollCount = y;
		}
	}
}

void AGOSEngine::startOverlayAnims() {
	VgaSprite *vsp = _vgaSprites;
	uint16 zoneNum;
	int i;

	zoneNum = _variableArray[999];
	
	for (i = 0; i < 600; i++) {
		if (_variableArray[1000 + i] < 100)
			continue;

		while (vsp->id)
			vsp++;

		vsp->windowNum = 4;
		vsp->priority = 4;
		vsp->flags = 0;
		vsp->palette = 0;
		vsp->image = _variableArray[1000 + i];
		if (i >= 300) {
			vsp->y = ((i - 300) / 20) * 32;
			vsp->x = ((i - 300) % 20) * 32;
		} else {
			vsp->y = (i / 20) * 32;
			vsp->x = (i % 20) * 32;
		}
		vsp->id = 1000 + i;
		vsp->zoneNum = zoneNum;
	}
}

void AGOSEngine::startAnOverlayAnim() {
	VgaSprite *vsp = _vgaSprites;
	const byte *vcPtrOrg;
	uint16 a, sprite, file, tmp, zoneNum;
	int16 x;

	zoneNum = _variableArray[999];

	_vcPtr += 4;
	a = vcReadNextWord();
	_vcPtr += 6;

	while (vsp->id)
		vsp++;

	vsp->windowNum = 4;
	vsp->priority = 20;
	vsp->flags = 0;
	vsp->palette = 0;
	vsp->image = vcReadVar(vcReadVar(a));

	x = vcReadVar(a) - 1300;
	if (x < 0) {
		x += 300;
		vsp->priority = 10;
	}

	vsp->y = x / 20 * 32;
	vsp->x = x % 20 * 32;
	vsp->id = vcReadVar(a);
	vsp->zoneNum = zoneNum;

	sprite = _vgaCurSpriteId;
	file = _vgaCurZoneNum;

	_vgaCurZoneNum = vsp->zoneNum;
	_vgaCurSpriteId = vsp->id;

	tmp = to16Wrapper(vsp->priority);

	vcPtrOrg = _vcPtr;
	_vcPtr = (byte *)&tmp;
	vc23_setPriority();

	_vcPtr = vcPtrOrg;
	_vgaCurSpriteId = sprite;
	_vgaCurZoneNum = file;
}

} // End of namespace AGOS
