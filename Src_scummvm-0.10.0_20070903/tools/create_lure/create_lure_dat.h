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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/tools/create_lure/create_lure_dat.h $
 * $Id: create_lure_dat.h 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

#ifndef __createlure_dat__
#define __createlure_dat__

#include "common/stdafx.h"
#include "common/endian.h"

#define VERSION_MAJOR 1
#define VERSION_MINOR 19
#define ENGLISH_LURE 

#define DATA_SEGMENT 0xac50

#define DIALOG_OFFSET 0x1dcb0
#define DIALOG_SIZE 0x150

#define TALK_DIALOG_OFFSET 0x1de00
#define TALK_DIALOG_SIZE 0x30

#define PALETTE_OFFSET 0xc0a7
#define PALETTE_SIZE 0x300

#define ROOM_TABLE 0xbf40
#define ROOM_NUM_ENTRIES 51

#define HOTSPOT_OVERRIDE_OFFSET 0x2A01

#define SCRIPT_SEGMENT 0x1df00
#define SCRIPT_SEGMENT_SIZE 0x2c57
#define SCRIPT2_SEGMENT 0x19c70
#define SCRIPT2_SEGMENT_SIZE 0x2800

#define HOTSPOT_SCRIPT_LIST 0x57e0
#define HOTSPOT_SCRIPT_SIZE 0x30

#define MAX_NUM_ANIM_RECORDS 0x200
#define MAX_NUM_ACTION_RECORDS 0x100

#define ROOM_EXITS_OFFSET 0x2f61
#define NUM_ROOM_EXITS 50
#define ROOM_EXIT_JOINS_OFFSET 0xce30

#define MESSAGES_SEGMENT 0x20b60
#define MESSAGES_SEGMENT_SIZE 0x490

#define MAX_HOTSPOTS 0x100
#define MAX_DATA_SIZE 0x4000

#define PATHFIND_OFFSET 0x984A
#define PATHFIND_SIZE (120 * ROOM_NUM_ENTRIES) 

#define HOTSPOT_WALK_TO_OFFSET 0xBC4B
#define EXIT_COORDINATES_OFFSET 0x1929
#define EXIT_COORDINATES_NUM_ROOMS 49

#define TABLED_ACTIONS_OFFSET 0x1380
#define NUM_TABLED_ACTION_BLOCKS 33
#define RANDOM_ACTIONS_OFFSET 0x4D10
#define RANDOM_ROOM_NUM_ENTRIES 41

#define WALK_AREAS_OFFSET 0x2EB1

#define EXIT_HOTSPOTS_OFFSET 0x2E57

#pragma pack(1)

// Rect currently copied from common/rect.h - if I try directly including it,
// the link complains about an unresolved external token Common.String.__dtor

struct Rect {
	int16 top, left;		//!< The point at the top left of the rectangle (part of the rect).
	int16 bottom, right;	//!< The point at the bottom right of the rectangle (not part of the rect).
};

struct FileEntry {
	uint16 id;
	byte unused;
	byte sizeExtension;
	uint16 size;
	uint16 offset;
};

struct RoomHeaderEntry {
	uint16 offset;
	uint16 roomNumber;
	uint16 descId;
	uint16 unused;
	byte hdrFlags;
};

struct HotspotHeaderEntry {
	uint16 offset;
	uint16 resourceId;
	uint16 descId, descId2;
	byte hdrFlags;
};

struct HotspotResource {
	uint32 actions;
	uint16 actionsOffset;
	uint16 roomNumber;
	byte scriptLoadFlag;
	uint16 loadOffset;
	uint16 unused;
	uint16 startX;
	uint16 startY;
	uint16 width;
	uint16 height;
	byte layer;
	byte flags;
	uint16 tickProcOffset;
	uint16 widthCopy;
	uint16 heightCopy;
	uint16 yCorrection;
	uint16 tickTimeout;
	uint16 animOffset;
	byte colourOffset;
	uint16 hotspotScriptOffset;
	byte unused1[7];
	uint16 talkScriptOffset;
	byte unused2[6];
	int8 talkX;
	int8 talkY;
	byte unused3[11];
	uint16 delayCtr;
	uint8 characterMode;
	uint16 tickSequenceOffset;
};

struct CurrentActionInput {
	uint8 action;
	uint16 dataOffset;
	uint16 roomNumber;
};

struct HotspotResourceOutput {
	uint16 hotspotId;
	uint16 nameId;
	uint16 descId;
	uint16 descId2;
	uint32 actions;
	uint16 actionsOffset;
	uint16 roomNumber;
	byte layer;
	byte scriptLoadFlag;
	uint16 loadOffset;
	uint16 startX;
	uint16 startY;
	uint16 width;
	uint16 height;
	uint16 widthCopy;
	uint16 heightCopy;
	uint16 yCorrection;
	int16 walkX;
	uint16 walkY;
	int8 talkX;
	int8 talkY;
	uint16 colourOffset;
	uint16 animRecordId;
	uint16 hotspotScriptOffset;
	uint16 talkScriptOffset;
	uint16 tickProcOffset;
	uint16 tickTimeout;
	uint16 tickSequenceOffset;
	uint16 npcSchedule;
	uint16 characterMode;
	uint16 delayCtr;
	byte flags;
	byte hdrFlags;
};

struct RoomResource {
	uint32 actions;
	uint16 unknown1;
	uint16 pixelListOffset;
	byte numLayers;
	uint16 layers[4];
	uint16 sequenceOffset;
	byte unknown2[5];
	uint8 walkBoundsIndex;
	int16 clippingXStart;
	int16 clippingXEnd;
};

