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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/engines/gob/inter_v1.cpp $
 * $Id: inter_v1.cpp 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

#include "common/stdafx.h"
#include "common/endian.h"
#include "common/file.h"

#include "gob/gob.h"
#include "gob/inter.h"
#include "gob/global.h"
#include "gob/util.h"
#include "gob/dataio.h"
#include "gob/music.h"
#include "gob/cdrom.h"
#include "gob/draw.h"
#include "gob/game.h"
#include "gob/goblin.h"
#include "gob/inter.h"
#include "gob/map.h"
#include "gob/mult.h"
#include "gob/palanim.h"
#include "gob/parse.h"
#include "gob/scenery.h"
#include "gob/sound.h"
#include "gob/video.h"

namespace Gob {

#define OPCODE(x) _OPCODE(Inter_v1, x)

const int Inter_v1::_goblinFuncLookUp[][2] = {
	{1, 0},
	{2, 1},
	{3, 2},
	{4, 3},
	{5, 4},
	{6, 5},
	{7, 6},
	{8, 7},
	{9, 8},
	{10, 9},
	{12, 10},
	{13, 11},
	{14, 12},
	{15, 13},
	{16, 14},
	{21, 15},
	{22, 16},
	{23, 17},
	{24, 18},
	{25, 19},
	{26, 20},
	{27, 21},
	{28, 22},
	{29, 23},
	{30, 24},
	{32, 25},
	{33, 26},
	{34, 27},
	{35, 28},
	{36, 29},
	{37, 30},
	{40, 31},
	{41, 32},
	{42, 33},
	{43, 34},
	{44, 35},
	{50, 36},
	{52, 37},
	{53, 38},
	{150, 39},
	{152, 40},
	{200, 41},
	{201, 42},
	{202, 43},
	{203, 44},
	{204, 45},
	{250, 46},
	{251, 47},
	{252, 48},
	{500, 49},
	{502, 50},
	{503, 51},
	{600, 52},
	{601, 53},
	{602, 54},
	{603, 55},
	{604, 56},
	{605, 57},
	{1000, 58},
	{1001, 59},
	{1002, 60},
	{1003, 61},
	{1004, 62},
	{1005, 63},
	{1006, 64},
	{1008, 65},
	{1009, 66},
	{1010, 67},
	{1011, 68},
	{1015, 69},
	{2005, 70}
};

Inter_v1::Inter_v1(GobEngine *vm) : Inter(vm) {
	setupOpcodes();
}

void Inter_v1::setupOpcodes() {
	static const OpcodeDrawEntryV1 opcodesDraw[256] = {
		/* 00 */
		OPCODE(o1_loadMult),
		OPCODE(o1_playMult),
		OPCODE(o1_freeMultKeys),
		{NULL, ""},
		/* 04 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		OPCODE(o1_initCursor),
		/* 08 */
		OPCODE(o1_initCursorAnim),
		OPCODE(o1_clearCursorAnim),
		OPCODE(o1_setRenderFlags),
		{NULL, ""},
		/* 0C */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 10 */
		OPCODE(o1_loadAnim),
		OPCODE(o1_freeAnim),
		OPCODE(o1_updateAnim),
		{NULL, ""},
		/* 14 */
		OPCODE(o1_initMult),
		OPCODE(o1_freeMult),
		OPCODE(o1_animate),
		OPCODE(o1_loadMultObject),
		/* 18 */
		OPCODE(o1_getAnimLayerInfo),
		OPCODE(o1_getObjAnimSize),
		OPCODE(o1_loadStatic),
		OPCODE(o1_freeStatic),
		/* 1C */
		OPCODE(o1_renderStatic),
		OPCODE(o1_loadCurLayer),
		{NULL, ""},
		{NULL, ""},
		/* 20 */
		OPCODE(o1_playCDTrack),
		OPCODE(o1_getCDTrackPos),
		OPCODE(o1_stopCD),
		{NULL, ""},
		/* 24 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 28 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 2C */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 30 */
		OPCODE(o1_loadFontToSprite),
		OPCODE(o1_freeFontToSprite),
		{NULL, ""},
		{NULL, ""},
		/* 34 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 38 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 3C */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 40 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 44 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 48 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 4C */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 50 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 54 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 58 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 5C */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 60 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 64 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 68 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 6C */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 70 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 74 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 78 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 7C */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 80 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 84 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 88 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 8C */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 90 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 94 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 98 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 9C */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* A0 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* A4 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* A8 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* AC */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* B0 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* B4 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* B8 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* BC */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* C0 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* C4 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* C8 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* CC */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* D0 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* D4 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* D8 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* DC */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* E0 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* E4 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* E8 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* EC */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* F0 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* F4 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* F8 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* FC */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""}
	};

	static const OpcodeFuncEntryV1 opcodesFunc[80] = {
		/* 00 */
		OPCODE(o1_callSub),
		OPCODE(o1_callSub),
		OPCODE(o1_printTotText),
		OPCODE(o1_loadCursor),
		/* 04 */
		{NULL, ""},
		OPCODE(o1_switch),
		OPCODE(o1_repeatUntil),
		OPCODE(o1_whileDo),
		/* 08 */
		OPCODE(o1_if),
		OPCODE(o1_evaluateStore),
		OPCODE(o1_loadSpriteToPos),
		{NULL, ""},
		/* 0C */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 10 */
		{NULL, ""},
		OPCODE(o1_printText),
		OPCODE(o1_loadTot),
		OPCODE(o1_palLoad),
		/* 14 */
		OPCODE(o1_keyFunc),
		OPCODE(o1_capturePush),
		OPCODE(o1_capturePop),
		OPCODE(o1_animPalInit),
		/* 18 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 1C */
		{NULL, ""},
		{NULL, ""},
		OPCODE(o1_drawOperations),
		OPCODE(o1_setcmdCount),
		/* 20 */
		OPCODE(o1_return),
		OPCODE(o1_renewTimeInVars),
		OPCODE(o1_speakerOn),
		OPCODE(o1_speakerOff),
		/* 24 */
		OPCODE(o1_putPixel),
		OPCODE(o1_goblinFunc),
		OPCODE(o1_createSprite),
		OPCODE(o1_freeSprite),
		/* 28 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 2C */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 30 */
		OPCODE(o1_returnTo),
		OPCODE(o1_loadSpriteContent),
		OPCODE(o1_copySprite),
		OPCODE(o1_fillRect),
		/* 34 */
		OPCODE(o1_drawLine),
		OPCODE(o1_strToLong),
		OPCODE(o1_invalidate),
		OPCODE(o1_setBackDelta),
		/* 38 */
		OPCODE(o1_playSound),
		OPCODE(o1_stopSound),
		OPCODE(o1_loadSound),
		OPCODE(o1_freeSoundSlot),
		/* 3C */
		OPCODE(o1_waitEndPlay),
		OPCODE(o1_playComposition),
		OPCODE(o1_getFreeMem),
		OPCODE(o1_checkData),
		/* 40 */
		{NULL, ""},
		OPCODE(o1_prepareStr),
		OPCODE(o1_insertStr),
		OPCODE(o1_cutStr),
		/* 44 */
		OPCODE(o1_strstr),
		OPCODE(o1_istrlen),
		OPCODE(o1_setMousePos),
		OPCODE(o1_setFrameRate),
		/* 48 */
		OPCODE(o1_animatePalette),
		OPCODE(o1_animateCursor),
		OPCODE(o1_blitCursor),
		OPCODE(o1_loadFont),
		/* 4C */
		OPCODE(o1_freeFont),
		OPCODE(o1_readData),
		OPCODE(o1_writeData),
		OPCODE(o1_manageDataFile),
	};

	static const OpcodeGoblinEntryV1 opcodesGoblin[71] = {
		/* 00 */
		OPCODE(o1_setState),
		OPCODE(o1_setCurFrame),
		OPCODE(o1_setNextState),
		OPCODE(o1_setMultState),
		/* 04 */
		OPCODE(o1_setOrder),
		OPCODE(o1_setActionStartState),
		OPCODE(o1_setCurLookDir),
		OPCODE(o1_setType),
		/* 08 */
		OPCODE(o1_setNoTick),
		OPCODE(o1_setPickable),
		OPCODE(o1_setXPos),
		OPCODE(o1_setYPos),
		/* 0C */
		OPCODE(o1_setDoAnim),
		OPCODE(o1_setRelaxTime),
		OPCODE(o1_setMaxTick),
		OPCODE(o1_getState),
		/* 10 */
		OPCODE(o1_getCurFrame),
		OPCODE(o1_getNextState),
		OPCODE(o1_getMultState),
		OPCODE(o1_getOrder),
		/* 14 */
		OPCODE(o1_getActionStartState),
		OPCODE(o1_getCurLookDir),
		OPCODE(o1_getType),
		OPCODE(o1_getNoTick),
		/* 18 */
		OPCODE(o1_getPickable),
		OPCODE(o1_getObjMaxFrame),
		OPCODE(o1_getXPos),
		OPCODE(o1_getYPos),
		/* 1C */
		OPCODE(o1_getDoAnim),
		OPCODE(o1_getRelaxTime),
		OPCODE(o1_getMaxTick),
		OPCODE(o1_manipulateMap),
		/* 20 */
		OPCODE(o1_getItem),
		OPCODE(o1_manipulateMapIndirect),
		OPCODE(o1_getItemIndirect),
		OPCODE(o1_setPassMap),
		/* 24 */
		OPCODE(o1_setGoblinPosH),
		OPCODE(o1_getGoblinPosXH),
		OPCODE(o1_getGoblinPosYH),
		OPCODE(o1_setGoblinMultState),
		/* 28 */
		OPCODE(o1_setGoblinUnk14),
		OPCODE(o1_setItemIdInPocket),
		OPCODE(o1_setItemIndInPocket),
		OPCODE(o1_getItemIdInPocket),
		/* 2C */
		OPCODE(o1_getItemIndInPocket),
		OPCODE(o1_setItemPos),
		OPCODE(o1_setGoblinPos),
		OPCODE(o1_setGoblinState),
		/* 30 */
		OPCODE(o1_setGoblinStateRedraw),
		OPCODE(o1_decRelaxTime),
		OPCODE(o1_getGoblinPosX),
		OPCODE(o1_getGoblinPosY),
		/* 34 */
		OPCODE(o1_clearPathExistence),
		OPCODE(o1_setGoblinVisible),
		OPCODE(o1_setGoblinInvisible),
		OPCODE(o1_getObjectIntersect),
		/* 38 */
		OPCODE(o1_getGoblinIntersect),
		OPCODE(o1_setItemPos),
		OPCODE(o1_loadObjects),
		OPCODE(o1_freeObjects),
		/* 3C */
		OPCODE(o1_animateObjects),
		OPCODE(o1_drawObjects),
		OPCODE(o1_loadMap),
		OPCODE(o1_moveGoblin),
		/* 40 */
		OPCODE(o1_switchGoblin),
		OPCODE(o1_loadGoblin),
		OPCODE(o1_writeTreatItem),
		OPCODE(o1_moveGoblin0),
		/* 44 */
		OPCODE(o1_setGoblinTarget),
		OPCODE(o1_setGoblinObjectsPos),
		OPCODE(o1_initGoblin)
	};

	_opcodesDrawV1 = opcodesDraw;
	_opcodesFuncV1 = opcodesFunc;
	_opcodesGoblinV1 = opcodesGoblin;
}

void Inter_v1::executeDrawOpcode(byte i) {
	debugC(1, kDebugDrawOp, "opcodeDraw %d [0x%X] (%s)",
			i, i, getOpcodeDrawDesc(i));

	OpcodeDrawProcV1 op = _opcodesDrawV1[i].proc;

	if (op == NULL)
		warning("unimplemented opcodeDraw: %d", i);
	else
		(this->*op) ();
}

bool Inter_v1::executeFuncOpcode(byte i, byte j, OpFuncParams &params) {
	debugC(1, kDebugFuncOp, "opcodeFunc %d.%d [0x%X.0x%X] (%s)",
			i, j, i, j, getOpcodeFuncDesc(i, j));

	if ((i > 4) || (j > 15)) {
		warning("unimplemented opcodeFunc: %d.%d", i, j);
		return false;
	}

	OpcodeFuncProcV1 op = _opcodesFuncV1[i*16 + j].proc;

	if (op == NULL)
		warning("unimplemented opcodeFunc: %d.%d", i, j);
	else
		return (this->*op) (params);
	return false;
}

