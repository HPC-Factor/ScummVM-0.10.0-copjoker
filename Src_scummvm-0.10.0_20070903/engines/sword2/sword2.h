/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1994-1998 Revolution Software Ltd.
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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/engines/sword2/sword2.h $
 * $Id: sword2.h 27030 2007-05-31 20:28:29Z fingolfin $
 */

#ifndef	SWORD2_H
#define	SWORD2_H

#define FRAMES_PER_SECOND 12

// Enable this to make it possible to clear the mouse cursor luggage by
// right-clicking. The original didn't do this, but it feels natural to me.
// However, I'm afraid that it'll interfer badly with parts of the game, so
// for now I'll keep it disabled.

#define RIGHT_CLICK_CLEARS_LUGGAGE 0

#include "engines/engine.h"

#include "common/util.h"

#define	MAX_starts	100
#define	MAX_description	100

class OSystem;

namespace Sword2 {

enum {
	GF_DEMO	= 1 << 0
};

class MemoryManager;
class ResourceManager;
class Sound;
class Screen;
class Mouse;
class Logic;
class FontRenderer;
class Gui;
class Debugger;

enum {
	RD_LEFTBUTTONDOWN		= 0x01,
	RD_LEFTBUTTONUP			= 0x02,
	RD_RIGHTBUTTONDOWN		= 0x04,
	RD_RIGHTBUTTONUP		= 0x08,
	RD_WHEELUP			= 0x10,
	RD_WHEELDOWN			= 0x20,
	RD_KEYDOWN			= 0x40
};

struct MouseEvent {
	bool pending;
	uint16 buttons;
};

struct KeyboardEvent {
	bool pending;
	uint16 ascii;
	int keycode;
	int modifiers;
};

struct StartUp {
	char description[MAX_description];

	// id of screen manager object
	uint32 start_res_id;

	// Tell the manager which startup you want (if there are more than 1)
	// (i.e more than 1 entrance to a screen and/or separate game boots)
	uint32 key;
};

class Sword2Engine : public Engine {
private:
	uint32 _inputEventFilter;

	// The event "buffers"
	MouseEvent _mouseEvent;
	KeyboardEvent _keyboardEvent;

	uint32 _bootParam;
	int32 _saveSlot;

	void getPlayerStructures();
	void putPlayerStructures();

	uint32 saveData(uint16 slotNo, byte *buffer, uint32 bufferSize);
	uint32 restoreData(uint16 slotNo, byte *buffer, uint32 bufferSize);

	uint32 calcChecksum(byte *buffer, uint32 size);

	void pauseGame();
	void unpauseGame();

	uint32 _totalStartups;
	uint32 _totalScreenManagers;
	uint32 _startRes;

	bool _useSubtitles;
	int _gameSpeed;

	StartUp _startList[MAX_starts];

public:
	Sword2Engine(OSystem *syst);
	~Sword2Engine();
	int go();
	int init();

	int getFramesPerSecond();

	void registerDefaultSettings();
	void readSettings();
	void writeSettings();

	void setupPersistentResources();

	bool getSubtitles() { return _useSubtitles; }
	void setSubtitles(bool b) { _useSubtitles = b; }

	bool _quit;

	uint32 _features;

	MemoryManager *_memory;
	ResourceManager	*_resman;
	Sound *_sound;
	Screen *_screen;
	Mouse *_mouse;
	Logic *_logic;
	FontRenderer *_fontRenderer;

	Debugger *_debugger;

	Common::RandomSource _rnd;

	uint32 _speechFontId;
	uint32 _controlsFontId;
	uint32 _redFontId;

	uint32 setInputEventFilter(uint32 filter);

	void parseInputEvents();

	bool checkForMouseEvents();
	MouseEvent *mouseEvent();
	KeyboardEvent *keyboardEvent();

	bool _wantSfxDebug;

	int32 _gameCycle;

#ifdef SWORD2_DEBUG
	bool _renderSkip;
	bool _stepOneCycle;
#endif

#if RIGHT_CLICK_CLEARS_LUGGAGE
	bool heldIsInInventory();
#endif

	byte *fetchPalette(byte *screenFile);
	byte *fetchScreenHeader(byte *screenFile);
	byte *fetchLayerHeader(byte *screenFile, uint16 layerNo);
	byte *fetchShadingMask(byte *screenFile);

	byte *fetchAnimHeader(byte *animFile);
	byte *fetchCdtEntry(byte *animFile, uint16 frameNo);
	byte *fetchFrameHeader(byte *animFile, uint16 frameNo);
	byte *fetchBackgroundParallaxLayer(byte *screenFile, int layer);
	byte *fetchBackgroundLayer(byte *screenFile);
	byte *fetchForegroundParallaxLayer(byte *screenFile, int layer);
	byte *fetchTextLine(byte *file, uint32 text_line);
	bool checkTextLine(byte *file, uint32 text_line);
	byte *fetchPaletteMatchTable(byte *screenFile);

	uint32 saveGame(uint16 slotNo, byte *description);
	uint32 restoreGame(uint16 slotNo);
	uint32 getSaveDescription(uint16 slotNo, byte *description);
	bool saveExists();
	bool saveExists(uint16 slotNo);
	uint32 restoreFromBuffer(byte *buffer, uint32 size);
	char *getSaveFileName(uint16 slotNo);
	uint32 findBufferSize();

	bool _gamePaused;
	bool _graphicsLevelFudged;

	void startGame();
	void gameCycle();
	void closeGame();
	void restartGame();

	void sleepUntil(uint32 time);

	GUI::Debugger *getDebugger();
	void initialiseFontResourceFlags();
	void initialiseFontResourceFlags(uint8 language);

	bool initStartMenu();
	void registerStartPoint(int32 key, char *name);

	uint32 getNumStarts() { return _totalStartups; }
	uint32 getNumScreenManagers() { return _totalScreenManagers; }
	StartUp *getStartList() { return _startList; }

	void runStart(int start);

	// Convenience alias for OSystem::getMillis().
	// This is a bit hackish, of course :-).
	uint32 getMillis();
};

} // End of namespace Sword2

#endif
