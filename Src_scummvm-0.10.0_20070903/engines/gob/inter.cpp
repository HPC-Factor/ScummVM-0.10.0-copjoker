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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/engines/gob/inter.cpp $
 * $Id: inter.cpp 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

#include "common/stdafx.h"
#include "common/endian.h"

#include "gob/gob.h"
#include "gob/inter.h"
#include "gob/global.h"
#include "gob/util.h"
#include "gob/draw.h"
#include "gob/game.h"
#include "gob/parse.h"
#include "gob/scenery.h"
#include "gob/sound.h"

namespace Gob {

Inter::Inter(GobEngine *vm) : _vm(vm) {
	_terminate = 0;
	_break = false;

	for (int i = 0; i < 8; i++) {
		_animPalLowIndex[i] = 0;
		_animPalHighIndex[i] = 0;
		_animPalDir[i] = 0;
	}

	_breakFromLevel = 0;
	_nestLevel = 0;

	_soundEndTimeKey = 0;
	_soundStopVal = 0;

	memset(_pasteBuf, 0, 300);
	memset(_pasteSizeBuf, 0, 300);
	_pastePos = 0;

	_noBusyWait = false;
}

void Inter::initControlVars(char full) {
	*_nestLevel = 0;
	*_breakFromLevel = -1;

	*_vm->_scenery->_pCaptureCounter = 0;

	_break = false;
	_terminate = 0;

	if (full == 1) {
		for (int i = 0; i < 8; i++)
			_animPalDir[i] = 0;
		_soundEndTimeKey = 0;
	}
}

int16 Inter::load16() {
	int16 tmp = (int16) READ_LE_UINT16(_vm->_global->_inter_execPtr);
	_vm->_global->_inter_execPtr += 2;
	return tmp;
}

char Inter::evalExpr(int16 *pRes) {
	byte token;

	_vm->_parse->printExpr(99);

	_vm->_parse->parseExpr(99, &token);
	if (!pRes)
		return token;

	switch (token) {
	case 20:
		*pRes = _vm->_global->_inter_resVal;
		break;

	case 22:
	case 23:
		*pRes = 0;
		break;

	case 24:
		*pRes = 1;
		break;
	}

	return token;
}

bool Inter::evalBoolResult() {
	byte token;

	_vm->_parse->printExpr(99);

	_vm->_parse->parseExpr(99, &token);
	if ((token == 24) || ((token == 20) && _vm->_global->_inter_resVal))
		return true;
	else
		return false;
}

void Inter::renewTimeInVars() {
	struct tm *t;
	time_t now = time(NULL);

	t = localtime(&now);

	WRITE_VAR(5, 1900 + t->tm_year);
	WRITE_VAR(6, t->tm_mon + 1);
	WRITE_VAR(7, 0);
	WRITE_VAR(8, t->tm_mday);
	WRITE_VAR(9, t->tm_hour);
	WRITE_VAR(10, t->tm_min);
	WRITE_VAR(11, t->tm_sec);
}

void Inter::storeMouse() {
	int16 x;
	int16 y;

	x = _vm->_global->_inter_mouseX;
	y = _vm->_global->_inter_mouseY;
	_vm->_draw->adjustCoords(1, &x, &y);

	WRITE_VAR(2, x);
	WRITE_VAR(3, y);
	WRITE_VAR(4, _vm->_game->_mouseButtons);
}

void Inter::storeKey(int16 key) {
	WRITE_VAR(12, _vm->_util->getTimeKey() - _vm->_game->_startTimeKey);

	storeMouse();
	WRITE_VAR(1, _vm->_snd->_playingSound);

	if (key == 0x4800)
		key = 0x0B;
	else if (key == 0x5000)
		key = 0x0A;
	else if (key == 0x4D00)
		key = 0x09;
	else if (key == 0x4B00)
		key = 0x08;
	else if (key == 0x011B)
		key = 0x1B;
	else if (key == 0x0E08)
		key = 0x19;
	else if (key == 0x5300)
		key = 0x1A;
	else if ((key & 0xFF) != 0)
		key &= 0xFF;

	WRITE_VAR(0, key);

	if (key != 0)
		_vm->_util->clearKeyBuf();
}

void Inter::funcBlock(int16 retFlag) {
	OpFuncParams params;
	byte cmd;
	byte cmd2;

	params.retFlag = retFlag;

	if (!_vm->_global->_inter_execPtr)
		return;

	_break = false;
	_vm->_global->_inter_execPtr++;
	params.cmdCount = *_vm->_global->_inter_execPtr++;
	_vm->_global->_inter_execPtr += 2;

	if (params.cmdCount == 0) {
		_vm->_global->_inter_execPtr = 0;
		return;
	}

	int startaddr = _vm->_global->_inter_execPtr - _vm->_game->_totFileData;

	params.counter = 0;
	do {
		if (_terminate)
			break;

		// WORKAROUND:
		// The EGA version of gob1 doesn't add a delay after showing
		// images between levels. We manually add it here.
		if ((_vm->_features & GF_GOB1) && (_vm->_features & GF_EGA)) {
			int addr = _vm->_global->_inter_execPtr-_vm->_game->_totFileData;
			if ((startaddr == 0x18B4 && addr == 0x1A7F && // Zombie
				 !strncmp(_vm->_game->_curTotFile, "avt005.tot", 10)) ||
				(startaddr == 0x1299 && addr == 0x139A && // Dungeon
				 !strncmp(_vm->_game->_curTotFile, "avt006.tot", 10)) ||
				(startaddr == 0x11C0 && addr == 0x12C9 && // Cauldron
				 !strncmp(_vm->_game->_curTotFile, "avt012.tot", 10)) ||
				(startaddr == 0x09F2 && addr == 0x0AF3 && // Statue
				 !strncmp(_vm->_game->_curTotFile, "avt016.tot", 10)) ||
				(startaddr == 0x0B92 && addr == 0x0C93 && // Castle
				 !strncmp(_vm->_game->_curTotFile, "avt019.tot", 10)) ||
				(startaddr == 0x17D9 && addr == 0x18DA && // Finale
				 !strncmp(_vm->_game->_curTotFile, "avt022.tot", 10))) {

				_vm->_util->longDelay(5000);
			}
		} // End of workaround

		cmd = *_vm->_global->_inter_execPtr;
		if ((cmd >> 4) >= 12) {
			cmd2 = 16 - (cmd >> 4);
			cmd &= 0xF;
		} else
			cmd2 = 0;

		_vm->_global->_inter_execPtr++;
		params.counter++;

		if (cmd2 == 0)
			cmd >>= 4;

		if (executeFuncOpcode(cmd2, cmd, params))
			return;

		if (_vm->_quitRequested)
			break;

		if (_break) {
			if (params.retFlag != 2)
				break;

			if (*_breakFromLevel == -1)
				_break = false;
			break;
		}
	} while (params.counter != params.cmdCount);

	_vm->_global->_inter_execPtr = 0;
	return;
}

void Inter::callSub(int16 retFlag) {
	byte block;

	while (!_vm->_quitRequested && _vm->_global->_inter_execPtr &&
			(_vm->_global->_inter_execPtr != _vm->_game->_totFileData)) {

		block = *_vm->_global->_inter_execPtr;
		if (block == 1)
			funcBlock(retFlag);
		else if (block == 2)
			_vm->_game->collisionsBlock();
		else
			error("Unknown block type %d in Inter::callSub()", block);
	}

	if (_vm->_global->_inter_execPtr == _vm->_game->_totFileData)
		_terminate = 1;
}

} // End of namespace Gob