struct RoomRectIn {
	uint16 xs, xe;
	uint16 ys, ye;
};

struct RoomRectOut {
	int16 xs, xe;
	int16 ys, ye;
};

struct RoomResourceOutput {
	uint16 roomNumber;
	uint8 hdrFlags;
	uint8 unused;
	uint32 actions;
	uint16 descId;
	uint16 numLayers;
	uint16 layers[4];
	uint16 sequenceOffset;
	int16 clippingXStart;
	int16 clippingXEnd;
	RoomRectOut walkBounds;
	uint16 numExits;
};

struct RoomResourceExit1 {
	int16 xs, xe, ys, ye;
	uint16 sequenceOffset;
};

struct RoomResourceExit2 {
	uint8 newRoom;
	uint8 direction;
	int16 newRoomX, newRoomY;
};

struct HotspotOverride {
	uint16 hotspotId;
	uint16 xs, xe, ys, ye;
};

struct AnimRecord {
	uint16 animId;
	uint8 flags;
	uint8 unused[6];
	uint16 upOffset;
	uint16 downOffset;
	uint16 leftOffset;
	uint16 rightOffset;
	uint8 upFrame;
	uint8 downFrame;
	uint8 leftFrame;
	uint8 rightFrame;
};

struct AnimRecordOutput {
	uint16 animRecordId;
	uint16 animId;
	uint16 flags;
	uint16 upOffset;
	uint16 downOffset;
	uint16 leftOffset;
	uint16 rightOffset;
	uint8 upFrame;
	uint8 downFrame;
	uint8 leftFrame;
	uint8 rightFrame;
};

struct MovementRecord {
	uint16 frameNumber;
	int16 xChange;
	int16 yChange;
};

struct RoomExitHotspotRecord {
	int16 xs, xe;
	int16 ys, ye;
	uint16 cursorNum;
	uint16 hotspotId;
	uint16 destRoomNumber;
};

struct RoomExitHotspotOutputRecord {
	uint16 hotspotId;
	int16 xs, xe;
	int16 ys, ye;
	uint16 cursorNum;
	uint16 destRoomNumber;
};

struct RoomExitHotspotJoinRecord {
	uint16 hotspot1Id;
	byte h1CurrentFrame;
	byte h1DestFrame;
	byte h1OpenSound;
	byte h1CloseSound;
	uint16 hotspot2Id;
	byte h2CurrentFrame;
	byte h2DestFrame;
	byte h2OpenSound;
	byte h2CloseSound;
	byte blocked;
};

struct HotspotActionSequenceRecord {
	byte actionNumber;
	uint16 sequenceOffset;
};

struct HotspotActionsRecord {
	uint16 recordId;
	uint16 offset;
};

struct RoomExitCoordinateResource {
	int16 x;
	int16 y;
	uint16 roomNumber;
};

struct HotspotWalkToRecord {
	uint16 hotspotId;
	int16 x;
	uint16 y;
};

struct RoomExitIndexedHotspotResource {
	uint8 roomNumber;
	uint8 hotspotIndex;
	uint16 hotspotId;
};


#define ROOM_EXIT_COORDINATES_NUM_ENTRIES 6
#define ROOM_EXIT_COORDINATES_ENTRY_NUM_ROOMS 52

struct RoomExitCoordinateEntryResource {
	RoomExitCoordinateResource entries[ROOM_EXIT_COORDINATES_NUM_ENTRIES];
	uint8 roomIndex[ROOM_EXIT_COORDINATES_ENTRY_NUM_ROOMS];
};

enum CurrentAction {NO_ACTION, START_WALKING, DISPATCH_ACTION, EXEC_HOTSPOT_SCRIPT, 
	PROCESSING_PATH, WALKING};

extern void read_action_sequence(byte *&data, uint16 &totalSize);

extern uint16 get_sequence_index(uint16 offset, int supportIndex = -1);

enum AccessMode {
	kFileReadMode = 1,
	kFileWriteMode = 2
};

class File {
private:
	FILE *f;
public:
	bool open(const char *filename, AccessMode mode = kFileReadMode) {
		f = fopen(filename, (mode == kFileReadMode) ? "rb" : "wb");
		return (f != NULL);
	}
	void close() {
		fclose(f);
		f = NULL;
	}
	int seek(int32 offset, int whence = SEEK_SET) {
		return fseek(f, offset, whence);
	}
	long read(void *buffer, int len) {
		return fread(buffer, 1, len, f);
	}
	void write(const void *buffer, int len) {
		fwrite(buffer, 1, len, f);
	}
	byte readByte() { 
		byte v;
		read(&v, sizeof(byte));
		return v;
	}
	uint16 readWord() {
		uint16 v;
		read(&v, sizeof(uint16));
		return FROM_LE_16(v);
	}
	uint16 readLong() {
		uint32 v;
		read(&v, sizeof(uint32));
		return FROM_LE_32(v);
	}
	void writeByte(byte v) { 
		write(&v, sizeof(byte));
	}
	void writeWord(uint16 v) { 
		uint16 vTemp = TO_LE_16(v);
		write(&vTemp, sizeof(uint16));
	}
	void writeLong(uint32 v) { 
		uint32 vTemp = TO_LE_32(v);
		write(&vTemp, sizeof(uint32));
	}
};

extern File lure_exe;
extern void add_talk_offset(uint16 offset);

#endif
