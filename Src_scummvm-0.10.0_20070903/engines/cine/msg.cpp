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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/engines/cine/msg.cpp $
 * $Id: msg.cpp 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

#include "common/stdafx.h"
#include "common/endian.h"

#include "cine/msg.h"
#include "cine/various.h"

namespace Cine {

uint16 messageCount;

void loadMsg(char *pMsgName) {
	uint16 i;
	byte *ptr, *dataPtr;

	checkDataDisk(-1);

	messageCount = 0;

	for (i = 0; i < NUM_MAX_MESSAGE; i++) {
		messageTable[i].len = 0;
		if (messageTable[i].ptr) {
			free(messageTable[i].ptr);
			messageTable[i].ptr = NULL;
		}
	}

	ptr = dataPtr = readBundleFile(findFileInBundle(pMsgName));

	setMouseCursor(MOUSE_CURSOR_DISK);

	messageCount = READ_BE_UINT16(ptr); ptr += 2;

	assert(messageCount <= NUM_MAX_MESSAGE);

	for (i = 0; i < messageCount; i++) {
		messageTable[i].len = READ_BE_UINT16(ptr); ptr += 2;
	}

	for (i = 0; i < messageCount; i++) {
		if (messageTable[i].len) {
			messageTable[i].ptr = (byte *) malloc(messageTable[i].len);

			assert(messageTable[i].ptr);

			memcpy(messageTable[i].ptr, ptr, messageTable[i].len);
			ptr += messageTable[i].len;
		}
	}

	free(dataPtr);
}

} // End of namespace Cine