void Inter_v1::executeGoblinOpcode(int i, OpGobParams &params) {
	debugC(1, kDebugGobOp, "opcodeGoblin %d [0x%X] (%s)",
			i, i, getOpcodeGoblinDesc(i));

	OpcodeGoblinProcV1 op = NULL;

	for (int j = 0; j < ARRAYSIZE(_goblinFuncLookUp); j++)
		if (_goblinFuncLookUp[j][0] == i) {
			op = _opcodesGoblinV1[_goblinFuncLookUp[j][1]].proc;
			break;
		}

	if (op == NULL) {
		warning("unimplemented opcodeGoblin: %d", i);
		_vm->_global->_inter_execPtr -= 2;
		int16 cmd = load16();
		_vm->_global->_inter_execPtr += cmd * 2;
	} else
		(this->*op) (params);
}

const char *Inter_v1::getOpcodeDrawDesc(byte i) {
	return _opcodesDrawV1[i].desc;
}

const char *Inter_v1::getOpcodeFuncDesc(byte i, byte j) {
	if ((i > 4) || (j > 15))
		return "";

	return _opcodesFuncV1[i*16 + j].desc;
}

const char *Inter_v1::getOpcodeGoblinDesc(int i) {
	for (int j = 0; j < ARRAYSIZE(_goblinFuncLookUp); j++)
		if (_goblinFuncLookUp[j][0] == i)
			return _opcodesGoblinV1[_goblinFuncLookUp[j][1]].desc;
	return "";
}

void Inter_v1::checkSwitchTable(byte **ppExec) {
	int16 len;
	int32 value;
	bool found;
	bool notFound;

	found = false;
	notFound = true;
	*ppExec = 0;
	value = VAR_OFFSET(_vm->_parse->parseVarIndex());

	len = (int8) *_vm->_global->_inter_execPtr++;
	while (len != -5) {
		for (int i = 0; i < len; i++) {
			evalExpr(0);

			if (_terminate)
				return;

			if (_vm->_global->_inter_resVal == value) {
				found = true;
				notFound = false;
			}
		}

		if (found)
			*ppExec = _vm->_global->_inter_execPtr;

		_vm->_global->_inter_execPtr +=
			READ_LE_UINT16(_vm->_global->_inter_execPtr + 2) + 2;
		found = false;
		len = (int8) *_vm->_global->_inter_execPtr++;
	} 

	if ((*_vm->_global->_inter_execPtr >> 4) != 4)
		return;

	_vm->_global->_inter_execPtr++;
	if (notFound)
		*ppExec = _vm->_global->_inter_execPtr;

	_vm->_global->_inter_execPtr +=
		READ_LE_UINT16(_vm->_global->_inter_execPtr + 2) + 2;
}

void Inter_v1::o1_loadMult() {
	_vm->_mult->loadMult(load16());
}

void Inter_v1::o1_playMult() {
	int16 checkEscape;

	checkEscape = load16();
	_vm->_mult->playMult(VAR(57), -1, checkEscape, 0);
}

void Inter_v1::o1_freeMultKeys() {
	load16();
	_vm->_mult->freeMultKeys();
}

void Inter_v1::o1_initCursor() {
	int16 width;
	int16 height;
	int16 count;

	_vm->_draw->_cursorHotspotXVar = _vm->_parse->parseVarIndex() / 4;
	_vm->_draw->_cursorHotspotYVar = _vm->_parse->parseVarIndex() / 4;

	width = load16();
	if (width < 16)
		width = 16;

	height = load16();
	if (height < 16)
		height = 16;

	_vm->_draw->adjustCoords(0, &width, &height);

	count = load16();
	if (count < 2)
		count = 2;

	if ((width != _vm->_draw->_cursorWidth) ||
			(height != _vm->_draw->_cursorHeight) ||
	    (_vm->_draw->_cursorSprites->getWidth() != (width * count))) {

		_vm->_draw->freeSprite(23);
		_vm->_draw->_cursorSprites = 0;
		_vm->_draw->_cursorSpritesBack = 0;
		_vm->_draw->_scummvmCursor = 0;

		_vm->_draw->_cursorWidth = width;
		_vm->_draw->_cursorHeight = height;

		if (count < 0x80)
			_vm->_draw->_transparentCursor = 1;
		else
			_vm->_draw->_transparentCursor = 0;

		if (count > 0x80)
			count -= 0x80;

		_vm->_draw->initSpriteSurf(23, _vm->_draw->_cursorWidth * count,
				_vm->_draw->_cursorHeight, 2);
		_vm->_draw->_cursorSpritesBack = _vm->_draw->_spritesArray[23];
		_vm->_draw->_cursorSprites = _vm->_draw->_cursorSpritesBack;

		_vm->_draw->_scummvmCursor =
			_vm->_video->initSurfDesc(_vm->_global->_videoMode,
					_vm->_draw->_cursorWidth, _vm->_draw->_cursorHeight,
					SCUMMVM_CURSOR);

		for (int i = 0; i < 40; i++) {
			_vm->_draw->_cursorAnimLow[i] = -1;
			_vm->_draw->_cursorAnimDelays[i] = 0;
			_vm->_draw->_cursorAnimHigh[i] = 0;
		}
		_vm->_draw->_cursorAnimLow[1] = 0;
	}
}

void Inter_v1::o1_initCursorAnim() {
	int16 ind;

	_vm->_draw->_showCursor = 3;
	ind = _vm->_parse->parseValExpr();
	_vm->_draw->_cursorAnimLow[ind] = load16();
	_vm->_draw->_cursorAnimHigh[ind] = load16();
	_vm->_draw->_cursorAnimDelays[ind] = load16();
}

void Inter_v1::o1_clearCursorAnim() {
	int16 ind;

	_vm->_draw->_showCursor = 0;
	ind = _vm->_parse->parseValExpr();
	_vm->_draw->_cursorAnimLow[ind] = -1;
	_vm->_draw->_cursorAnimHigh[ind] = 0;
	_vm->_draw->_cursorAnimDelays[ind] = 0;
}

void Inter_v1::o1_setRenderFlags() {
	_vm->_draw->_renderFlags = _vm->_parse->parseValExpr();
}

void Inter_v1::o1_loadAnim() {
	_vm->_scenery->loadAnim(0);
}

void Inter_v1::o1_freeAnim() {
	_vm->_scenery->freeAnim(-1);
}

void Inter_v1::o1_updateAnim() {
	int16 deltaX;
	int16 deltaY;
	int16 flags;
	int16 frame;
	int16 layer;
	int16 animation;

	evalExpr(&deltaX);
	evalExpr(&deltaY);
	evalExpr(&animation);
	evalExpr(&layer);
	evalExpr(&frame);
	flags = load16();
	_vm->_scenery->updateAnim(layer, frame, animation, flags,
			deltaX, deltaY, 1);
}

void Inter_v1::o1_initMult() {
	int16 oldAnimHeight;
	int16 oldAnimWidth;
	int16 oldObjCount;
	int16 posXVar;
	int16 posYVar;
	int16 animDataVar;

	oldAnimWidth = _vm->_mult->_animWidth;
	oldAnimHeight = _vm->_mult->_animHeight;
	oldObjCount = _vm->_mult->_objCount;

	_vm->_mult->_animLeft = load16();
	_vm->_mult->_animTop = load16();
	_vm->_mult->_animWidth = load16();
	_vm->_mult->_animHeight = load16();
	_vm->_mult->_objCount = load16();
	posXVar = _vm->_parse->parseVarIndex();
	posYVar = _vm->_parse->parseVarIndex();
	animDataVar = _vm->_parse->parseVarIndex();

	if (_vm->_mult->_objects && (oldObjCount != _vm->_mult->_objCount)) {
		warning("Initializing new objects without having "
				"cleaned up the old ones at first");
		delete[] _vm->_mult->_objects;
		delete[] _vm->_mult->_renderData;
		_vm->_mult->_objects = 0;
		_vm->_mult->_renderObjs = 0;
	}

	if (_vm->_mult->_objects == 0) {
		_vm->_mult->_renderData = new int16[_vm->_mult->_objCount * 9];
		memset(_vm->_mult->_renderData, 0,
				_vm->_mult->_objCount * 9 * sizeof(int16));
		_vm->_mult->_objects = new Mult::Mult_Object[_vm->_mult->_objCount];
		memset(_vm->_mult->_objects, 0,
				_vm->_mult->_objCount * sizeof(Mult::Mult_Object));

		for (int i = 0; i < _vm->_mult->_objCount; i++) {
			_vm->_mult->_objects[i].pPosX =
				(int32 *)(_vm->_global->_inter_variables +
						i * 4 + (posXVar / 4) * 4);
			_vm->_mult->_objects[i].pPosY =
				(int32 *)(_vm->_global->_inter_variables +
						i * 4 + (posYVar / 4) * 4);

			_vm->_mult->_objects[i].pAnimData =
			    (Mult::Mult_AnimData *) (_vm->_global->_inter_variables +
							animDataVar + i * 4 * _vm->_global->_inter_animDataSize);

			_vm->_mult->_objects[i].pAnimData->isStatic = 1;
			_vm->_mult->_objects[i].tick = 0;
			_vm->_mult->_objects[i].lastLeft = -1;
			_vm->_mult->_objects[i].lastRight = -1;
			_vm->_mult->_objects[i].lastTop = -1;
			_vm->_mult->_objects[i].lastBottom = -1;
		}
	}

	if (_vm->_mult->_animSurf &&
	    ((oldAnimWidth != _vm->_mult->_animWidth) ||
			 (oldAnimHeight != _vm->_mult->_animHeight))) {
		_vm->_draw->freeSprite(22);
		_vm->_mult->_animSurf = 0;
	}

	if (!_vm->_mult->_animSurf) {
		_vm->_draw->initSpriteSurf(22, _vm->_mult->_animWidth,
				_vm->_mult->_animHeight, 0);
		_vm->_mult->_animSurf = _vm->_draw->_spritesArray[22];
	}

	_vm->_video->drawSprite(_vm->_draw->_backSurface, _vm->_mult->_animSurf,
	    _vm->_mult->_animLeft, _vm->_mult->_animTop,
	    _vm->_mult->_animLeft + _vm->_mult->_animWidth - 1,
	    _vm->_mult->_animTop + _vm->_mult->_animHeight - 1, 0, 0, 0);

	debugC(4, kDebugGraphics, "o1_initMult: x = %d, y = %d, w = %d, h = %d",
		  _vm->_mult->_animLeft, _vm->_mult->_animTop,
			_vm->_mult->_animWidth, _vm->_mult->_animHeight);
	debugC(4, kDebugGraphics, "    _vm->_mult->_objCount = %d, "
			"animation data size = %d", _vm->_mult->_objCount,
			_vm->_global->_inter_animDataSize);
}

void Inter_v1::o1_freeMult() {
	_vm->_mult->freeMult();
}

void Inter_v1::o1_animate() {
	_vm->_mult->animate();
}

void Inter_v1::o1_loadMultObject() {
	int16 val;
	int16 objIndex;
	byte *multData;

	evalExpr(&objIndex);
	evalExpr(&val);
	*_vm->_mult->_objects[objIndex].pPosX = val;
	evalExpr(&val);
	*_vm->_mult->_objects[objIndex].pPosY = val;

	debugC(4, kDebugGameFlow, "Loading mult object %d", objIndex);

	multData = (byte *) _vm->_mult->_objects[objIndex].pAnimData;
	for (int i = 0; i < 11; i++) {
		if (READ_LE_UINT16(_vm->_global->_inter_execPtr) != 99) {
			evalExpr(&val);
			multData[i] = val;
		} else
			_vm->_global->_inter_execPtr++;
	}
}

void Inter_v1::o1_getAnimLayerInfo() {
	int16 anim;
	int16 layer;
	int16 varDX, varDY;
	int16 varUnk0;
	int16 varFrames;

	evalExpr(&anim);
	evalExpr(&layer);

	varDX = _vm->_parse->parseVarIndex();
	varDY = _vm->_parse->parseVarIndex();
	varUnk0 = _vm->_parse->parseVarIndex();
	varFrames = _vm->_parse->parseVarIndex();

	_vm->_scenery->writeAnimLayerInfo(anim, layer,
			varDX, varDY, varUnk0, varFrames);
}

void Inter_v1::o1_getObjAnimSize() {
	int16 objIndex;

	evalExpr(&objIndex);

	Mult::Mult_AnimData &animData = *(_vm->_mult->_objects[objIndex].pAnimData);
	if (animData.isStatic == 0)
		_vm->_scenery->updateAnim(animData.layer, animData.frame,
		    animData.animation, 0, *(_vm->_mult->_objects[objIndex].pPosX),
		    *(_vm->_mult->_objects[objIndex].pPosY), 0);

	_vm->_scenery->_toRedrawLeft = MAX(_vm->_scenery->_toRedrawLeft, (int16) 0);
	_vm->_scenery->_toRedrawTop = MAX(_vm->_scenery->_toRedrawTop, (int16) 0);
	WRITE_VAR_OFFSET(_vm->_parse->parseVarIndex(), _vm->_scenery->_toRedrawLeft);
	WRITE_VAR_OFFSET(_vm->_parse->parseVarIndex(), _vm->_scenery->_toRedrawTop);
	WRITE_VAR_OFFSET(_vm->_parse->parseVarIndex(), _vm->_scenery->_toRedrawRight);
	WRITE_VAR_OFFSET(_vm->_parse->parseVarIndex(), _vm->_scenery->_toRedrawBottom);
}

