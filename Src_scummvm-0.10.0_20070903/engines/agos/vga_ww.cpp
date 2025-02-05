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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/engines/agos/vga_ww.cpp $
 * $Id: vga_ww.cpp 27052 2007-06-02 13:35:24Z Kirben $
 *
 */

// Video script opcodes for Simon1/Simon2
#include "common/stdafx.h"

#include "agos/agos.h"
#include "agos/intern.h"

#include "common/system.h"

namespace AGOS {

void AGOSEngine_Waxworks::setupVideoOpcodes(VgaOpcodeProc *op) {
	AGOSEngine_Elvira2::setupVideoOpcodes(op);

	op[58] = &AGOSEngine::vc58_checkCodeWheel;
	op[60] = &AGOSEngine::vc60_stopAnimation;
	op[61] = &AGOSEngine::vc61;
	op[62] = &AGOSEngine::vc62_fastFadeOut;
	op[63] = &AGOSEngine::vc63_fastFadeIn;
}

void AGOSEngine::vcStopAnimation(uint zone, uint sprite) {
	uint16 old_sprite_id;
	VgaSprite *vsp;
	VgaTimerEntry *vte;
	const byte *vcPtrOrg;

	old_sprite_id = _vgaCurSpriteId;
	vcPtrOrg = _vcPtr;

	_vgaCurSpriteId = sprite;

	vsp = findCurSprite();
	if (vsp->id) {
		vc25_halt_sprite();

		vte = _vgaTimerList;
		while (vte->delay) {
			if (vte->sprite_id == _vgaCurSpriteId) {
				deleteVgaEvent(vte);
				break;
			}
			vte++;
		}
	}

	_vgaCurSpriteId = old_sprite_id;
	_vcPtr = vcPtrOrg;
}

void AGOSEngine_Simon1::vcStopAnimation(uint zone, uint sprite) {
	uint16 old_sprite_id, old_cur_file_id;
	VgaSleepStruct *vfs;
	VgaSprite *vsp;
	VgaTimerEntry *vte;
	const byte *vcPtrOrg;

	old_sprite_id = _vgaCurSpriteId;
	old_cur_file_id = _vgaCurZoneNum;
	vcPtrOrg = _vcPtr;

	_vgaCurZoneNum = zone;
	_vgaCurSpriteId = sprite;

	vfs = _waitSyncTable;
	while (vfs->ident != 0) {
		if (vfs->sprite_id == _vgaCurSpriteId && ((getGameType() == GType_SIMON1) || vfs->cur_vga_file == _vgaCurZoneNum)) {
			while (vfs->ident != 0) {
				memcpy(vfs, vfs + 1, sizeof(VgaSleepStruct));
				vfs++;
			}
			break;
		}
		vfs++;
	}

	vsp = findCurSprite();
	if (vsp->id) {
		vc25_halt_sprite();

		vte = _vgaTimerList;
		while (vte->delay) {
			if (vte->sprite_id == _vgaCurSpriteId && (getGameType() == GType_SIMON1 || vte->cur_vga_file == _vgaCurZoneNum)) {
				deleteVgaEvent(vte);
				break;
			}
			vte++;
		}
	}

	_vgaCurZoneNum = old_cur_file_id;
	_vgaCurSpriteId = old_sprite_id;
	_vcPtr = vcPtrOrg;
}

void AGOSEngine::vc60_stopAnimation() {
	uint16 sprite, zoneNum;

	if (getGameType() == GType_PP) {
		zoneNum = vcReadNextWord();
		sprite = vcReadVarOrWord();
	} else if (getGameType() == GType_SIMON2 || getGameType() == GType_FF) {
		zoneNum = vcReadNextWord();
		sprite = vcReadNextWord();
	} else {
		zoneNum = _vgaCurZoneNum;
		sprite = vcReadNextWord();
	}

	vcStopAnimation(zoneNum, sprite);
}

void AGOSEngine::vc61() {
	uint16 a = vcReadNextWord();
	byte *src, *dst, *dstPtr;
	uint h, tmp;

	if (a == 6) {
		src = _curVgaFile2 + 800;
		dstPtr = getFrontBuf();
		memcpy(dstPtr, src, 64000);
		tmp = 4 - 1;
	} else {
		dstPtr = getFrontBuf();
		tmp = a - 1;
	}

	src = _curVgaFile2 + 3840 * 16 + 3360;
	while (tmp--)
		src += 1536 * 16 + 1712;


	src += 800;

	if (a != 5) {
		dst = dstPtr + 7448;
		for (h = 0; h < 177; h++) {
			memcpy(dst, src, 144);
			src += 144;
			dst += _screenWidth;
		}

		if (a != 6)
			return;

		src = _curVgaFile2 + 9984 * 16 + 15344;
	}

	dst = dstPtr + 50296;
	for (h = 0; h < 17; h++) {
		memcpy(dst, src, 208);
		src += 208;
		dst += _screenWidth;
	}

	if (a == 6) {
		//fullFade();
		src = _curVgaFile2 + 32;

		uint8 palette[1024];
		for (int i = 0; i < 256; i++) {
			palette[i * 4 + 0] = *src++ * 4;
			palette[i * 4 + 1] = *src++ * 4;
			palette[i * 4 + 2] = *src++ * 4;
			palette[i * 4 + 3] = 0;
		}

		_system->setPalette(palette, 0, 256);
	}
}

void AGOSEngine::vc62_fastFadeOut() {
	vc29_stopAllSounds();

	if (!_fastFadeOutFlag) {
		uint i, fadeSize, fadeCount;

		_fastFadeCount = 256;
		if (getGameType() == GType_SIMON1 || getGameType() == GType_SIMON2) {
			if (_windowNum == 4)
				_fastFadeCount = 208;
		}

		memcpy(_videoBuf1, _currentPalette, _fastFadeCount * 4);

		if (getGameType() == GType_FF || getGameType() == GType_PP) {
			if (getGameType() == GType_FF && getBitFlag(75)) {
				fadeCount = 4;
				fadeSize = 64;
			} else {
				fadeCount = 32;
				fadeSize = 8;
			}
		} else {
			fadeCount = 64;
			fadeSize = 4;
		}

		for (i = fadeCount; i != 0; --i) {
			paletteFadeOut(_videoBuf1, _fastFadeCount, fadeSize);
			_system->setPalette(_videoBuf1, 0, _fastFadeCount);
			delay(5);
		}

		if (getGameType() == GType_FF || getGameType() == GType_PP) {
			clearSurfaces(_screenHeight);
		} else if (getGameType() == GType_WW) {
			memset(getFrontBuf(), 0, _screenWidth * _screenHeight);
		} else {
			if (_windowNum != 4) {
				memset(getFrontBuf(), 0, _screenWidth * _screenHeight);
			}
		}
	}
	if (getGameType() == GType_SIMON2) {
		if (_nextMusicToPlay != -1)
			loadMusic(_nextMusicToPlay);
	}
}

void AGOSEngine::vc63_fastFadeIn() {
	if (getGameType() == GType_PP) {
		_fastFadeInFlag = 256;
		if (getBitFlag(100)) {
			startOverlayAnims();
		} else if (getBitFlag(103)) {
			printf("NameAndTime\n");
		} else if (getBitFlag(104)) {
			printf("HiScoreTable\n");
		}
	} else if (getGameType() == GType_FF) {
		_fastFadeInFlag = 256;
	} else if (getGameType() == GType_SIMON1 || getGameType() == GType_SIMON2) {
		_fastFadeInFlag = 208;
		if (_windowNum != 4) {
			_fastFadeInFlag = 256;
		}
	}
	_fastFadeOutFlag = false;
}

} // End of namespace AGOS
