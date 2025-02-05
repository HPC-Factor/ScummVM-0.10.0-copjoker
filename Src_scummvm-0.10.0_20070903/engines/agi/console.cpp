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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/engines/agi/console.cpp $
 * $Id: console.cpp 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

#include "common/stdafx.h"

#include "agi/agi.h"
#include "agi/graphics.h"
#include "agi/sprite.h"
#include "agi/keyboard.h"
#include "agi/opcodes.h"
#include "agi/console.h"

namespace Agi {

Console::Console(AgiEngine *vm) : GUI::Debugger() {
	_vm = vm;

	DCmd_Register("debug",      WRAP_METHOD(Console, Cmd_Debug));
	DCmd_Register("cont",       WRAP_METHOD(Console, Cmd_Cont));
	DCmd_Register("agiver",     WRAP_METHOD(Console, Cmd_Agiver));
	DCmd_Register("crc",        WRAP_METHOD(Console, Cmd_Crc));
	DCmd_Register("flags",      WRAP_METHOD(Console, Cmd_Flags));
	DCmd_Register("logic0",     WRAP_METHOD(Console, Cmd_Logic0));
	DCmd_Register("objs",       WRAP_METHOD(Console, Cmd_Objs));
	DCmd_Register("runopcode",  WRAP_METHOD(Console, Cmd_RunOpcode));
	DCmd_Register("opcode",     WRAP_METHOD(Console, Cmd_Opcode));
	DCmd_Register("step",       WRAP_METHOD(Console, Cmd_Step));
	DCmd_Register("trigger",    WRAP_METHOD(Console, Cmd_Trigger));
	DCmd_Register("vars",       WRAP_METHOD(Console, Cmd_Vars));
	DCmd_Register("setvar",     WRAP_METHOD(Console, Cmd_SetVar));
	DCmd_Register("setflag",    WRAP_METHOD(Console, Cmd_SetFlag));
	DCmd_Register("setobj",     WRAP_METHOD(Console, Cmd_SetObj));
}

Console::~Console() {
}

void Console::preEnter() {
}

void Console::postEnter() {
}

bool Console::Cmd_SetVar(int argc, const char **argv) {
	if (argc != 3) {
		DebugPrintf("Usage: setvar <varnum> <value>");
		return true;
	}
	int p1 = (int)atoi(argv[1]);
	int p2 = (int)atoi(argv[2]);
	_vm->setvar(p1, p2);

	return true;
}

bool Console::Cmd_SetFlag(int argc, const char **argv) {
	if (argc != 3) {
		DebugPrintf("Usage: setvar <varnum> <value>");
		return true;
	}
	int p1 = (int)atoi(argv[1]);
	int p2 = (int)atoi(argv[2]);
	_vm->setflag(p1, !!p2);

	return true;
}

bool Console::Cmd_SetObj(int argc, const char **argv) {
	if (argc != 3) {
		DebugPrintf("Usage: setvar <varnum> <value>");
		return true;
	}
	int p1 = (int)atoi(argv[1]);
	int p2 = (int)atoi(argv[2]);
	_vm->objectSetLocation(p1, p2);

	return true;
}

bool Console::Cmd_RunOpcode(int argc, const char **argv) {
	for (int i = 0; logicNamesCmd[i].name; i++) {
		if (!strcmp(argv[1], logicNamesCmd[i].name)) {
			uint8 p[16];
			if ((argc - 2) != logicNamesCmd[i].numArgs) {
				DebugPrintf("AGI command wants %d arguments\n", logicNamesCmd[i].numArgs);
				return 0;
			}
			p[0] = argv[2] ? (char)strtoul(argv[2], NULL, 0) : 0;
			p[1] = argv[3] ? (char)strtoul(argv[3], NULL, 0) : 0;
			p[2] = argv[4] ? (char)strtoul(argv[4], NULL, 0) : 0;
			p[3] = argv[5] ? (char)strtoul(argv[5], NULL, 0) : 0;
			p[4] = argv[6] ? (char)strtoul(argv[6], NULL, 0) : 0;

			debugC(5, kDebugLevelMain, "Opcode: %s %s %s %s", logicNamesCmd[i].name, argv[1], argv[2], argv[3]);

			_vm->executeAgiCommand(i, p);

			return true;
		}
	}

	return true;
}

bool Console::Cmd_Crc(int argc, const char **argv) {
	DebugPrintf("command removed from scummvm\n");

	return true;
}

bool Console::Cmd_Agiver(int argc, const char **argv) {
	int ver, maj, min;

	ver = _vm->agiGetRelease();
	maj = (ver >> 12) & 0xf;
	min = ver & 0xfff;

	DebugPrintf(maj <= 2 ? "%x.%03x\n" : "%x.002.%03x\n", maj, min);

	return true;
}

bool Console::Cmd_Flags(int argc, const char **argv) {
	int i, j;

	DebugPrintf("    ");
	for (j = 0; j < 10; j++)
		DebugPrintf("%d ", j);
	DebugPrintf("\n");

	for (i = 0; i < 255;) {
		DebugPrintf("%3d ", i);
		for (j = 0; j < 10; j++, i++) {
			DebugPrintf("%c ", _vm->getflag(i) ? 'T' : 'F');
		}
		report("\n");
	}

	return true;
}

bool Console::Cmd_Vars(int argc, const char **argv) {
	int i, j;

	for (i = 0; i < 255;) {
		for (j = 0; j < 5; j++, i++) {
			DebugPrintf("%03d:%3d ", i, _vm->getvar(i));
		}
		DebugPrintf("\n");
	}

	return true;
}

bool Console::Cmd_Objs(int argc, const char **argv) {
	unsigned int i;

	for (i = 0; i < _vm->_game.numObjects; i++) {
		DebugPrintf("%3d]%-24s(%3d)\n", i, _vm->objectName(i), _vm->objectGetLocation(i));
	}

	return true;
}

bool Console::Cmd_Opcode(int argc, const char **argv) {
	if (argc != 2 || (strcmp(argv[1], "on") && strcmp(argv[1], "off"))) {
		DebugPrintf("Usage: opcode on|off\n");
		return true;
	}

	_vm->_debug.opcodes = !strcmp(argv[1], "on");

	return true;
}

bool Console::Cmd_Logic0(int argc, const char **argv) {
	if (argc != 2 || (strcmp(argv[1], "on") && strcmp(argv[1], "off"))) {
		DebugPrintf("Usage: logic0 on|off\n");
		return true;
	}

	_vm->_debug.logic0 = !strcmp(argv[1], "on");

	return true;
}

bool Console::Cmd_Trigger(int argc, const char **argv) {
	if (argc != 2 || (strcmp(argv[1], "on") && strcmp(argv[1], "off"))) {
		DebugPrintf("Usage: trigger on|off\n");
		return true;
	}
	_vm->_debug.ignoretriggers = strcmp (argv[1], "on");

	return true;
}

bool Console::Cmd_Step(int argc, const char **argv) {
	_vm->_debug.enabled = 1;

	if (argc == 1) {
		_vm->_debug.steps = 1;
		return true;
	}

	_vm->_debug.steps = strtoul(argv[1], NULL, 0);

	return true;
}

bool Console::Cmd_Debug(int argc, const char **argv) {
	_vm->_debug.enabled = 1;
	_vm->_debug.steps = 0;

	return true;
}

bool Console::Cmd_Cont(int argc, const char **argv) {
	_vm->_debug.enabled = 0;
	_vm->_debug.steps = 0;

	return true;
}

} // End of namespace Agi