void Inter_v1::o1_loadStatic() {
	_vm->_scenery->loadStatic(0);
}

void Inter_v1::o1_freeStatic() {
	_vm->_scenery->freeStatic(-1);
}

void Inter_v1::o1_renderStatic() {
	int16 layer;
	int16 index;

	_vm->_inter->evalExpr(&index);
	_vm->_inter->evalExpr(&layer);
	_vm->_scenery->renderStatic(index, layer);
}

void Inter_v1::o1_loadCurLayer() {
	evalExpr(&_vm->_scenery->_curStatic);
	evalExpr(&_vm->_scenery->_curStaticLayer);
}

void Inter_v1::o1_playCDTrack() {
	evalExpr(0);
	if (_vm->_platform == Common::kPlatformMacintosh) {
		if (_vm->_adlib)
			_vm->_adlib->playTrack(_vm->_global->_inter_resStr);
	} else
		// Used in gob1 CD
		_vm->_cdrom->startTrack(_vm->_global->_inter_resStr);
}

void Inter_v1::o1_getCDTrackPos() {
	// Used in gob1 CD

	// Some scripts busy-wait while calling this opcode.
	// This is a very nasty thing to do, so let's add a
	// short delay here. It's probably a safe thing to do.

	_vm->_util->longDelay(1);

	int pos = _vm->_cdrom->getTrackPos();
	if (pos == -1)
		pos = 32767;
	WRITE_VAR(5, pos);
}

void Inter_v1::o1_stopCD() {
	if (_vm->_platform == Common::kPlatformMacintosh) {
		if (_vm->_adlib)
			_vm->_adlib->stopPlay();
	} else
		// Used in gob1 CD
		_vm->_cdrom->stopPlaying();
}

void Inter_v1::o1_loadFontToSprite() {
	int16 i = load16();
	_vm->_draw->_fontToSprite[i].sprite = load16();
	_vm->_draw->_fontToSprite[i].base = load16();
	_vm->_draw->_fontToSprite[i].width = load16();
	_vm->_draw->_fontToSprite[i].height = load16();
}

void Inter_v1::o1_freeFontToSprite() {
	int16 i = load16();
	_vm->_draw->_fontToSprite[i].sprite = -1;
	_vm->_draw->_fontToSprite[i].base = -1;
	_vm->_draw->_fontToSprite[i].width = -1;
	_vm->_draw->_fontToSprite[i].height = -1;
}

bool Inter_v1::o1_callSub(OpFuncParams &params) {
	byte *storedIP;
	uint16 offset;

	offset = load16();
	storedIP = _vm->_global->_inter_execPtr;

	debugC(5, kDebugGameFlow, "tot = \"%s\", offset = %d",
			_vm->_game->_curTotFile, offset);

	if (offset < 128) {
		warning("Inter_v1::o1_callSub(): Offset %d points into the header. "
				"Skipping call", offset);
		return false;
	}

	// Skipping the copy protection screen in Gobliiins
	if (!_vm->_copyProtection && (_vm->_features & GF_GOB1) && (offset == 3905)
			&& !scumm_stricmp(_vm->_game->_curTotFile, _vm->_startTot)) {
		debugC(2, kDebugGameFlow, "Skipping copy protection screen");
		return false;
	}
	// Skipping the copy protection screen in Gobliins 2
	if (!_vm->_copyProtection && (_vm->_features & GF_GOB2) && (offset == 1746)
			&& !scumm_stricmp(_vm->_game->_curTotFile, _vm->_startTot0)) {
		debugC(2, kDebugGameFlow, "Skipping copy protection screen");
		return false;
	}

	_vm->_global->_inter_execPtr = _vm->_game->_totFileData + offset;

	if ((params.counter == params.cmdCount) && (params.retFlag == 2))
		return true;

	callSub(2);
	_vm->_global->_inter_execPtr = storedIP;

	return false;
}

bool Inter_v1::o1_printTotText(OpFuncParams &params) {
	_vm->_draw->printTotText(load16());
	return false;
}

bool Inter_v1::o1_loadCursor(OpFuncParams &params) {
	Game::TotResItem *itemPtr;
	int16 width, height;
	byte *dataBuf;
	int32 offset;
	int16 id;
	int8 index;

	id = load16();
	index = (int8) *_vm->_global->_inter_execPtr++;
	itemPtr = &_vm->_game->_totResourceTable->items[id];
	offset = itemPtr->offset;

	if (offset < 0) {
		offset = (-offset - 1) * 4;
		dataBuf = _vm->_game->_imFileData +
			(int32) READ_LE_UINT32(_vm->_game->_imFileData + offset);
	} else
		dataBuf = _vm->_game->_totResourceTable->dataPtr + szGame_TotResTable +
			szGame_TotResItem * _vm->_game->_totResourceTable->itemsCount +
			offset;

	width = itemPtr->width;
	height = itemPtr->height;

	_vm->_video->fillRect(_vm->_draw->_cursorSprites,
			index * _vm->_draw->_cursorWidth, 0,
			index * _vm->_draw->_cursorWidth + _vm->_draw->_cursorWidth - 1,
			_vm->_draw->_cursorHeight - 1, 0);

	_vm->_video->drawPackedSprite(dataBuf, width, height,
			index * _vm->_draw->_cursorWidth, 0, 0, _vm->_draw->_cursorSprites);
	_vm->_draw->_cursorAnimLow[index] = 0;

	return false;
}

bool Inter_v1::o1_switch(OpFuncParams &params) {
	byte *callAddr;

	checkSwitchTable(&callAddr);
	byte *storedIP = _vm->_global->_inter_execPtr;
	_vm->_global->_inter_execPtr = callAddr;

	if ((params.counter == params.cmdCount) && (params.retFlag == 2))
		return true;

	funcBlock(0);
	_vm->_global->_inter_execPtr = storedIP;

	return false;
}

bool Inter_v1::o1_repeatUntil(OpFuncParams &params) {
	byte *blockPtr;
	int16 size;
	bool flag;

	_nestLevel[0]++;
	blockPtr = _vm->_global->_inter_execPtr;

	do {
		_vm->_global->_inter_execPtr = blockPtr;
		size = READ_LE_UINT16(_vm->_global->_inter_execPtr + 2) + 2;

		funcBlock(1);
		_vm->_global->_inter_execPtr = blockPtr + size + 1;
		flag = evalBoolResult();
	} while (!flag && !_break && !_terminate && !_vm->_quitRequested);

	_nestLevel[0]--;

	if (*_breakFromLevel > -1) {
		_break = false;
		*_breakFromLevel = -1;
	}
	return false;
}

bool Inter_v1::o1_whileDo(OpFuncParams &params) {
	byte *blockPtr;
	byte *savedIP;
	bool flag;
	int16 size;

	_nestLevel[0]++;
	do {
		savedIP = _vm->_global->_inter_execPtr;
		flag = evalBoolResult();

		if (_terminate)
			return false;

		blockPtr = _vm->_global->_inter_execPtr;

		size = READ_LE_UINT16(_vm->_global->_inter_execPtr + 2) + 2;

		if (flag) {
			funcBlock(1);
			_vm->_global->_inter_execPtr = savedIP;
		} else
			_vm->_global->_inter_execPtr += size;

		if (_break || _terminate || _vm->_quitRequested) {
			_vm->_global->_inter_execPtr = blockPtr;
			_vm->_global->_inter_execPtr += size;
			break;
		}
	} while (flag);

	_nestLevel[0]--;
	if (*_breakFromLevel > -1) {
		_break = false;
		*_breakFromLevel = -1;
	}
	return false;
}

bool Inter_v1::o1_if(OpFuncParams &params) {
	byte cmd;
	bool boolRes;
	byte *storedIP;
	
	boolRes = evalBoolResult();
	if (boolRes) {
		if ((params.counter == params.cmdCount) && (params.retFlag == 2))
			return true;

		storedIP = _vm->_global->_inter_execPtr;
		funcBlock(0);
		_vm->_global->_inter_execPtr = storedIP;

		_vm->_global->_inter_execPtr +=
			READ_LE_UINT16(_vm->_global->_inter_execPtr + 2) + 2;

		debugC(5, kDebugGameFlow, "cmd = %d",
				(int16) *_vm->_global->_inter_execPtr);

		cmd = *_vm->_global->_inter_execPtr >> 4;
		_vm->_global->_inter_execPtr++;
		if (cmd != 12)
			return false;

		_vm->_global->_inter_execPtr +=
			READ_LE_UINT16(_vm->_global->_inter_execPtr + 2) + 2;
	} else {
		_vm->_global->_inter_execPtr +=
			READ_LE_UINT16(_vm->_global->_inter_execPtr + 2) + 2;

		debugC(5, kDebugGameFlow, "cmd = %d",
				(int16) *_vm->_global->_inter_execPtr);

		cmd = *_vm->_global->_inter_execPtr >> 4;
		_vm->_global->_inter_execPtr++;
		if (cmd != 12)
			return false;

		if ((params.counter == params.cmdCount) && (params.retFlag == 2))
			return true;

		storedIP = _vm->_global->_inter_execPtr;
		funcBlock(0);
		_vm->_global->_inter_execPtr = storedIP;

		_vm->_global->_inter_execPtr +=
			READ_LE_UINT16(_vm->_global->_inter_execPtr + 2) + 2;
	}
	return false;
}

bool Inter_v1::o1_evaluateStore(OpFuncParams &params) {
	byte *savedPos;
	int16 token;
	int16 result;
	int16 varOff;

	savedPos = _vm->_global->_inter_execPtr;
	varOff = _vm->_parse->parseVarIndex();
	token = evalExpr(&result);
	switch (savedPos[0]) {
	case 23:
	case 26:
		WRITE_VAR_OFFSET(varOff, _vm->_global->_inter_resVal);
		break;

	case 25:
	case 28:
		if (token == 20)
			WRITE_VARO_UINT8(varOff, result);
		else
			WRITE_VARO_STR(varOff, _vm->_global->_inter_resStr);
		break;

	}
	return false;
}

bool Inter_v1::o1_loadSpriteToPos(OpFuncParams &params) {
	_vm->_draw->_spriteLeft = load16();

	_vm->_draw->_destSpriteX = _vm->_parse->parseValExpr();
	_vm->_draw->_destSpriteY = _vm->_parse->parseValExpr();

	_vm->_draw->_transparency = *_vm->_global->_inter_execPtr & 1;
	_vm->_draw->_destSurface = ((int16) (*_vm->_global->_inter_execPtr >> 1)) - 1;
	if (_vm->_draw->_destSurface < 0)
		_vm->_draw->_destSurface = 101;

	_vm->_global->_inter_execPtr += 2;

	_vm->_draw->spriteOperation(DRAW_LOADSPRITE);

	return false;
}

bool Inter_v1::o1_printText(OpFuncParams &params) {
	char buf[60];
	int i;

	_vm->_draw->_destSpriteX = _vm->_parse->parseValExpr();
	_vm->_draw->_destSpriteY = _vm->_parse->parseValExpr();

	_vm->_draw->_backColor = _vm->_parse->parseValExpr();
	_vm->_draw->_frontColor = _vm->_parse->parseValExpr();
	_vm->_draw->_fontIndex = _vm->_parse->parseValExpr();
	_vm->_draw->_destSurface = 21;
	_vm->_draw->_textToPrint = buf;
	_vm->_draw->_transparency = 0;

	if (_vm->_draw->_backColor >= 16) {
		_vm->_draw->_backColor = 0;
		_vm->_draw->_transparency = 1;
	}

	do {
		for (i = 0; (((char) *_vm->_global->_inter_execPtr) != '.') &&
				(*_vm->_global->_inter_execPtr != 200);
				i++, _vm->_global->_inter_execPtr++) {
			buf[i] = (char) *_vm->_global->_inter_execPtr;
		}

		if (*_vm->_global->_inter_execPtr != 200) {
			_vm->_global->_inter_execPtr++;
			switch (*_vm->_global->_inter_execPtr) {
			case 23:
			case 26:
				sprintf(buf + i, "%d",
					VAR_OFFSET(_vm->_parse->parseVarIndex()));
				break;

			case 25:
			case 28:
				sprintf(buf + i, "%s",
					GET_VARO_STR(_vm->_parse->parseVarIndex()));
				break;
			}
			_vm->_global->_inter_execPtr++;
		} else
			buf[i] = 0;

		_vm->_draw->spriteOperation(DRAW_PRINTTEXT);
	} while (*_vm->_global->_inter_execPtr != 200);

	_vm->_global->_inter_execPtr++;

	return false;
}

