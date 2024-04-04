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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/engines/scumm/imuse_digi/dimuse_sndmgr.h $
 * $Id: dimuse_sndmgr.h 27039 2007-06-01 11:22:49Z aquadran $
 */

#ifndef SCUMM_IMUSE_DIGI_SNDMGR_H
#define SCUMM_IMUSE_DIGI_SNDMGR_H

#include "common/stdafx.h"
#include "common/scummsys.h"
#include "sound/audiostream.h"
#include "scumm/imuse_digi/dimuse_bndmgr.h"

namespace Scumm {

class ScummEngine;
class BundleMgr;

class ImuseDigiSndMgr {
public:

#define MAX_IMUSE_SOUNDS 16

#define IMUSE_RESOURCE 1
#define IMUSE_BUNDLE 2

#define IMUSE_VOLGRP_VOICE 1
#define IMUSE_VOLGRP_SFX 2
#define IMUSE_VOLGRP_MUSIC 3

private:
	struct _region {
		int32 offset;		// offset of region
		int32 length;		// lenght of region
	};

	struct _jump {
		int32 offset;		// jump offset position
		int32 dest;			// jump to dest position
		byte hookId;		// id of hook
		int16 fadeDelay;	// fade delay in ms
	};

	struct _sync {
		int32 size;			// size of sync
		byte *ptr;			// pointer to sync
	};

public:

	struct soundStruct {
		uint16 freq;		// frequency
		byte channels;		// stereo or mono
		byte bits;			// 8, 12, 16
		int numJumps;		// number of Jumps
		int numRegions;		// number of Regions
		int numSyncs;		// number of Syncs
		_region *region;
		_jump *jump;
		_sync *sync;
		bool endFlag;
		bool inUse;
		byte *allData;
		int32 offsetData;
		byte *resPtr;
		char name[15];
		int16 soundId;
		BundleMgr *bundle;
		int type;
		int volGroupId;
		int disk;
		Audio::AudioStream *compressedStream;
		bool compressed;
		char lastFileName[24];
	};

private:

	soundStruct _sounds[MAX_IMUSE_SOUNDS];

	bool checkForProperHandle(soundStruct *soundHandle);
	soundStruct *allocSlot();
	void prepareSound(byte *ptr, soundStruct *sound);
	void prepareSoundFromRMAP(Common::File *file, soundStruct *sound, int32 offset, int32 size);

	ScummEngine *_vm;
	byte _disk;
	BundleDirCache *_cacheBundleDir;

	bool openMusicBundle(soundStruct *sound, int disk);
	bool openVoiceBundle(soundStruct *sound, int disk);

	void countElements(byte *ptr, int &numRegions, int &numJumps, int &numSyncs);

public:

	ImuseDigiSndMgr(ScummEngine *scumm);
	~ImuseDigiSndMgr();

	soundStruct *openSound(int32 soundId, const char *soundName, int soundType, int volGroupId, int disk);
	void closeSound(soundStruct *soundHandle);
	soundStruct *cloneSound(soundStruct *soundHandle);

	bool isSndDataExtComp(soundStruct *soundHandle);
	int getFreq(soundStruct *soundHandle);
	int getBits(soundStruct *soundHandle);
	int getChannels(soundStruct *soundHandle);
	bool isEndOfRegion(soundStruct *soundHandle, int region);
	int getNumRegions(soundStruct *soundHandle);
	int getNumJumps(soundStruct *soundHandle);
	int getRegionOffset(soundStruct *soundHandle, int region);
	int getJumpIdByRegionAndHookId(soundStruct *soundHandle, int region, int hookId);
	int getRegionIdByJumpId(soundStruct *soundHandle, int jumpId);
	int getJumpHookId(soundStruct *soundHandle, int number);
	int getJumpFade(soundStruct *soundHandle, int number);
	void getSyncSizeAndPtrById(soundStruct *soundHandle, int number, int32 &sync_size, byte **sync_ptr);

	int32 getDataFromRegion(soundStruct *soundHandle, int region, byte **buf, int32 offset, int32 size);
};

} // End of namespace Scumm

#endif
