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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/engines/scumm/script.h $
 * $Id: script.h 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

#ifndef SCUMM_SCRIPT_H
#define SCUMM_SCRIPT_H

#include "engines/engine.h"

namespace Scumm {

/**
 * The number of script slots, which determines the maximal number
 * of concurrently running scripts.
 * WARNING: Do NOT changes this value unless you really have to, as
 * this will break savegame compatibility if done carelessly. If you
 * have to change it, make sure you update saveload.cpp accordingly!
 */
enum {
	NUM_SCRIPT_SLOT = 80
};

/* Script status type (slot.status) */
enum {
	ssDead = 0,
	ssPaused = 1,
	ssRunning = 2
};

struct ScriptSlot {
	uint32 offs;
	int32 delay;
	uint16 number;
	uint16 delayFrameCount;
	bool freezeResistant, recursive;
	bool didexec;
	byte status;
	byte where;
	byte freezeCount;
	byte cutsceneOverride;
	byte cycle;
};

struct NestedScript {
	uint16 number;
	uint8 where;
	uint8 slot;
};

struct VirtualMachineState {
	uint32 cutScenePtr[5];
	byte cutSceneScript[5];
	int16 cutSceneData[5];
	int16 cutSceneScriptIndex;
	byte cutSceneStackPointer;
	ScriptSlot slot[NUM_SCRIPT_SLOT];
	int32 localvar[NUM_SCRIPT_SLOT][26];

	NestedScript nest[15];
	byte numNestedScripts;
};

} // End of namespace Scumm

#endif