bool Inter_v1::o1_loadTot(OpFuncParams &params) {
	char buf[20];
	int8 size;

	if ((*_vm->_global->_inter_execPtr & 0x80) != 0) {
		_vm->_global->_inter_execPtr++;
		evalExpr(0);
		strncpy0(buf, _vm->_global->_inter_resStr, 15);
	} else {
		size = (int8) *_vm->_global->_inter_execPtr++;
		for (int i = 0; i < size; i++)
			buf[i] = *_vm->_global->_inter_execPtr++;

		buf[size] = 0;
	}

	strcat(buf, ".tot");
	if (_terminate != 2)
		_terminate = 1;
	strcpy(_vm->_game->_totToLoad, buf);

	return false;
}

bool Inter_v1::o1_palLoad(OpFuncParams &params) {
	int index1, index2;
	byte *palPtr;
	byte cmd;

	cmd = *_vm->_global->_inter_execPtr++;
	switch (cmd & 0x7F) {
	case 48:
		if ((_vm->_global->_fakeVideoMode < 0x32) ||
				(_vm->_global->_fakeVideoMode > 0x63)) {
			_vm->_global->_inter_execPtr += 48;
			return false;
		}
		break;

	case 49:
		if ((_vm->_global->_fakeVideoMode != 5) &&
				(_vm->_global->_fakeVideoMode != 7)) {
			_vm->_global->_inter_execPtr += 18;
			return false;
		}
		break;

	case 50:
		if (_vm->_global->_colorCount == 256) {
			_vm->_global->_inter_execPtr += 16;
			return false;
		}
		break;

	case 51:
		if (_vm->_global->_fakeVideoMode < 0x64) {
			_vm->_global->_inter_execPtr += 2;
			return false;
		}
		break;

	case 52:
		if (_vm->_global->_colorCount == 256) {
			_vm->_global->_inter_execPtr += 48;
			return false;
		}
		break;

	case 53:
		if (_vm->_global->_colorCount != 256) {
			_vm->_global->_inter_execPtr += 2;
			return false;
		}
		break;

	case 54:
		if (_vm->_global->_fakeVideoMode < 0x13) {
			return false;
		}
		break;

	case 61:
		if (_vm->_global->_fakeVideoMode < 0x13) {
			*_vm->_global->_inter_execPtr += 4;
			return false;
		}
		break;
	}

	if ((cmd & 0x7F) == 0x30) {
		_vm->_global->_inter_execPtr += 48;
		return false;
	}

	_vm->_draw->_applyPal = !(cmd & 0x80);
	cmd &= 0x7F;

	if (cmd == 49) {
		bool allZero = true;

		for (int i = 2; i < 18; i++) {
			if (_vm->_global->_inter_execPtr[i] != 0) {
				allZero = false;
				break;
			}
		}
		if (!allZero) {
			_vm->_video->clearSurf(_vm->_draw->_frontSurface);
			_vm->_draw->_noInvalidated57 = true;
			_vm->_global->_inter_execPtr += 18;
			return false;
		}
		_vm->_draw->_noInvalidated57 = false;

		for (int i = 0; i < 18; i++, _vm->_global->_inter_execPtr++) {
			if (i < 2) {
				if (!_vm->_draw->_applyPal)
					continue;

				_vm->_draw->_unusedPalette1[i] = *_vm->_global->_inter_execPtr;
				continue;
			}

			index1 = *_vm->_global->_inter_execPtr >> 4;
			index2 = (*_vm->_global->_inter_execPtr & 0xF);

			_vm->_draw->_unusedPalette1[i] =
				((_vm->_draw->_palLoadData1[index1] +
					_vm->_draw->_palLoadData2[index2]) << 8) +
				(_vm->_draw->_palLoadData2[index1] +
					_vm->_draw->_palLoadData1[index2]);
		}

		_vm->_global->_pPaletteDesc->unused1 = _vm->_draw->_unusedPalette1;
		_vm->_video->setFullPalette(_vm->_global->_pPaletteDesc);
		return false;
	}

	switch (cmd) {
	case 50:
		for (int i = 0; i < 16; i++, _vm->_global->_inter_execPtr++)
			_vm->_draw->_unusedPalette2[i] = *_vm->_global->_inter_execPtr;
		break;

	case 52:
		for (int i = 0; i < 16; i++, _vm->_global->_inter_execPtr += 3) {
			_vm->_draw->_vgaPalette[i].red = _vm->_global->_inter_execPtr[0];
			_vm->_draw->_vgaPalette[i].green = _vm->_global->_inter_execPtr[1];
			_vm->_draw->_vgaPalette[i].blue = _vm->_global->_inter_execPtr[2];
		}
		break;

	case 53:
		palPtr = _vm->_game->loadTotResource(_vm->_inter->load16());
		memcpy((char *) _vm->_draw->_vgaPalette, palPtr, 768);
		break;

	case 54:
		memset((char *) _vm->_draw->_vgaPalette, 0, 768);
		break;

	case 61:
		index1 = *_vm->_global->_inter_execPtr++;
		index2 = (*_vm->_global->_inter_execPtr++ - index1 + 1) * 3;
		palPtr = _vm->_game->loadTotResource(_vm->_inter->load16());
		memcpy((char *) _vm->_draw->_vgaPalette + index1 * 3,
				palPtr + index1 * 3, index2);

		if (_vm->_draw->_applyPal) {
			_vm->_draw->_applyPal = false;
			_vm->_video->setFullPalette(_vm->_global->_pPaletteDesc);
			return false;
		}
		break;
	}
	
	if (!_vm->_draw->_applyPal) {
		_vm->_global->_pPaletteDesc->unused2 = _vm->_draw->_unusedPalette2;
		_vm->_global->_pPaletteDesc->unused1 = _vm->_draw->_unusedPalette1;

		if (_vm->_global->_videoMode < 0x13) {
			_vm->_global->_pPaletteDesc->vgaPal = _vm->_draw->_vgaSmallPalette;
			_vm->_palAnim->fade(_vm->_global->_pPaletteDesc, 0, 0);
			return false;
		}
		if ((_vm->_global->_videoMode < 0x32) ||
				(_vm->_global->_videoMode >= 0x64)) {
			_vm->_global->_pPaletteDesc->vgaPal = _vm->_draw->_vgaPalette;
			_vm->_palAnim->fade(_vm->_global->_pPaletteDesc, 0, 0);
			return false;
		}
		_vm->_global->_pPaletteDesc->vgaPal = _vm->_draw->_vgaSmallPalette;
		_vm->_palAnim->fade(_vm->_global->_pPaletteDesc, 0, 0);
	}

	return false;
}

bool Inter_v1::o1_keyFunc(OpFuncParams &params) {
	static uint32 lastCalled = 0;
	int16 cmd;
	int16 key;
	uint32 now;

	cmd = load16();
	animPalette();
	_vm->_draw->blitInvalidated();

	now = _vm->_util->getTimeKey();
	if (!_noBusyWait)
		if ((now - lastCalled) <= 20)
			_vm->_util->longDelay(1);
	lastCalled = now;
	_noBusyWait = false;

	switch (cmd) {
	case 0:
		_vm->_draw->_showCursor &= ~2;
		_vm->_util->longDelay(1);
		key = _vm->_game->checkCollisions(0, 0, 0, 0);
		storeKey(key);

		_vm->_util->clearKeyBuf();
		break;

	case 1:
		_vm->_util->forceMouseUp(true);
		key = _vm->_game->checkKeys(&_vm->_global->_inter_mouseX,
				&_vm->_global->_inter_mouseY, &_vm->_game->_mouseButtons, 0);
		storeKey(key);
		break;

	case 2:
		_vm->_util->processInput(true);
		key = _vm->_util->checkKey();

		WRITE_VAR(0, key);
		_vm->_util->clearKeyBuf();
		break;

	default:
		_vm->_snd->speakerOnUpdate(cmd);
		if (cmd < 20) {
			_vm->_util->delay(cmd);
			_noBusyWait = true;
		} else
			_vm->_util->longDelay(cmd);
		break;
	}

	return false;
}

bool Inter_v1::o1_capturePush(OpFuncParams &params) {
	int16 left, top;
	int16 width, height;

	left = _vm->_parse->parseValExpr();
	top = _vm->_parse->parseValExpr();
	width = _vm->_parse->parseValExpr();
	height = _vm->_parse->parseValExpr();
	_vm->_game->capturePush(left, top, width, height);
	(*_vm->_scenery->_pCaptureCounter)++;
	return false;
}

bool Inter_v1::o1_capturePop(OpFuncParams &params) {
	if (*_vm->_scenery->_pCaptureCounter != 0) {
		(*_vm->_scenery->_pCaptureCounter)--;
		_vm->_game->capturePop(1);
	}
	return false;
}

bool Inter_v1::o1_animPalInit(OpFuncParams &params) {
	_animPalDir[0] = load16();
	_animPalLowIndex[0] = _vm->_parse->parseValExpr();
	_animPalHighIndex[0] = _vm->_parse->parseValExpr();
	return false;
}

bool Inter_v1::o1_drawOperations(OpFuncParams &params) {
	byte cmd;

	cmd = *_vm->_global->_inter_execPtr++;

	executeDrawOpcode(cmd);

	return false;
}

bool Inter_v1::o1_setcmdCount(OpFuncParams &params) {
	params.cmdCount = *_vm->_global->_inter_execPtr++;
	params.counter = 0;
	return false;
}

bool Inter_v1::o1_return(OpFuncParams &params) {
	if (params.retFlag != 2)
		_break = true;

	_vm->_global->_inter_execPtr = 0;
	return true;
}

bool Inter_v1::o1_renewTimeInVars(OpFuncParams &params) {
	renewTimeInVars();
	return false;
}

bool Inter_v1::o1_speakerOn(OpFuncParams &params) {
	_vm->_snd->speakerOn(_vm->_parse->parseValExpr(), -1);
	return false;
}

bool Inter_v1::o1_speakerOff(OpFuncParams &params) {
	_vm->_snd->speakerOff();
	return false;
}

bool Inter_v1::o1_putPixel(OpFuncParams &params) {
	_vm->_draw->_destSurface = load16();

	_vm->_draw->_destSpriteX = _vm->_parse->parseValExpr();
	_vm->_draw->_destSpriteY = _vm->_parse->parseValExpr();
	_vm->_draw->_frontColor = _vm->_parse->parseValExpr();
	_vm->_draw->spriteOperation(DRAW_PUTPIXEL);
	return false;
}

bool Inter_v1::o1_goblinFunc(OpFuncParams &params) {
	OpGobParams gobParams;
	bool objDescSet = false;
	int16 cmd;

	gobParams.extraData = 0;
	gobParams.objDesc = 0;
	gobParams.retVarPtr = (int32 *) VAR_ADDRESS(59);

	cmd = load16();
	_vm->_global->_inter_execPtr += 2;
	if ((cmd > 0) && (cmd < 17)) {
		objDescSet = true;
		gobParams.extraData = load16();
		gobParams.objDesc = _vm->_goblin->_objects[gobParams.extraData];
		gobParams.extraData = load16();
	}

	if ((cmd > 90) && (cmd < 107)) {
		objDescSet = true;
		gobParams.extraData = load16();
		gobParams.objDesc = _vm->_goblin->_goblins[gobParams.extraData];
		gobParams.extraData = load16();
		cmd -= 90;
	}

	if ((cmd > 110) && (cmd < 128)) {
		objDescSet = true;
		gobParams.extraData = load16();
		gobParams.objDesc = _vm->_goblin->_goblins[gobParams.extraData];
		cmd -= 90;
	} else if ((cmd > 20) && (cmd < 38)) {
		objDescSet = true;
		gobParams.extraData = load16();
		gobParams.objDesc = _vm->_goblin->_objects[gobParams.extraData];
	}

/*
	NB: The original gobliiins engine did not initialize the gobParams.objDesc
	variable, so we manually check if gobParams.objDesc is properly set before
	checking if it is zero. If it was not set, we do not return. This
	fixes a crash in the EGA version if the life bar is depleted, because
	interFunc is called multiple times with cmd == 39.
	Bug #1324814
*/

	if ((cmd < 40) && objDescSet && !gobParams.objDesc)
		return false;

	executeGoblinOpcode(cmd, gobParams);

	return false;
}

