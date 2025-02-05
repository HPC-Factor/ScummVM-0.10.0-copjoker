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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/engines/cine/part.h $
 * $Id: part.h 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

#ifndef CINE_PART_H
#define CINE_PART_H

namespace Cine {

struct PartBuffer {
	char partName[14];
	uint32 offset;
	uint32 packedSize;
	uint32 unpackedSize;
};

struct AnimData {
	uint16 width;
	uint16 var1;
	uint16 bpp;
	uint16 height;

	byte *ptr1;
	byte *ptr2;
	int16 fileIdx;
	int16 frameIdx;
	char name[10];

	// Not part of the data, but used when saving/restoring it.
	bool refresh;
};

#define NUM_MAX_PARTDATA 255
#define NUM_MAX_ANIMDATA 255

extern AnimData *animDataTable;
extern PartBuffer *partBuffer;

void loadPart(const char *partName);
void closePart(void);

int16 findFileInBundle(const char *fileName);

void readFromPart(int16 idx, byte *dataPtr);

byte *readBundleFile(int16 foundFileIdx);
byte *readBundleSoundFile(const char *entryName, uint32 *size = 0);
byte *readFile(const char *filename);

void checkDataDisk(int16 param);

void dumpBundle(const char *filename);

} // End of namespace Cine

#endif
