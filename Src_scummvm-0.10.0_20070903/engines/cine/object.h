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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/engines/cine/object.h $
 * $Id: object.h 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

#ifndef CINE_OBJECT_H
#define CINE_OBJECT_H

namespace Cine {

struct objectStruct {
	int16 x;
	int16 y;
	uint16 mask;
	int16 frame;
	int16 costume;
	char name[20];
	uint16 part;
};

struct overlayHeadElement {
	struct overlayHeadElement *next;
	struct overlayHeadElement *previous;
	uint16 objIdx;
	uint16 type;
	int16 x;
	int16 y;
	int16 width;
	int16 color;
};

#define NUM_MAX_OBJECT 255
#define NUM_MAX_OBJECTDATA 255

extern objectStruct objectTable[NUM_MAX_OBJECT];
extern uint16 globalVars[NUM_MAX_OBJECTDATA];

extern overlayHeadElement overlayHead;

void unloadAllMasks(void);
void resetMessageHead(void);

void loadObject(char *pObjectName);
void setupObject(byte objIdx, uint16 param1, uint16 param2, uint16 param3, uint16 param4);
void modifyObjectParam(byte objIdx, byte paramIdx, int16 newValue);

void loadOverlayElement(uint16 objIdx, uint16 param);
int8 removeOverlayElement(uint16 objIdx, uint16 param);

int16 getObjectParam(uint16 objIdx, uint16 paramIdx);
int16 freeOverlay(uint16 objIdx, uint16 param);

void addObjectParam(byte objIdx, byte paramIdx, int16 newValue);
void subObjectParam(byte objIdx, byte paramIdx, int16 newValue);
byte compareObjectParam(byte objIdx, byte param1, int16 param2);

} // End of namespace Cine

#endif