bool Inter_v1::o1_createSprite(OpFuncParams &params) {
	int16 index;
	int16 width, height;
	int16 flag;

	index = load16();
	width = load16();
	height = load16();

	flag = load16();
	_vm->_draw->initSpriteSurf(index, width, height, flag ? 2 : 0);

	return false;
}

bool Inter_v1::o1_freeSprite(OpFuncParams &params) {
	_vm->_draw->freeSprite(load16());
	return false;
}

bool Inter_v1::o1_returnTo(OpFuncParams &params) {
	if (params.retFlag == 1) {
		_break = true;
		_vm->_global->_inter_execPtr = 0;
		return true;
	}

	if (*_nestLevel == 0)
		return false;

	*_breakFromLevel = *_nestLevel;
	_break = true;
	_vm->_global->_inter_execPtr = 0;
	return true;
}

bool Inter_v1::o1_loadSpriteContent(OpFuncParams &params) {
	_vm->_draw->_spriteLeft = load16();
	_vm->_draw->_destSurface = load16();
	_vm->_draw->_transparency = load16();
	_vm->_draw->_destSpriteX = 0;
	_vm->_draw->_destSpriteY = 0;
	_vm->_draw->spriteOperation(DRAW_LOADSPRITE);
	return false;
}

bool Inter_v1::o1_copySprite(OpFuncParams &params) {
	_vm->_draw->_sourceSurface = load16();
	_vm->_draw->_destSurface = load16();

	_vm->_draw->_spriteLeft = _vm->_parse->parseValExpr();
	_vm->_draw->_spriteTop = _vm->_parse->parseValExpr();
	_vm->_draw->_spriteRight = _vm->_parse->parseValExpr();
	_vm->_draw->_spriteBottom = _vm->_parse->parseValExpr();

	_vm->_draw->_destSpriteX = _vm->_parse->parseValExpr();
	_vm->_draw->_destSpriteY = _vm->_parse->parseValExpr();

	_vm->_draw->_transparency = load16();
	_vm->_draw->spriteOperation(DRAW_BLITSURF);
	return false;
}

bool Inter_v1::o1_fillRect(OpFuncParams &params) {
	_vm->_draw->_destSurface = load16();

	_vm->_draw->_destSpriteX = _vm->_parse->parseValExpr();
	_vm->_draw->_destSpriteY = _vm->_parse->parseValExpr();
	_vm->_draw->_spriteRight = _vm->_parse->parseValExpr();
	_vm->_draw->_spriteBottom = _vm->_parse->parseValExpr();

	_vm->_draw->_backColor = _vm->_parse->parseValExpr();
	_vm->_draw->spriteOperation(DRAW_FILLRECT);
	return false;
}

bool Inter_v1::o1_drawLine(OpFuncParams &params) {
	_vm->_draw->_destSurface = load16();

	_vm->_draw->_destSpriteX = _vm->_parse->parseValExpr();
	_vm->_draw->_destSpriteY = _vm->_parse->parseValExpr();
	_vm->_draw->_spriteRight = _vm->_parse->parseValExpr();
	_vm->_draw->_spriteBottom = _vm->_parse->parseValExpr();

	_vm->_draw->_frontColor = _vm->_parse->parseValExpr();
	_vm->_draw->spriteOperation(DRAW_DRAWLINE);
	return false;
}

bool Inter_v1::o1_strToLong(OpFuncParams &params) {
	char str[20];
	int16 strVar;
	int16 destVar;
	int32 res;

	strVar = _vm->_parse->parseVarIndex();
	strncpy0(str, GET_VARO_STR(strVar), 19);
	res = atol(str);

	destVar = _vm->_parse->parseVarIndex();
	WRITE_VAR_OFFSET(destVar, res);
	return false;
}

bool Inter_v1::o1_invalidate(OpFuncParams &params) {
	_vm->_draw->_destSurface = load16();
	_vm->_draw->_destSpriteX = _vm->_parse->parseValExpr();
	_vm->_draw->_destSpriteY = _vm->_parse->parseValExpr();
	_vm->_draw->_spriteRight = _vm->_parse->parseValExpr();
	_vm->_draw->_frontColor = _vm->_parse->parseValExpr();
	_vm->_draw->spriteOperation(DRAW_INVALIDATE);
	return false;
}

bool Inter_v1::o1_setBackDelta(OpFuncParams &params) {
	_vm->_draw->_backDeltaX = _vm->_parse->parseValExpr();
	_vm->_draw->_backDeltaY = _vm->_parse->parseValExpr();
	return false;
}

bool Inter_v1::o1_playSound(OpFuncParams &params) {
	int16 frequency;
	int16 freq2;
	int16 repCount;
	int16 index;
	int16 endRep;

	index = _vm->_parse->parseValExpr();
	repCount = _vm->_parse->parseValExpr();
	frequency = _vm->_parse->parseValExpr();

	SoundDesc &sample = _vm->_game->_soundSamples[index];

	_soundEndTimeKey = 0;
	if (sample.empty())
		return false;

	if (repCount < 0) {
		if (_vm->_global->_soundFlags < 2)
			return false;

		repCount = -repCount;
		_soundEndTimeKey = _vm->_util->getTimeKey();

		freq2 = frequency ? frequency : sample._frequency;
		endRep = MAX(repCount - 1, 1);

		_soundStopVal = sample.calcFadeOutLength(freq2);
		_soundEndTimeKey += sample.calcLength(endRep, freq2, true);
	}

	if (sample.getType() == SOUND_ADL) {
		if (_vm->_adlib) {
			_vm->_adlib->load(sample.getData(), sample.size(), index);
			_vm->_adlib->setRepeating(repCount - 1);
			_vm->_adlib->startPlay();
		}
	} else {
		_vm->_snd->stopSound(0);
		_vm->_snd->playSample(sample, repCount - 1, frequency);
	}

	return false;
}

bool Inter_v1::o1_stopSound(OpFuncParams &params) {
	if (_vm->_adlib)
		_vm->_adlib->stopPlay();
	_vm->_snd->stopSound(_vm->_parse->parseValExpr());

	_soundEndTimeKey = 0;
	return false;
}

bool Inter_v1::o1_loadSound(OpFuncParams &params) {
	loadSound(-1);
	return false;
}

bool Inter_v1::o1_freeSoundSlot(OpFuncParams &params) {
	_vm->_game->freeSoundSlot(-1);
	return false;
}

bool Inter_v1::o1_waitEndPlay(OpFuncParams &params) {
	_vm->_snd->waitEndPlay();
	return false;
}

bool Inter_v1::o1_playComposition(OpFuncParams &params) {
	int16 composition[50];
	int16 dataVar;
	int16 freqVal;

	dataVar = _vm->_parse->parseVarIndex();
	freqVal = _vm->_parse->parseValExpr();
	for (int i = 0; i < 50; i++)
		composition[i] = (int16) VAR_OFFSET(dataVar + i * 4);

	_vm->_snd->playComposition(composition, freqVal);
	return false;
}

bool Inter_v1::o1_getFreeMem(OpFuncParams &params) {
	int16 freeVar;
	int16 maxFreeVar;

	freeVar = _vm->_parse->parseVarIndex();
	maxFreeVar = _vm->_parse->parseVarIndex();

	// HACK
	WRITE_VAR_OFFSET(freeVar, 1000000);
	WRITE_VAR_OFFSET(maxFreeVar, 1000000);
	return false;
}

bool Inter_v1::o1_checkData(OpFuncParams &params) {
	int16 handle;
	int16 varOff;

	evalExpr(0);
	varOff = _vm->_parse->parseVarIndex();
	handle = _vm->_dataIO->openData(_vm->_global->_inter_resStr);

	WRITE_VAR_OFFSET(varOff, handle);
	if (handle >= 0)
		_vm->_dataIO->closeData(handle);
	else
		warning("File \"%s\" not found", _vm->_global->_inter_resStr);
	return false;
}

bool Inter_v1::o1_prepareStr(OpFuncParams &params) {
	int16 strVar;

	strVar = _vm->_parse->parseVarIndex();
	_vm->_util->prepareStr(GET_VARO_STR(strVar));
	_vm->_global->writeVarSizeStr(strVar, strlen(GET_VARO_STR(strVar)));
	return false;
}

bool Inter_v1::o1_insertStr(OpFuncParams &params) {
	int16 pos;
	int16 strVar;

	strVar = _vm->_parse->parseVarIndex();
	evalExpr(0);
	pos = _vm->_parse->parseValExpr();
	_vm->_util->insertStr(_vm->_global->_inter_resStr, GET_VARO_STR(strVar), pos);
	_vm->_global->writeVarSizeStr(strVar, strlen(GET_VARO_STR(strVar)));
	return false;
}

bool Inter_v1::o1_cutStr(OpFuncParams &params) {
	int16 strVar;
	int16 pos;
	int16 size;

	strVar = _vm->_parse->parseVarIndex();
	pos = _vm->_parse->parseValExpr();
	size = _vm->_parse->parseValExpr();
	_vm->_util->cutFromStr(GET_VARO_STR(strVar), pos, size);
	return false;
}

bool Inter_v1::o1_strstr(OpFuncParams &params) {
	int16 strVar;
	int16 resVar;
	int16 pos;

	strVar = _vm->_parse->parseVarIndex();
	evalExpr(0);
	resVar = _vm->_parse->parseVarIndex();

	char *res = strstr(GET_VARO_STR(strVar), _vm->_global->_inter_resStr);
	pos = res ? (res - (GET_VARO_STR(strVar))) : -1;
	WRITE_VAR_OFFSET(resVar, pos);
	return false;
}

bool Inter_v1::o1_istrlen(OpFuncParams &params) {
	int16 len;
	int16 strVar;

	strVar = _vm->_parse->parseVarIndex();
	len = strlen(GET_VARO_STR(strVar));
	strVar = _vm->_parse->parseVarIndex();

	WRITE_VAR_OFFSET(strVar, len);
	return false;
}

bool Inter_v1::o1_setMousePos(OpFuncParams &params) {
	_vm->_global->_inter_mouseX = _vm->_parse->parseValExpr();
	_vm->_global->_inter_mouseY = _vm->_parse->parseValExpr();
	_vm->_global->_inter_mouseX -= _vm->_video->_scrollOffsetX;
	_vm->_global->_inter_mouseY -= _vm->_video->_scrollOffsetY;
	if (_vm->_global->_useMouse != 0)
		_vm->_util->setMousePos(_vm->_global->_inter_mouseX,
				_vm->_global->_inter_mouseY);
	return false;
}

bool Inter_v1::o1_setFrameRate(OpFuncParams &params) {
	_vm->_util->setFrameRate(_vm->_parse->parseValExpr());
	return false;
}

bool Inter_v1::o1_animatePalette(OpFuncParams &params) {
	_vm->_draw->blitInvalidated();
	_vm->_util->waitEndFrame();
	animPalette();
	storeKey(_vm->_game->checkKeys(&_vm->_global->_inter_mouseX,
		&_vm->_global->_inter_mouseY, &_vm->_game->_mouseButtons, 0));
	return false;
}

bool Inter_v1::o1_animateCursor(OpFuncParams &params) {
	_vm->_draw->animateCursor(1);
	return false;
}

bool Inter_v1::o1_blitCursor(OpFuncParams &params) {
	_vm->_draw->blitCursor();
	return false;
}

bool Inter_v1::o1_loadFont(OpFuncParams &params) {
	int16 index;

	evalExpr(0);
	index = load16();

	if (_vm->_draw->_fonts[index])
		_vm->_util->freeFont(_vm->_draw->_fonts[index]);

	_vm->_draw->animateCursor(4);
	if (_vm->_game->_extHandle >= 0)
		_vm->_dataIO->closeData(_vm->_game->_extHandle);

	_vm->_draw->_fonts[index] =
		_vm->_util->loadFont(_vm->_global->_inter_resStr);

	if (_vm->_game->_extHandle >= 0)
		_vm->_game->_extHandle = _vm->_dataIO->openData(_vm->_game->_curExtFile);
	return false;
}

bool Inter_v1::o1_freeFont(OpFuncParams &params) {
	int16 index;

	index = load16();
	if (_vm->_draw->_fonts[index])
		_vm->_util->freeFont(_vm->_draw->_fonts[index]);

	_vm->_draw->_fonts[index] = 0;
	return false;
}

