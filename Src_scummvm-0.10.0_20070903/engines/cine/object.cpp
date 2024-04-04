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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/engines/cine/object.cpp $
 * $Id: object.cpp 27377 2007-06-12 12:50:30Z thebluegr $
 *
 */

#include "common/stdafx.h"
#include "common/endian.h"
#include "common/scummsys.h"
#include "common/util.h"

#include "cine/cine.h"
#include "cine/object.h"
#include "cine/part.h"
#include "cine/various.h"

namespace Cine {

objectStruct objectTable[NUM_MAX_OBJECT];
uint16 globalVars[NUM_MAX_OBJECTDATA];
overlayHeadElement overlayHead;

void unloadAllMasks(void) {
	overlayHeadElement *current = overlayHead.next;

	while (current) {
		overlayHeadElement *next = current->next;

		free(current);

		current = next;
	}

	resetMessageHead();
}

void resetMessageHead(void) {
	overlayHead.next = NULL;
	overlayHead.previous = NULL;
}

void loadObject(char *pObjectName) {
	uint16 numEntry;
	uint16 entrySize;
	uint16 i;
	byte *ptr, *dataPtr;

	checkDataDisk(-1);

	ptr = dataPtr = readBundleFile(findFileInBundle(pObjectName));

	setMouseCursor(MOUSE_CURSOR_DISK);

	numEntry = READ_BE_UINT16(ptr); ptr += 2;

	entrySize = READ_BE_UINT16(ptr); ptr += 2;

	assert(numEntry <= NUM_MAX_OBJECT);

	for (i = 0; i < numEntry; i++) {
		if (objectTable[i].costume != -2) {	// flag is keep ?
			Common::MemoryReadStream readS(ptr, entrySize);

			objectTable[i].x = readS.readSint16BE();
			objectTable[i].y = readS.readSint16BE();
			objectTable[i].mask = readS.readUint16BE();
			objectTable[i].frame = readS.readSint16BE();
			objectTable[i].costume = readS.readSint16BE();
			readS.read(objectTable[i].name, 20);
			objectTable[i].part = readS.readUint16BE();
		}
		ptr += entrySize;
	}

	if (!strcmp(pObjectName, "INTRO.OBJ")) {
		for (i = 0; i < 10; i++) {
			objectTable[i].costume = 0;
		}
	}

	free(dataPtr);
}

int8 removeOverlayElement(uint16 objIdx, uint16 param) {
	overlayHeadElement *currentHeadPtr = &overlayHead;
	overlayHeadElement *tempHead = currentHeadPtr;
	overlayHeadElement *tempPtr2;

	currentHeadPtr = tempHead->next;

	while (currentHeadPtr && (objIdx != currentHeadPtr->objIdx || param != currentHeadPtr->type)) {
		tempHead = currentHeadPtr;
		currentHeadPtr = tempHead->next;
	}

	if (!currentHeadPtr) {
		return -1;
	}

	if (objIdx != currentHeadPtr->objIdx || param != currentHeadPtr->type) {
		return -1;
	}

	tempHead->next = currentHeadPtr->next;
	tempPtr2 = currentHeadPtr->next;

	if (!tempPtr2) {
		tempPtr2 = &overlayHead;
	}

	tempPtr2->previous = currentHeadPtr->previous;

	free(currentHeadPtr);

	return 0;
}

int16 freeOverlay(uint16 objIdx, uint16 param) {
	overlayHeadElement *currentHeadPtr = overlayHead.next;
	overlayHeadElement *previousPtr = &overlayHead;

	while (currentHeadPtr && ((currentHeadPtr->objIdx != objIdx) || (currentHeadPtr->type != param))) {
		previousPtr = currentHeadPtr;
		currentHeadPtr = previousPtr->next;
	}

	if (!currentHeadPtr) {
		return -1;
	}

	if (!((currentHeadPtr->objIdx == objIdx) && (currentHeadPtr->type == param))) {
		return -1;
	}

	previousPtr->next = currentHeadPtr->next;
	overlayHeadElement *tempPtr2 = currentHeadPtr->next;

	if (!tempPtr2) {
		tempPtr2 = &overlayHead;
	}

	tempPtr2->previous = currentHeadPtr->previous;

	// FIXME: is this needed? It causes crashes in Windows in the drawOverlays function
	// (the currentOverlay pointer is incorrect)
	// Removing this fixes bug #1733238 - FW: crash in copier room
	// Also, it stops the game from crashing right after the introduction
	//free(currentHeadPtr);
	return 0;
}

void loadOverlayElement(uint16 objIdx, uint16 param) {
	overlayHeadElement *currentHeadPtr = &overlayHead;
	overlayHeadElement *pNewElement;

	uint16 si = objectTable[objIdx].mask;

	overlayHeadElement *tempHead = currentHeadPtr;

	currentHeadPtr = tempHead->next;

	while (currentHeadPtr && (objectTable[currentHeadPtr->objIdx].mask < si)) {
		tempHead = currentHeadPtr;
		currentHeadPtr = tempHead->next;
	}

	pNewElement = (overlayHeadElement *)malloc(sizeof(overlayHeadElement));

	assert(pNewElement);

	pNewElement->next = tempHead->next;
	tempHead->next = pNewElement;

	pNewElement->objIdx = objIdx;
	pNewElement->type = param;

	if (!currentHeadPtr) {
		currentHeadPtr = &overlayHead;
	}

	pNewElement->previous = currentHeadPtr->previous;
	currentHeadPtr->previous = pNewElement;
}

void setupObject(byte objIdx, uint16 param1, uint16 param2, uint16 param3, uint16 param4) {
	objectTable[objIdx].x = param1;
	objectTable[objIdx].y = param2;
	objectTable[objIdx].mask = param3;
	objectTable[objIdx].frame = param4;

	if (!removeOverlayElement(objIdx, 0)) {
		loadOverlayElement(objIdx, 0);
	}
}

void subObjectParam(byte objIdx, byte paramIdx, int16 newValue) {
	paramIdx--;

	assert(paramIdx <= 5);

	switch (paramIdx) {
	case 0:
		objectTable[objIdx].x -= newValue;
		break;
	case 1:
		objectTable[objIdx].y -= newValue;
		break;
	case 2:
		objectTable[objIdx].mask -= newValue;

		if (!removeOverlayElement(objIdx, 0)) {
			loadOverlayElement(objIdx, 0);
		}
		break;
	case 3:
		objectTable[objIdx].frame -= newValue;
		break;
	case 4:
		objectTable[objIdx].costume -= newValue;
		break;
	case 5:
		objectTable[objIdx].part -= newValue;
		break;
	}
}

void addObjectParam(byte objIdx, byte paramIdx, int16 newValue) {
	paramIdx--;

	assert(paramIdx <= 5);

	switch (paramIdx) {
	case 0:
		objectTable[objIdx].x += newValue;
		break;
	case 1:
		objectTable[objIdx].y += newValue;
		break;
	case 2:
		objectTable[objIdx].mask += newValue;

		if (!removeOverlayElement(objIdx, 0)) {
			loadOverlayElement(objIdx, 0);
		}
		break;
	case 3:
		objectTable[objIdx].frame += newValue;
		break;
	case 4:
		objectTable[objIdx].costume += newValue;
		break;
	case 5:
		objectTable[objIdx].part += newValue;
		break;
	}
}

void modifyObjectParam(byte objIdx, byte paramIdx, int16 newValue) {
	paramIdx--;

	assert(paramIdx <= 5);

	switch (paramIdx) {
	case 0:
		objectTable[objIdx].x = newValue;
		break;
	case 1:
		objectTable[objIdx].y = newValue;
		break;
	case 2:
		objectTable[objIdx].mask = newValue;

		if (!removeOverlayElement(objIdx, 0)) {
			loadOverlayElement(objIdx, 0);
		}
		break;
	case 3:
		objectTable[objIdx].frame = newValue;
		break;
	case 4:
		if (newValue == -1) {
			objectTable[objIdx].costume = globalVars[0];
		} else {
			objectTable[objIdx].costume = newValue;
		}
		break;
	case 5:
		objectTable[objIdx].part = newValue;
		break;
	}
}

byte compareObjectParam(byte objIdx, byte type, int16 value) {
	byte compareResult = 0;

	switch (type - 1) {
	case 0:
		if (objectTable[objIdx].x == value) {
			compareResult |= kCmpEQ;
		}

		if (objectTable[objIdx].x > value) {
			compareResult |= kCmpGT;
		}

		if (objectTable[objIdx].x < value) {
			compareResult |= kCmpLT;
		}

		break;
	case 1:
		if (objectTable[objIdx].y == value) {
			compareResult |= kCmpEQ;
		}

		if (objectTable[objIdx].y > value) {
			compareResult |= kCmpGT;
		}

		if (objectTable[objIdx].y < value) {
			compareResult |= kCmpLT;
		}

		break;
	case 2:
		if (objectTable[objIdx].mask == value) {
			compareResult |= kCmpEQ;
		}

		if (objectTable[objIdx].mask > value) {
			compareResult |= kCmpGT;
		}

		if (objectTable[objIdx].mask < value) {
			compareResult |= kCmpLT;
		}

		break;
	case 3:
		if (objectTable[objIdx].frame == value) {
			compareResult |= kCmpEQ;
		}

		if (objectTable[objIdx].frame > value) {
			compareResult |= kCmpGT;
		}

		if (objectTable[objIdx].frame < value) {
			compareResult |= kCmpLT;
		}

		break;
	case 4:
		if (objectTable[objIdx].costume == value) {
			compareResult |= kCmpEQ;
		}

		if (objectTable[objIdx].costume > value) {
			compareResult |= kCmpGT;
		}

		if (objectTable[objIdx].costume < value) {
			compareResult |= kCmpLT;
		}

		break;
	default:
		error("Unsupported compare type: %d in compareObjectParam", type - 1);
	}

	return compareResult;
}

int16 getObjectParam(uint16 objIdx, uint16 paramIdx) {
	assert(objIdx <= NUM_MAX_OBJECT);

	paramIdx--;

	assert(paramIdx <= 5);

	switch (paramIdx) {
	case 0:
		return objectTable[objIdx].x;
	case 1:
		return objectTable[objIdx].y;
	case 2:
		return objectTable[objIdx].mask;
	case 3:
		return objectTable[objIdx].frame;
	case 4:
		return objectTable[objIdx].costume;
	case 5:
		return objectTable[objIdx].part;
	}

	return 0;
}

} // End of namespace Cine
