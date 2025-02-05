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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/engines/scumm/smush/smush_player.h $
 * $Id: smush_player.h 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

#if !defined(SCUMM_SMUSH_PLAYER_H) && !defined(DISABLE_SCUMM_7_8)
#define SCUMM_SMUSH_PLAYER_H

#include "common/util.h"
#include "scumm/smush/chunk.h"
#include "scumm/sound.h"

namespace Scumm {

class ScummEngine_v7;
class SmushFont;
class SmushMixer;
class StringResource;
class Codec37Decoder;
class Codec47Decoder;

class SmushPlayer {
	friend class Insane;
private:
	ScummEngine_v7 *_vm;
	int32 _nbframes;
	SmushMixer *_smixer;
	int16 _deltaPal[0x300];
	byte _pal[0x300];
	SmushFont *_sf[5];
	StringResource *_strings;
	Codec37Decoder *_codec37;
	Codec47Decoder *_codec47;
	FileChunk *_base;
	byte *_frameBuffer;
	byte *_specialBuffer;

	Common::String _seekFile;
	uint32 _startFrame;
	uint32 _startTime;
	int32 _seekPos;
	uint32 _seekFrame;

	bool _skipNext;
	uint32 _frame;

	Audio::SoundHandle _IACTchannel;
	Audio::AppendableAudioStream *_IACTstream;

	Audio::SoundHandle _compressedFileSoundHandle;
	bool _compressedFileMode;
	byte _IACToutput[4096];
	int32 _IACTpos;
	bool _storeFrame;
	int _speed;
	bool _endOfFile;

	byte *_dst;
	bool _updateNeeded;
	bool _warpNeeded;
	int _palDirtyMin, _palDirtyMax;
	int _warpX, _warpY;
	int _warpButtons;
	bool _insanity;
	bool _middleAudio;
	bool _skipPalette;
#ifndef DISABLE_SMUSHREDRAW		// cybercop
#ifdef _WIN32_WCE
	bool _inTimer;
	int16 _inTimerCount;
	int16 _inTimerCountRedraw;
#endif
#endif

public:
	SmushPlayer(ScummEngine_v7 *scumm);
	~SmushPlayer();

	void pause();
	void unpause();

	void play(const char *filename, int32 speed, int32 offset = 0, int32 startFrame = 0);
	void release();
	void warpMouse(int x, int y, int buttons);

protected:
	int _width, _height;

	int _origPitch, _origNumStrips;
	bool _paused;
	uint32 _pauseStartTime;
	uint32 _pauseTime;

	void insanity(bool);
	void setPalette(const byte *palette);
	void setPaletteValue(int n, byte r, byte g, byte b);
	void setDirtyColors(int min, int max);
	void seekSan(const char *file, int32 pos, int32 contFrame);
	const char *getString(int id);

private:
	SmushFont *getFont(int font);
	void parseNextFrame();
	void init(int32 spped);
	void setupAnim(const char *file);
	void updateScreen();
	void tryCmpFile(const char *filename);

	bool readString(const char *file);
	void decodeFrameObject(int codec, const uint8 *src, int left, int top, int width, int height);
	void checkBlock(const Chunk &, Chunk::type, uint32 = 0);
	void handleAnimHeader(Chunk &);
	void handleFrame(Chunk &);
	void handleNewPalette(Chunk &);
#ifdef USE_ZLIB
	void handleZlibFrameObject(Chunk &b);
#endif
	void handleFrameObject(Chunk &);
	void handleSoundBuffer(int32, int32, int32, int32, int32, int32, Chunk &, int32);
	void handleSoundFrame(Chunk &);
	void handleStore(Chunk &);
	void handleFetch(Chunk &);
	void handleIACT(Chunk &);
	void handleTextResource(Chunk &);
	void handleDeltaPalette(Chunk &);
	void readPalette(byte *, Chunk &);

	void timerCallback();
};

} // End of namespace Scumm

#endif