bool Inter_v1::o1_readData(OpFuncParams &params) {
	int16 retSize;
	int16 size;
	int16 dataVar;
	int16 offset;
	int16 handle;

	evalExpr(0);
	dataVar = _vm->_parse->parseVarIndex();
	size = _vm->_parse->parseValExpr();
	offset = _vm->_parse->parseValExpr();
	retSize = 0;

	if (_vm->_game->_extHandle >= 0)
		_vm->_dataIO->closeData(_vm->_game->_extHandle);

	WRITE_VAR(1, 1);
	handle = _vm->_dataIO->openData(_vm->_global->_inter_resStr);
	if (handle >= 0) {
		_vm->_draw->animateCursor(4);
		if (offset < 0)
			_vm->_dataIO->seekData(handle, -offset - 1, SEEK_END);
		else
			_vm->_dataIO->seekData(handle, offset, SEEK_SET);

		if (((dataVar >> 2) == 59) && (size == 4))
			WRITE_VAR(59, _vm->_dataIO->readUint32(handle));
		else
			retSize = _vm->_dataIO->readData(handle,
					_vm->_global->_inter_variables + dataVar, size);

		_vm->_dataIO->closeData(handle);

		if (retSize == size)
			WRITE_VAR(1, 0);
	}

	if (_vm->_game->_extHandle >= 0)
		_vm->_game->_extHandle = _vm->_dataIO->openData(_vm->_game->_curExtFile);
	return false;
}

bool Inter_v1::o1_writeData(OpFuncParams &params) {
	int16 offset;
	int16 size;
	int16 dataVar;

	// This writes into a file. It's not portable and isn't needed anyway
	// (Gobliiins 1 doesn't use save file), so we just warn should it be
	// called regardless.

	evalExpr(0);
	dataVar = _vm->_parse->parseVarIndex();
	size = _vm->_parse->parseValExpr();
	offset = _vm->_parse->parseValExpr();

	warning("Attempted to write to file \"%s\"", _vm->_global->_inter_resStr);
	WRITE_VAR(1, 0);

	return false;
}

bool Inter_v1::o1_manageDataFile(OpFuncParams &params) {
	evalExpr(0);

	if (_vm->_global->_inter_resStr[0] != 0)
		_vm->_dataIO->openDataFile(_vm->_global->_inter_resStr);
	else
		_vm->_dataIO->closeDataFile();
	return false;
}

void Inter_v1::o1_setState(OpGobParams &params) {
	params.objDesc->state = params.extraData;
	if (params.objDesc == _vm->_goblin->_actDestItemDesc)
		*_vm->_goblin->_destItemStateVarPtr = params.extraData;
}

void Inter_v1::o1_setCurFrame(OpGobParams &params) {
	params.objDesc->curFrame = params.extraData;
	if (params.objDesc == _vm->_goblin->_actDestItemDesc)
		*_vm->_goblin->_destItemFrameVarPtr = params.extraData;
}

void Inter_v1::o1_setNextState(OpGobParams &params) {
	params.objDesc->nextState = params.extraData;
	if (params.objDesc == _vm->_goblin->_actDestItemDesc)
		*_vm->_goblin->_destItemNextStateVarPtr = params.extraData;
}

void Inter_v1::o1_setMultState(OpGobParams &params) {
	params.objDesc->multState = params.extraData;
	if (params.objDesc == _vm->_goblin->_actDestItemDesc)
		*_vm->_goblin->_destItemMultStateVarPtr = params.extraData;
}

void Inter_v1::o1_setOrder(OpGobParams &params) {
	params.objDesc->order = params.extraData;
	if (params.objDesc == _vm->_goblin->_actDestItemDesc)
		*_vm->_goblin->_destItemOrderVarPtr = params.extraData;
}

void Inter_v1::o1_setActionStartState(OpGobParams &params) {
	params.objDesc->actionStartState = params.extraData;
	if (params.objDesc == _vm->_goblin->_actDestItemDesc)
		*_vm->_goblin->_destItemActStartStVarPtr = params.extraData;
}

void Inter_v1::o1_setCurLookDir(OpGobParams &params) {
	params.objDesc->curLookDir = params.extraData;
	if (params.objDesc == _vm->_goblin->_actDestItemDesc)
		*_vm->_goblin->_destItemLookDirVarPtr = params.extraData;
}

void Inter_v1::o1_setType(OpGobParams &params) {
	params.objDesc->type = params.extraData;
	if (params.objDesc == _vm->_goblin->_actDestItemDesc)
		*_vm->_goblin->_destItemTypeVarPtr = params.extraData;

	if (params.extraData == 0)
		params.objDesc->toRedraw = 1;
}

void Inter_v1::o1_setNoTick(OpGobParams &params) {
	params.objDesc->noTick = params.extraData;
	if (params.objDesc == _vm->_goblin->_actDestItemDesc)
		*_vm->_goblin->_destItemNoTickVarPtr = params.extraData;
}

void Inter_v1::o1_setPickable(OpGobParams &params) {
	params.objDesc->pickable = params.extraData;
	if (params.objDesc == _vm->_goblin->_actDestItemDesc)
		*_vm->_goblin->_destItemPickableVarPtr = params.extraData;
}

void Inter_v1::o1_setXPos(OpGobParams &params) {
	params.objDesc->xPos = params.extraData;
	if (params.objDesc == _vm->_goblin->_actDestItemDesc)
		*_vm->_goblin->_destItemScrXVarPtr = params.extraData;
}

void Inter_v1::o1_setYPos(OpGobParams &params) {
	params.objDesc->yPos = params.extraData;
	if (params.objDesc == _vm->_goblin->_actDestItemDesc)
		*_vm->_goblin->_destItemScrYVarPtr = params.extraData;
}

void Inter_v1::o1_setDoAnim(OpGobParams &params) {
	params.objDesc->doAnim = params.extraData;
	if (params.objDesc == _vm->_goblin->_actDestItemDesc)
		*_vm->_goblin->_destItemDoAnimVarPtr = params.extraData;
}

void Inter_v1::o1_setRelaxTime(OpGobParams &params) {
	params.objDesc->relaxTime = params.extraData;
	if (params.objDesc == _vm->_goblin->_actDestItemDesc)
		*_vm->_goblin->_destItemRelaxVarPtr = params.extraData;
}

void Inter_v1::o1_setMaxTick(OpGobParams &params) {
	params.objDesc->maxTick = params.extraData;
	if (params.objDesc == _vm->_goblin->_actDestItemDesc)
		*_vm->_goblin->_destItemMaxTickVarPtr = params.extraData;
}

void Inter_v1::o1_getState(OpGobParams &params) {
	*params.retVarPtr = params.objDesc->state;
}

void Inter_v1::o1_getCurFrame(OpGobParams &params) {
	*params.retVarPtr = params.objDesc->curFrame;
}

void Inter_v1::o1_getNextState(OpGobParams &params) {
	*params.retVarPtr = params.objDesc->nextState;
}

void Inter_v1::o1_getMultState(OpGobParams &params) {
	*params.retVarPtr = params.objDesc->multState;
}

void Inter_v1::o1_getOrder(OpGobParams &params) {
	*params.retVarPtr = params.objDesc->order;
}

void Inter_v1::o1_getActionStartState(OpGobParams &params) {
	*params.retVarPtr = params.objDesc->actionStartState;
}

void Inter_v1::o1_getCurLookDir(OpGobParams &params) {
	*params.retVarPtr = params.objDesc->curLookDir;
}

void Inter_v1::o1_getType(OpGobParams &params) {
	*params.retVarPtr = params.objDesc->type;
}

void Inter_v1::o1_getNoTick(OpGobParams &params) {
	*params.retVarPtr = params.objDesc->noTick;
}

void Inter_v1::o1_getPickable(OpGobParams &params) {
	*params.retVarPtr = params.objDesc->pickable;
}

void Inter_v1::o1_getObjMaxFrame(OpGobParams &params) {
	*params.retVarPtr = _vm->_goblin->getObjMaxFrame(params.objDesc);
}

void Inter_v1::o1_getXPos(OpGobParams &params) {
	*params.retVarPtr = params.objDesc->xPos;
}

void Inter_v1::o1_getYPos(OpGobParams &params) {
	*params.retVarPtr = params.objDesc->yPos;
}

void Inter_v1::o1_getDoAnim(OpGobParams &params) {
	*params.retVarPtr = params.objDesc->doAnim;
}

void Inter_v1::o1_getRelaxTime(OpGobParams &params) {
	*params.retVarPtr = params.objDesc->relaxTime;
}

void Inter_v1::o1_getMaxTick(OpGobParams &params) {
	*params.retVarPtr = params.objDesc->maxTick;
}

void Inter_v1::o1_manipulateMap(OpGobParams &params) {
	int16 xPos = load16();
	int16 yPos = load16();
	int16 item = load16();

	manipulateMap(xPos, yPos, item);
}

void Inter_v1::o1_getItem(OpGobParams &params) {
	int16 xPos = load16();
	int16 yPos = load16();

	if ((_vm->_map->_itemsMap[yPos][xPos] & 0xFF00) != 0)
		*params.retVarPtr = (_vm->_map->_itemsMap[yPos][xPos] & 0xFF00) >> 8;
	else
		*params.retVarPtr = _vm->_map->_itemsMap[yPos][xPos];
}

void Inter_v1::o1_manipulateMapIndirect(OpGobParams &params) {
	int16 xPos = load16();
	int16 yPos = load16();
	int16 item = load16();

	xPos = VAR(xPos);
	yPos = VAR(yPos);
	item = VAR(item);

	manipulateMap(xPos, yPos, item);
}

void Inter_v1::o1_getItemIndirect(OpGobParams &params) {
	int16 xPos = load16();
	int16 yPos = load16();

	xPos = VAR(xPos);
	yPos = VAR(yPos);

	if ((_vm->_map->_itemsMap[yPos][xPos] & 0xFF00) != 0)
		*params.retVarPtr = (_vm->_map->_itemsMap[yPos][xPos] & 0xFF00) >> 8;
	else
		*params.retVarPtr = _vm->_map->_itemsMap[yPos][xPos];
}

void Inter_v1::o1_setPassMap(OpGobParams &params) {
	int16 xPos = load16();
	int16 yPos = load16();
	int16 val = load16();
	_vm->_map->setPass(xPos, yPos, val);
}

void Inter_v1::o1_setGoblinPosH(OpGobParams &params) {
	int16 layer;
	int16 item = load16();
	int16 xPos = load16();
	int16 yPos = load16();

	_vm->_goblin->_gobPositions[item].x = xPos * 2;
	_vm->_goblin->_gobPositions[item].y = yPos * 2;

	params.objDesc = _vm->_goblin->_goblins[item];
	params.objDesc->nextState = 21;

	_vm->_goblin->nextLayer(params.objDesc);

	layer = params.objDesc->stateMach[params.objDesc->state][0]->layer;

	_vm->_scenery->updateAnim(layer, 0, params.objDesc->animation, 0,
			params.objDesc->xPos, params.objDesc->yPos, 0);

	params.objDesc->yPos = (_vm->_goblin->_gobPositions[item].y * 6 + 6) -
		(_vm->_scenery->_toRedrawBottom - _vm->_scenery->_animTop);
	params.objDesc->xPos =
			_vm->_goblin->_gobPositions[item].x * 12 -
			(_vm->_scenery->_toRedrawLeft - _vm->_scenery->_animLeft);

	params.objDesc->curFrame = 0;
	params.objDesc->state = 21;
	if (_vm->_goblin->_currentGoblin == item) {
		*_vm->_goblin->_curGobScrXVarPtr = params.objDesc->xPos;
		*_vm->_goblin->_curGobScrYVarPtr = params.objDesc->yPos;

		*_vm->_goblin->_curGobFrameVarPtr = 0;
		*_vm->_goblin->_curGobStateVarPtr = 18;
		_vm->_goblin->_pressedMapX = _vm->_goblin->_gobPositions[item].x;
		_vm->_goblin->_pressedMapY = _vm->_goblin->_gobPositions[item].y;
	}
}

void Inter_v1::o1_getGoblinPosXH(OpGobParams &params) {
	int16 item = load16();
	*params.retVarPtr = _vm->_goblin->_gobPositions[item].x >> 1;
}

void Inter_v1::o1_getGoblinPosYH(OpGobParams &params) {
	int16 item = load16();
	*params.retVarPtr = _vm->_goblin->_gobPositions[item].y >> 1;
}

