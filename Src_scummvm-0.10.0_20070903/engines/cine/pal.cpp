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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/engines/cine/pal.cpp $
 * $Id: pal.cpp 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

#include "cine/cine.h"
#include "cine/various.h"

namespace Cine {

uint16 tempPalette[256];

byte colorMode256 = 0;
byte palette256[256 * 3];

uint16 palEntriesCount;

PalEntry *palPtr = NULL;

byte paletteBuffer1[16];
byte paletteBuffer2[16];

void loadPal(const char *fileName) {
	char buffer[20];

	removeExtention(buffer, fileName);

	strcat(buffer, ".PAL");

	if (palPtr) {
		free(palPtr);
		palPtr = NULL;
	}

	palEntriesCount = 0;

	Common::File palFileHandle;
	if (!palFileHandle.open(buffer))
		error("loadPal(): Cannot open file %s", fileName);

	palEntriesCount = palFileHandle.readUint16LE();
	palFileHandle.readUint16LE(); // entry size
	
	palPtr = (PalEntry *)malloc(palEntriesCount * sizeof(PalEntry));
	assert(palPtr);
	for (int i = 0; i < palEntriesCount; ++i) {
		palFileHandle.read(palPtr[i].name, 10);
		palFileHandle.read(palPtr[i].pal1, 16);
		palFileHandle.read(palPtr[i].pal2, 16);
	}
	palFileHandle.close();
}

int16 findPaletteFromName(const char *fileName) {
	char buffer[10];
	uint16 position = 0;
	uint16 i;

	strcpy(buffer, fileName);

	while (position < strlen(fileName)) {
		if (buffer[position] > 'a' && buffer[position] < 'z') {
			buffer[position] += 'A' - 'a';
		}

		position++;
	}

	for (i = 0; i < palEntriesCount; i++) {
		if (!strcmp(buffer, palPtr[i].name)) {
			return i;
		}
	}

	return -1;

}

void loadRelatedPalette(const char *fileName) {
	char localName[16];
	byte i;
	int16 paletteIndex;

	removeExtention(localName, fileName);

	paletteIndex = findPaletteFromName(localName);

	if (paletteIndex == -1) {
		for (i = 0; i < 16; i++) {	// generate default palette
			paletteBuffer1[i] = paletteBuffer2[i] = (i << 4) + i;
		}
	} else {
		assert(paletteIndex < palEntriesCount);
		memcpy(paletteBuffer1, palPtr[paletteIndex].pal1, 16);
		memcpy(paletteBuffer2, palPtr[paletteIndex].pal2, 16);
	}
}

} // End of namespace Cine
