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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/engines/cruise/script.h $
 * $Id: script.h 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

#ifndef CRUISE_SCRIPT_H
#define CRUISE_SCRIPT_H

namespace Cruise {

enum scriptTypeEnum {
	scriptType_MinusPROC = -20,
	scriptType_Minus30 = -30,
	scriptType_PROC = 20,
	scriptType_REL = 30
};

struct scriptInstanceStruct {
	struct scriptInstanceStruct *nextScriptPtr;
	int16 var4;
	uint8 *var6;
	int16 varA;
	int16 scriptNumber;
	int16 overlayNumber;
	int16 sysKey;
	int16 freeze;
	scriptTypeEnum type;
	int16 var16;
	int16 var18;
	int16 var1A;
////// EXTRA ! not in original code. Needed for cross platform.
	int16 bitMask;
};

extern scriptInstanceStruct relHead;
extern scriptInstanceStruct procHead;
extern scriptInstanceStruct *currentScriptPtr;

void setupFuncArray(void);
uint8 getByteFromScript(void);

int removeScript(int overlay, int idx, scriptInstanceStruct * headPtr);
uint8 *attacheNewScriptToTail(int16 overlayNumber,
    scriptInstanceStruct * scriptHandlePtr, int16 param, int16 arg0,
    int16 arg1, int16 arg2, scriptTypeEnum scriptType);
void manageScripts(scriptInstanceStruct * scriptHandle);

} // End of namespace Cruise

#endif