void Inter_v1::o1_setGoblinMultState(OpGobParams &params) {
	int16 layer;
	int16 item = load16();
	int16 xPos = load16();
	int16 yPos = load16();

	params.objDesc = _vm->_goblin->_goblins[item];
	if (yPos == 0) {
		params.objDesc->multState = xPos;
		params.objDesc->nextState = xPos;
		_vm->_goblin->nextLayer(params.objDesc);

		layer = params.objDesc->stateMach[params.objDesc->state][0]->layer;

		Scenery::AnimLayer *animLayer =
			_vm->_scenery->getAnimLayer(params.objDesc->animation, layer);
		params.objDesc->xPos = animLayer->posX;
		params.objDesc->yPos = animLayer->posY;

		*_vm->_goblin->_curGobScrXVarPtr = params.objDesc->xPos;
		*_vm->_goblin->_curGobScrYVarPtr = params.objDesc->yPos;
		*_vm->_goblin->_curGobFrameVarPtr = 0;
		*_vm->_goblin->_curGobStateVarPtr = params.objDesc->state;
		*_vm->_goblin->_curGobNextStateVarPtr = params.objDesc->nextState;
		*_vm->_goblin->_curGobMultStateVarPtr = params.objDesc->multState;
		*_vm->_goblin->_curGobMaxFrameVarPtr =
				_vm->_goblin->getObjMaxFrame(params.objDesc);
		_vm->_goblin->_noPick = 1;
		return;
	}

	params.objDesc->multState = 21;
	params.objDesc->nextState = 21;
	params.objDesc->state = 21;
	_vm->_goblin->nextLayer(params.objDesc);
	layer = params.objDesc->stateMach[params.objDesc->state][0]->layer;

	_vm->_scenery->updateAnim(layer, 0, params.objDesc->animation, 0,
			params.objDesc->xPos, params.objDesc->yPos, 0);

	params.objDesc->yPos = (yPos * 6 + 6) -
		(_vm->_scenery->_toRedrawBottom - _vm->_scenery->_animTop);
	params.objDesc->xPos = xPos * 12 -
		(_vm->_scenery->_toRedrawLeft - _vm->_scenery->_animLeft);

	_vm->_goblin->_gobPositions[item].x = xPos;
	_vm->_goblin->_pressedMapX = xPos;
	_vm->_map->_curGoblinX = xPos;

	_vm->_goblin->_gobPositions[item].y = yPos;
	_vm->_goblin->_pressedMapY = yPos;
	_vm->_map->_curGoblinY = yPos;

	*_vm->_goblin->_curGobScrXVarPtr = params.objDesc->xPos;
	*_vm->_goblin->_curGobScrYVarPtr = params.objDesc->yPos;
	*_vm->_goblin->_curGobFrameVarPtr = 0;
	*_vm->_goblin->_curGobStateVarPtr = 21;
	*_vm->_goblin->_curGobNextStateVarPtr = 21;
	*_vm->_goblin->_curGobMultStateVarPtr = -1;
	_vm->_goblin->_noPick = 0;
}

void Inter_v1::o1_setGoblinUnk14(OpGobParams &params) {
	int16 item = load16();
	int16 val = load16();
	params.objDesc = _vm->_goblin->_objects[item];
	params.objDesc->unk14 = val;
}

void Inter_v1::o1_setItemIdInPocket(OpGobParams &params) {
	_vm->_goblin->_itemIdInPocket = load16();
}

void Inter_v1::o1_setItemIndInPocket(OpGobParams &params) {
	_vm->_goblin->_itemIndInPocket = load16();
}

void Inter_v1::o1_getItemIdInPocket(OpGobParams &params) {
	*params.retVarPtr = _vm->_goblin->_itemIdInPocket;
}

void Inter_v1::o1_getItemIndInPocket(OpGobParams &params) {
	*params.retVarPtr = _vm->_goblin->_itemIndInPocket;
}

void Inter_v1::o1_setGoblinPos(OpGobParams &params) {
	int16 layer;
	int16 item = load16();
	int16 xPos = load16();
	int16 yPos = load16();

	_vm->_goblin->_gobPositions[item].x = xPos;
	_vm->_goblin->_gobPositions[item].y = yPos;

	params.objDesc = _vm->_goblin->_goblins[item];
	params.objDesc->nextState = 21;
	_vm->_goblin->nextLayer(params.objDesc);

	layer = params.objDesc->stateMach[params.objDesc->state][0]->layer;

	_vm->_scenery->updateAnim(layer, 0, params.objDesc->animation, 0,
			params.objDesc->xPos, params.objDesc->yPos, 0);

	params.objDesc->yPos = (yPos * 6 + 6) -
		(_vm->_scenery->_toRedrawBottom - _vm->_scenery->_animTop);
	params.objDesc->xPos = xPos * 12 -
		(_vm->_scenery->_toRedrawLeft - _vm->_scenery->_animLeft);

	params.objDesc->curFrame = 0;
	params.objDesc->state = 21;

	if (_vm->_goblin->_currentGoblin == item) {
		*_vm->_goblin->_curGobScrXVarPtr = params.objDesc->xPos;
		*_vm->_goblin->_curGobScrYVarPtr = params.objDesc->yPos;
		*_vm->_goblin->_curGobFrameVarPtr = 0;
		*_vm->_goblin->_curGobStateVarPtr = 18;

		_vm->_goblin->_pressedMapX = _vm->_goblin->_gobPositions[item].x;
		_vm->_goblin->_pressedMapY = _vm->_goblin->_gobPositions[item].y;
	}
}

void Inter_v1::o1_setGoblinState(OpGobParams &params) {
	int16 layer;
	int16 item = load16();
	int16 state = load16();

	params.objDesc = _vm->_goblin->_goblins[item];
	params.objDesc->nextState = state;

	_vm->_goblin->nextLayer(params.objDesc);
	layer = params.objDesc->stateMach[params.objDesc->state][0]->layer;

	Scenery::AnimLayer *animLayer =
		_vm->_scenery->getAnimLayer(params.objDesc->animation, layer);
	params.objDesc->xPos = animLayer->posX;
	params.objDesc->yPos = animLayer->posY;

	if (item == _vm->_goblin->_currentGoblin) {
		*_vm->_goblin->_curGobScrXVarPtr = params.objDesc->xPos;
		*_vm->_goblin->_curGobScrYVarPtr = params.objDesc->yPos;
		*_vm->_goblin->_curGobFrameVarPtr = 0;
		*_vm->_goblin->_curGobStateVarPtr = params.objDesc->state;
		*_vm->_goblin->_curGobMultStateVarPtr = params.objDesc->multState;
	}
}

void Inter_v1::o1_setGoblinStateRedraw(OpGobParams &params) {
	int16 layer;
	int16 item = load16();
	int16 state = load16();
	params.objDesc = _vm->_goblin->_objects[item];

	params.objDesc->nextState = state;

	_vm->_goblin->nextLayer(params.objDesc);
	layer = params.objDesc->stateMach[params.objDesc->state][0]->layer;

	Scenery::AnimLayer *animLayer =
		_vm->_scenery->getAnimLayer(params.objDesc->animation, layer);
	params.objDesc->xPos = animLayer->posX;
	params.objDesc->yPos = animLayer->posY;

	params.objDesc->toRedraw = 1;
	params.objDesc->type = 0;
	if (params.objDesc == _vm->_goblin->_actDestItemDesc) {
		*_vm->_goblin->_destItemScrXVarPtr = params.objDesc->xPos;
		*_vm->_goblin->_destItemScrYVarPtr = params.objDesc->yPos;

		*_vm->_goblin->_destItemStateVarPtr = params.objDesc->state;
		*_vm->_goblin->_destItemNextStateVarPtr = -1;
		*_vm->_goblin->_destItemMultStateVarPtr = -1;
		*_vm->_goblin->_destItemFrameVarPtr = 0;
	}
}

void Inter_v1::o1_decRelaxTime(OpGobParams &params) {
	params.extraData = load16();
	params.objDesc = _vm->_goblin->_objects[params.extraData];

	params.objDesc->relaxTime--;
	if ((params.objDesc->relaxTime < 0) &&
	    (_vm->_goblin->getObjMaxFrame(params.objDesc) ==
	     params.objDesc->curFrame)) {
		params.objDesc->relaxTime = _vm->_util->getRandom(100) + 50;
		params.objDesc->curFrame = 0;
		params.objDesc->toRedraw = 1;
	}
}

void Inter_v1::o1_getGoblinPosX(OpGobParams &params) {
	int16 item = load16();
	*params.retVarPtr = _vm->_goblin->_gobPositions[item].x;
}

void Inter_v1::o1_getGoblinPosY(OpGobParams &params) {
	int16 item = load16();
	*params.retVarPtr = _vm->_goblin->_gobPositions[item].y;
}

void Inter_v1::o1_clearPathExistence(OpGobParams &params) {
	_vm->_goblin->_pathExistence = 0;
}

void Inter_v1::o1_setGoblinVisible(OpGobParams &params) {
	params.extraData = load16();
	_vm->_goblin->_goblins[params.extraData]->visible = 1;
}

void Inter_v1::o1_setGoblinInvisible(OpGobParams &params) {
	params.extraData = load16();
	_vm->_goblin->_goblins[params.extraData]->visible = 0;
}

void Inter_v1::o1_getObjectIntersect(OpGobParams &params) {
	params.extraData = load16();
	int16 item = load16();

	params.objDesc = _vm->_goblin->_objects[params.extraData];
	if (_vm->_goblin->objIntersected(params.objDesc,
				_vm->_goblin->_goblins[item]))
		*params.retVarPtr = 1;
	else
		*params.retVarPtr = 0;
}

void Inter_v1::o1_getGoblinIntersect(OpGobParams &params) {
	params.extraData = load16();
	int16 item = load16();

	params.objDesc = _vm->_goblin->_goblins[params.extraData];
	if (_vm->_goblin->objIntersected(params.objDesc,
				_vm->_goblin->_goblins[item]))
		*params.retVarPtr = 1;
	else
		*params.retVarPtr = 0;
}

void Inter_v1::o1_setItemPos(OpGobParams &params) {
	int16 item = load16();
	int16 xPos = load16();
	int16 yPos = load16();
	int16 val = load16();

	_vm->_map->_itemPoses[item].x = xPos;
	_vm->_map->_itemPoses[item].y = yPos;
	_vm->_map->_itemPoses[item].orient = val;
}

void Inter_v1::o1_loadObjects(OpGobParams &params) {
	params.extraData = load16();
	if (_vm->_game->_extHandle >= 0)
		_vm->_dataIO->closeData(_vm->_game->_extHandle);

	_vm->_goblin->loadObjects((char *) VAR_ADDRESS(params.extraData));
	_vm->_game->_extHandle = _vm->_dataIO->openData(_vm->_game->_curExtFile);
}

void Inter_v1::o1_freeObjects(OpGobParams &params) {
	_vm->_goblin->freeAllObjects();
}

void Inter_v1::o1_animateObjects(OpGobParams &params) {
	_vm->_goblin->animateObjects();
}

void Inter_v1::o1_drawObjects(OpGobParams &params) {
	_vm->_goblin->drawObjects();

	if (_vm->_platform == Common::kPlatformMacintosh) {
		if (_vm->_adlib)
			_vm->_adlib->playBgMusic();
	} else if (_vm->_cdrom->getTrackPos() == -1)
		_vm->_cdrom->playBgMusic();
}

void Inter_v1::o1_loadMap(OpGobParams &params) {
	_vm->_map->loadMapsInitGobs();
}

void Inter_v1::o1_moveGoblin(OpGobParams &params) {
	int16 item;
	params.extraData = load16();
	int16 xPos = load16();

	if ((uint16) VAR(xPos) == 0) {
		item =
				_vm->_goblin->doMove(_vm->_goblin->_goblins[_vm->_goblin->_currentGoblin],
					1, (uint16) VAR(params.extraData));
	} else {
		item =
				_vm->_goblin->doMove(_vm->_goblin->_goblins[_vm->_goblin->_currentGoblin],
					1, 3);
	}

	if (item != 0)
		_vm->_goblin->switchGoblin(item);
}

void Inter_v1::o1_switchGoblin(OpGobParams &params) {
	_vm->_goblin->switchGoblin(0);
}

void Inter_v1::o1_loadGoblin(OpGobParams &params) {
	_vm->_goblin->loadGobDataFromVars();
}

void Inter_v1::o1_writeTreatItem(OpGobParams &params) {
	params.extraData = load16();
	int16 cmd = load16();
	int16 xPos = load16();

	if ((uint16) VAR(xPos) == 0) {
		WRITE_VAR(cmd, _vm->_goblin->treatItem((uint16) VAR(params.extraData)));
		return;
	}

	WRITE_VAR(cmd, _vm->_goblin->treatItem(3));
}

void Inter_v1::o1_moveGoblin0(OpGobParams &params) {
	_vm->_goblin->doMove(_vm->_goblin->_goblins[_vm->_goblin->_currentGoblin],
			0, 0);
}

void Inter_v1::o1_setGoblinTarget(OpGobParams &params) {
	params.extraData = load16();
	if (VAR(params.extraData) != 0)
		_vm->_goblin->_goesAtTarget = 1;
	else
		_vm->_goblin->_goesAtTarget = 0;
}

void Inter_v1::o1_setGoblinObjectsPos(OpGobParams &params) {
	params.extraData = load16();
	params.extraData = VAR(params.extraData);
	_vm->_goblin->_objects[10]->xPos = params.extraData;

	params.extraData = load16();
	params.extraData = VAR(params.extraData);
	_vm->_goblin->_objects[10]->yPos = params.extraData;
}

void Inter_v1::o1_initGoblin(OpGobParams &params) {
	Goblin::Gob_Object *gobDesc = _vm->_goblin->_goblins[0];
	int16 xPos;
	int16 yPos;
	int16 layer;

	if (_vm->_goblin->_currentGoblin != 0) {
		_vm->_goblin->_goblins[_vm->_goblin->_currentGoblin]->doAnim = 1;
		_vm->_goblin->_goblins[_vm->_goblin->_currentGoblin]->nextState = 21;

		_vm->_goblin->nextLayer(_vm->_goblin->_goblins[_vm->_goblin->_currentGoblin]);
		_vm->_goblin->_currentGoblin = 0;

		gobDesc->doAnim = 0;
		gobDesc->type = 0;
		gobDesc->toRedraw = 1;

		_vm->_goblin->_pressedMapX = _vm->_goblin->_gobPositions[0].x;
		_vm->_map->_destX = _vm->_goblin->_gobPositions[0].x;
		_vm->_goblin->_gobDestX = _vm->_goblin->_gobPositions[0].x;

		_vm->_goblin->_pressedMapY = _vm->_goblin->_gobPositions[0].y;
		_vm->_map->_destY = _vm->_goblin->_gobPositions[0].y;
		_vm->_goblin->_gobDestY = _vm->_goblin->_gobPositions[0].y;

		*_vm->_goblin->_curGobVarPtr = 0;
		_vm->_goblin->_pathExistence = 0;
		_vm->_goblin->_readyToAct = 0;
	}

	if ((gobDesc->state != 10) && (_vm->_goblin->_itemIndInPocket != -1) &&
			(_vm->_goblin->getObjMaxFrame(gobDesc) == gobDesc->curFrame)) {

		gobDesc->stateMach = gobDesc->realStateMach;
		xPos = _vm->_goblin->_gobPositions[0].x;
		yPos = _vm->_goblin->_gobPositions[0].y;

		gobDesc->nextState = 10;
		layer = _vm->_goblin->nextLayer(gobDesc);

		_vm->_scenery->updateAnim(layer, 0, gobDesc->animation, 0,
				gobDesc->xPos, gobDesc->yPos, 0);

		gobDesc->yPos = (yPos * 6 + 6) -
			(_vm->_scenery->_toRedrawBottom - _vm->_scenery->_animTop);
		gobDesc->xPos = xPos * 12 -
			(_vm->_scenery->_toRedrawLeft - _vm->_scenery->_animLeft);
	}

	if (gobDesc->state != 10)
		return;

	if (_vm->_goblin->_itemIndInPocket == -1)
		return;

	if (gobDesc->curFrame != 10)
		return;

	params.objDesc = _vm->_goblin->_objects[_vm->_goblin->_itemIndInPocket];
	params.objDesc->type = 0;
	params.objDesc->toRedraw = 1;
	params.objDesc->curFrame = 0;

	params.objDesc->order = gobDesc->order;
	params.objDesc->animation =
			params.objDesc->stateMach[params.objDesc->state][0]->animation;

	layer = params.objDesc->stateMach[params.objDesc->state][0]->layer;

	_vm->_scenery->updateAnim(layer, 0, params.objDesc->animation, 0,
			params.objDesc->xPos, params.objDesc->yPos, 0);

	params.objDesc->yPos += (_vm->_goblin->_gobPositions[0].y * 6 + 5) -
		_vm->_scenery->_toRedrawBottom;

	if (gobDesc->curLookDir == 4) {
		params.objDesc->xPos += _vm->_goblin->_gobPositions[0].x * 12 + 14
				- (_vm->_scenery->_toRedrawLeft + _vm->_scenery->_toRedrawRight) / 2;
	} else {
		params.objDesc->xPos += _vm->_goblin->_gobPositions[0].x * 12
				- (_vm->_scenery->_toRedrawLeft + _vm->_scenery->_toRedrawRight) / 2;
	}

	_vm->_goblin->_itemIndInPocket = -1;
	_vm->_goblin->_itemIdInPocket = -1;
	_vm->_util->beep(50);
}

int16 Inter_v1::loadSound(int16 slot) {
	byte *dataPtr;
	int16 id;
	uint32 dataSize;
	SoundSource source;

	if (slot == -1)
		slot = _vm->_parse->parseValExpr();

	id = load16();
	if (id == -1) {
		_vm->_global->_inter_execPtr += 9;
		return 0;
	}

	if (id >= 30000) {
		source = SOUND_EXT;

		dataPtr = (byte *) _vm->_game->loadExtData(id, 0, 0, &dataSize);
	} else {
		int16 totSize;

		source = SOUND_TOT;

		dataPtr = (byte *) _vm->_game->loadTotResource(id, &totSize);
		dataSize = (uint32) ((int32) totSize);
	}

	if (dataPtr)
		_vm->_game->_soundSamples[slot].load(SOUND_SND, source,
				dataPtr, dataSize);
	return 0;
}

void Inter_v1::animPalette() {
	int16 i;
	Video::Color col;

	if (_animPalDir[0] == 0)
		return;

	_vm->_video->waitRetrace();

	if (_animPalDir[0] == -1) {
		col = _vm->_draw->_vgaSmallPalette[_animPalLowIndex[0]];

		for (i = _animPalLowIndex[0]; i < _animPalHighIndex[0]; i++)
			_vm->_draw->_vgaSmallPalette[i] = _vm->_draw->_vgaSmallPalette[i + 1];

		_vm->_draw->_vgaSmallPalette[_animPalHighIndex[0]] = col;
	} else {
		col = _vm->_draw->_vgaSmallPalette[_animPalHighIndex[0]];
		for (i = _animPalHighIndex[0]; i > _animPalLowIndex[0]; i--)
			_vm->_draw->_vgaSmallPalette[i] = _vm->_draw->_vgaSmallPalette[i - 1];

		_vm->_draw->_vgaSmallPalette[_animPalLowIndex[0]] = col;
	}

	_vm->_global->_pPaletteDesc->vgaPal = _vm->_draw->_vgaSmallPalette;
	_vm->_video->setFullPalette(_vm->_global->_pPaletteDesc);
}

void Inter_v1::manipulateMap(int16 xPos, int16 yPos, int16 item) {
	for (int y = 0; y < _vm->_map->_mapHeight; y++) {
		for (int x = 0; x < _vm->_map->_mapWidth; x++) {
			if ((_vm->_map->_itemsMap[y][x] & 0xFF) == item)
				_vm->_map->_itemsMap[y][x] &= 0xFF00;
			else if (((_vm->_map->_itemsMap[y][x] & 0xFF00) >> 8) == item)
				_vm->_map->_itemsMap[y][x] &= 0xFF;
		}
	}

	if (xPos < _vm->_map->_mapWidth - 1) {
		if (yPos > 0) {
			if (((_vm->_map->_itemsMap[yPos][xPos] & 0xFF00) != 0) ||
					((_vm->_map->_itemsMap[yPos - 1][xPos] & 0xFF00) != 0) ||
					((_vm->_map->_itemsMap[yPos][xPos + 1] & 0xFF00) != 0) ||
					((_vm->_map->_itemsMap[yPos - 1][xPos + 1] & 0xFF00) != 0)) {

				_vm->_map->_itemsMap[yPos][xPos] =
						(_vm->_map->_itemsMap[yPos][xPos] & 0xFF00) + item;

				_vm->_map->_itemsMap[yPos - 1][xPos] =
						(_vm->_map->_itemsMap[yPos - 1][xPos] & 0xFF00) + item;

				_vm->_map->_itemsMap[yPos][xPos + 1] =
						(_vm->_map->_itemsMap[yPos][xPos + 1] & 0xFF00) + item;

				_vm->_map->_itemsMap[yPos - 1][xPos + 1] =
						(_vm->_map->_itemsMap[yPos - 1][xPos + 1] & 0xFF00) + item;

			} else {
				_vm->_map->_itemsMap[yPos][xPos] =
						(_vm->_map->_itemsMap[yPos][xPos] & 0xFF) + (item << 8);

				_vm->_map->_itemsMap[yPos - 1][xPos] =
						(_vm->_map->_itemsMap[yPos - 1][xPos] & 0xFF) + (item << 8);

				_vm->_map->_itemsMap[yPos][xPos + 1] =
						(_vm->_map->_itemsMap[yPos][xPos + 1] & 0xFF) + (item << 8);

				_vm->_map->_itemsMap[yPos - 1][xPos + 1] =
						(_vm->_map->_itemsMap[yPos - 1][xPos + 1] & 0xFF) + (item << 8);
			}
		} else {
			if (((_vm->_map->_itemsMap[yPos][xPos] & 0xFF00) != 0) ||
					((_vm->_map->_itemsMap[yPos][xPos + 1] & 0xFF00) != 0)) {

				_vm->_map->_itemsMap[yPos][xPos] =
						(_vm->_map->_itemsMap[yPos][xPos] & 0xFF00) + item;

				_vm->_map->_itemsMap[yPos][xPos + 1] =
						(_vm->_map->_itemsMap[yPos][xPos + 1] & 0xFF00) + item;

			} else {
				_vm->_map->_itemsMap[yPos][xPos] =
						(_vm->_map->_itemsMap[yPos][xPos] & 0xFF) + (item << 8);

				_vm->_map->_itemsMap[yPos][xPos + 1] =
						(_vm->_map->_itemsMap[yPos][xPos + 1] & 0xFF) + (item << 8);
			}
		}
	} else {
		if (yPos > 0) {
			if (((_vm->_map->_itemsMap[yPos][xPos] & 0xFF00) != 0) ||
					((_vm->_map->_itemsMap[yPos - 1][xPos] & 0xFF00) != 0)) {

				_vm->_map->_itemsMap[yPos][xPos] =
						(_vm->_map->_itemsMap[yPos][xPos] & 0xFF00) + item;

				_vm->_map->_itemsMap[yPos - 1][xPos] =
						(_vm->_map->_itemsMap[yPos - 1][xPos] & 0xFF00) + item;

			} else {
				_vm->_map->_itemsMap[yPos][xPos] =
						(_vm->_map->_itemsMap[yPos][xPos] & 0xFF) + (item << 8);

				_vm->_map->_itemsMap[yPos - 1][xPos] =
						(_vm->_map->_itemsMap[yPos - 1][xPos] & 0xFF) + (item << 8);
			}
		} else {
			if ((_vm->_map->_itemsMap[yPos][xPos] & 0xFF00) != 0) {
				_vm->_map->_itemsMap[yPos][xPos] =
						(_vm->_map->_itemsMap[yPos][xPos] & 0xFF00) + item;
			} else {
				_vm->_map->_itemsMap[yPos][xPos] =
						(_vm->_map->_itemsMap[yPos][xPos] & 0xFF) + (item << 8);
			}
		}
	}

	if ((item < 0) || (item >= 20))
		return;

	if ((xPos > 1) && (_vm->_map->getPass(xPos - 2, yPos) == 1)) {
		_vm->_map->_itemPoses[item].x = xPos - 2;
		_vm->_map->_itemPoses[item].y = yPos;
		_vm->_map->_itemPoses[item].orient = 4;
		return;
	}

	if ((xPos < _vm->_map->_mapWidth - 2) &&
			(_vm->_map->getPass(xPos + 2, yPos) == 1)) {
		_vm->_map->_itemPoses[item].x = xPos + 2;
		_vm->_map->_itemPoses[item].y = yPos;
		_vm->_map->_itemPoses[item].orient = 0;
		return;
	}

	if ((xPos < _vm->_map->_mapWidth - 1) &&
			(_vm->_map->getPass(xPos + 1, yPos) == 1)) {
		_vm->_map->_itemPoses[item].x = xPos + 1;
		_vm->_map->_itemPoses[item].y = yPos;
		_vm->_map->_itemPoses[item].orient = 0;
		return;
	}

	if ((xPos > 0) && (_vm->_map->getPass(xPos - 1, yPos) == 1)) {
		_vm->_map->_itemPoses[item].x = xPos - 1;
		_vm->_map->_itemPoses[item].y = yPos;
		_vm->_map->_itemPoses[item].orient = 4;
		return;
	}
}

} // End of namespace Gob
