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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/engines/scumm/scumm.h $
 * $Id: scumm.h 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

#ifndef SCUMM_H
#define SCUMM_H

#include "engines/engine.h"
#include "common/endian.h"
#include "common/file.h"
#include "common/rect.h"
#include "common/str.h"
#include "graphics/surface.h"

#include "scumm/gfx.h"
#include "scumm/detection.h"
#include "scumm/script.h"

#include "sound/mididrv.h"

namespace GUI {
	class Dialog;
}
using GUI::Dialog;
namespace Common {
	class InSaveFile;
	class OutSaveFile;
}

namespace Scumm {

class Actor;
class BaseCostumeLoader;
class BaseCostumeRenderer;
class BaseScummFile;
class CharsetRenderer;
class IMuse;
class IMuseDigital;
class MusicEngine;
class ScummEngine;
class ScummDebugger;
class Serializer;
class Sound;

struct Box;
struct BoxCoords;
struct FindObjectInRoom;

// Use g_scumm from error() ONLY
extern ScummEngine *g_scumm;

/* System Wide Constants */
enum {
	NUM_SENTENCE = 6,
	NUM_SHADOW_PALETTE = 8
};

/**
 * SCUMM feature flags define for every game which specific set of engine
 * features are used by that game.
 * Note that some of them could be replaced by checks for the SCUMM version.
 */
enum GameFeatures {
	/** Games with the AKOS costume system (ScummEngine_v7 and subclasses, HE games). */
	GF_NEW_COSTUMES        = 1 << 2,

	/** Games using XOR encrypted data files. */
	GF_USE_KEY             = 1 << 4,

	/** Small header games (ScummEngine_v4 and subclasses). */
	GF_SMALL_HEADER        = 1 << 5,

	/** Old bundle games (ScummEngine_v3old and subclasses). */
	GF_OLD_BUNDLE          = 1 << 6,

	/** EGA games. */
	GF_16COLOR             = 1 << 7,

	/** VGA versions of V3 games.  Equivalent to (version == 3 && not GF_16COLOR) */
	GF_OLD256              = 1 << 8,

	/** Games which have Audio CD tracks. */
	GF_AUDIOTRACKS         = 1 << 9,

	/**
	 * Games using only very few local variables in scripts.
	 * Apparently that is only the case for 256 color version of Indy3.
	 */
	GF_FEW_LOCALS          = 1 << 11,

	/** HE games for which localized versions exist */
	GF_HE_LOCALIZED        = 1 << 13,

	/**
	 *  HE Games with more global scripts and different sprite handling
	 *  i.e. read it as HE version 9.85. Used for HE98 only.
	 */
	GF_HE_985             = 1 << 14,

	/** HE games with 16 bit color */
	GF_16BIT_COLOR         = 1 << 15,

	/** HE games which use sprites for subtitles */
	GF_HE_NOSUBTITLES      = 1 << 16,

	/** A demo, not a full blown game. */
	GF_DEMO                = 1 << 17
};

/* SCUMM Debug Channels */
void CDECL debugC(int level, const char *s, ...);

enum {
	DEBUG_GENERAL	=	1 << 0,		// General debug
	DEBUG_SCRIPTS	=	1 << 2,		// Track script execution (start/stop/pause)
	DEBUG_OPCODES	=	1 << 3,		// Track opcode invocations
	DEBUG_VARS	=	1 << 4,		// Track variable changes
	DEBUG_RESOURCE	=	1 << 5,		// Track resource loading / allocation
	DEBUG_IMUSE	=	1 << 6,		// Track iMUSE events
	DEBUG_SOUND	=	1 << 7,		// General Sound Debug
	DEBUG_ACTORS	=	1 << 8,		// General Actor Debug
	DEBUG_INSANE	=	1 << 9,		// Track INSANE
	DEBUG_SMUSH	=	1 << 10		// Track SMUSH
};

/**
 * Internal header for any memory block allocated by the resource manager.
 *
 * @todo Hide MemBlkHeader; no code outside the resource manager should
 * have to use it, ever. Currently script code needs it to detect whether
 * some scripts have moved (in fetchScriptByte()).
 */
struct MemBlkHeader {
	uint32 size;
};

struct VerbSlot;
struct ObjectData;

enum {
	/**
	 * Lighting flag that indicates whether the normal palette, or the 'dark'
	 * palette shall be used to draw actors.
	 * Apparantly only used in very old games (so far only NESCostumeRenderer
	 * checks it).
	 */
	LIGHTMODE_actor_use_base_palette	= 1 << 0,

	/**
	 * Lighting flag that indicates whether the room is currently lit. Normally
	 * always on. Used for rooms in which the light can be switched "off".
	 */
	LIGHTMODE_room_lights_on			= 1 << 1,

	/**
	 * Lighting flag that indicates whether a flashlight like device is active.
	 * Used in Loom (flashlight follows the actor) and Indy 3 (flashlight
	 * follows the mouse). Only has any effect if the room lights are off.
	 */
	LIGHTMODE_flashlight_on				= 1 << 2,

	/**
	 * Lighting flag that indicates whether actors are to be drawn with their
	 * own custom palette, or using a fixed 'dark' palette. This is the
	 * modern successor of LIGHTMODE_actor_use_base_palette.
	 * Note: It is tempting to 'merge' these two flags, but since flags can
	 * check their values, this is probably not a good idea.
	 */
	LIGHTMODE_actor_use_colors	= 1 << 3
	//
};

enum {
	MBS_LEFT_CLICK = 0x8000,
	MBS_RIGHT_CLICK = 0x4000,
	MBS_MOUSE_MASK = (MBS_LEFT_CLICK | MBS_RIGHT_CLICK),
	MBS_MAX_KEY	= 0x0200
};

enum ScummGameId {
	GID_CMI,
	GID_DIG,
	GID_FT,
	GID_INDY3,
	GID_INDY4,
	GID_LOOM,
	GID_MANIAC,
	GID_MONKEY_EGA,
	GID_MONKEY_VGA,
	GID_MONKEY,
	GID_MONKEY2,
	GID_PASS,
	GID_SAMNMAX,
	GID_TENTACLE,
	GID_ZAK,

	GID_HEGAME,      // Generic name for all HE games with default behaviour
	GID_PUTTDEMO,
	GID_FBEAR,
	GID_FUNPACK,
	GID_WATER,
	GID_PUTTRACE,
	GID_FUNSHOP,	// Used for all three funshops
	GID_FOOTBALL,
	GID_SOCCER,
	GID_HECUP		// CUP demos
};

struct SentenceTab {
	byte verb;
	byte preposition;
	uint16 objectA;
	uint16 objectB;
	uint8 freezeCount;
};

struct StringSlot {
	int16 xpos;
	int16 ypos;
	int16 right;
	int16 height;
	byte color;
	byte charset;
	bool center;
	bool overhead;
	bool no_talk_anim;
	bool wrapping;
};

struct StringTab : StringSlot {
	// The 'default' values for this string slot. This is used so that the
	// string slot can temporarily be set to different values, and then be
	// easily reset to a previously set default.
	StringSlot _default;

	void saveDefault() {
		StringSlot &s = *this;
		_default = s;
	}

	void loadDefault() {
		StringSlot &s = *this;
		s = _default;
	}
};



enum WhereIsObject {
	WIO_NOT_FOUND = -1,
	WIO_INVENTORY = 0,
	WIO_ROOM = 1,
	WIO_GLOBAL = 2,
	WIO_LOCAL = 3,
	WIO_FLOBJECT = 4
};

struct AuxBlock {
	bool visible;
	Common::Rect r;

	void reset() {
		visible = false;
		r.left = r.top = 0;
		r.right = r.bottom = -1;
	}
};

struct AuxEntry {
	int actorNum;
	int subIndex;
};

// TODO: Rename InfoStuff to something more descriptive
struct InfoStuff {
	uint32 date;
	uint16 time;
	uint32 playtime;
};

/**
 * A list of resource types.
 * WARNING: Do not change the order of these, as the savegame format relies
 * on it; any change made here will break savegame compatibility!
 */
enum ResTypes {
	rtFirst = 1,
	rtRoom = 1,
	rtScript = 2,
	rtCostume = 3,
	rtSound = 4,
	rtInventory = 5,
	rtCharset = 6,
	rtString = 7,
	rtVerb = 8,
	rtActorName = 9,
	rtBuffer = 10,
	rtScaleTable = 11,
	rtTemp = 12,
	rtFlObject = 13,
	rtMatrix = 14,
	rtBox = 15,
	rtObjectName = 16,
	rtRoomScripts = 17,
	rtRoomImage = 18,
	rtImage = 19,
	rtTalkie = 20,
	rtSpoolBuffer = 21,
	rtLast = 21,
	rtNumTypes = 22
};

enum {
	RES_INVALID_OFFSET = 0xFFFFFFFF
};

/**
 * The 'resource manager' class. Currently doesn't really deserve to be called
 * a 'class', at least until somebody gets around to OOfying this more.
 */
class ResourceManager {
	//friend class ScummDebugger;
	//friend class ScummEngine;
protected:
	ScummEngine *_vm;

public:
	byte mode[rtNumTypes];
	uint16 num[rtNumTypes];
	uint32 tags[rtNumTypes];
	const char *name[rtNumTypes];
	byte **address[rtNumTypes];
protected:
	byte *flags[rtNumTypes];
	byte *status[rtNumTypes];
public:
	byte *roomno[rtNumTypes];
	uint32 *roomoffs[rtNumTypes];
	uint32 *globsize[rtNumTypes];

protected:
	uint32 _allocatedSize;
	uint32 _maxHeapThreshold, _minHeapThreshold;
	byte _expireCounter;

public:
	ResourceManager(ScummEngine *vm);
	~ResourceManager();

	void setHeapThreshold(int min, int max);

	void allocResTypeData(int id, uint32 tag, int num, const char *name, int mode);
	void freeResources();

	byte *createResource(int type, int index, uint32 size);
	void nukeResource(int type, int i);

	bool isResourceLoaded(int type, int index) const;

	void lock(int type, int i);
	void unlock(int type, int i);
	bool isLocked(int type, int i) const;

	void setModified(int type, int i);
	bool isModified(int type, int i) const;

	void increaseExpireCounter();
	void setResourceCounter(int type, int index, byte flag);
	void increaseResourceCounter();

	void resourceStats();
	
//protected:
	bool validateResource(const char *str, int type, int index) const;
protected:
	void expireResources(uint32 size);
};

/**
 * Base class for all SCUMM engines.
 */
class ScummEngine : public Engine {
	friend class ScummDebugger;
	friend class CharsetRenderer;
	friend class ResourceManager;

	GUI::Debugger *getDebugger();
	void errorString(const char *buf_input, char *buf_output);

public:
	/* Put often used variables at the top.
	 * That results in a shorter form of the opcode
	 * on some architectures. */
	IMuse *_imuse;
	IMuseDigital *_imuseDigital;
	MusicEngine *_musicEngine;
	Sound *_sound;

	VerbSlot *_verbs;
	ObjectData *_objs;
	ScummDebugger *_debugger;

	// Core variables
	GameSettings _game;
	uint8 _gameMD5[16];

	/** Random number generator */
	Common::RandomSource _rnd;

	/** Graphics manager */
	Gdi *_gdi;

	/** Central resource data. */
	ResourceManager *_res;

protected:
	VirtualMachineState vm;

public:
	// Constructor / Destructor
	ScummEngine(OSystem *syst, const DetectorResult &dr);
	virtual ~ScummEngine();

	/** Startup function, main loop. */
	int go();

	// Init functions
	int init();

protected:
	virtual void setupScumm();
	virtual void resetScumm();

	virtual void setupScummVars();
	virtual void resetScummVars();

	void setupCharsetRenderer();
	void setupCostumeRenderer();

	virtual void loadLanguageBundle() {}
	void loadCJKFont();
	void setupMusic(int midi);
	void updateSoundSettings();
	void setTalkspeed(int talkspeed);
	int getTalkspeed();

	// Scumm main loop & helper functions.
	virtual int scummLoop(int delta);
	virtual void scummLoop_updateScummVars();
	virtual void scummLoop_handleSaveLoad();
	virtual void scummLoop_handleDrawing();
	virtual void scummLoop_handleActors() = 0;
	virtual void scummLoop_handleEffects();
	virtual void scummLoop_handleSound();
	
	virtual void runBootscript();

	// Event handling
public:
	void parseEvents();	// Used by IMuseDigital::startSound
protected:
	void waitForTimer(int msec_delay);
	virtual void processInput();
	virtual void processKeyboard(int lastKeyHit);
	virtual void clearClickedStatus();

	// Cursor/palette
	void updateCursor();
	virtual void animateCursor() {}
	virtual void updatePalette();

public:
	void pauseGame();
	void restart();
	void shutDown();

	/** We keep running until this is set to true. */
	bool _quit;

protected:
	Dialog *_pauseDialog;
	Dialog *_versionDialog;
	Dialog *_mainMenuDialog;

	virtual int runDialog(Dialog &dialog);
	void confirmExitDialog();
	void confirmRestartDialog();
	void pauseDialog();
	void versionDialog();
	void mainMenuDialog();

	char displayMessage(const char *altButton, const char *message, ...);

	byte _fastMode;

	byte _numActors;
	Actor **_actors;	// Has _numActors elements
	Actor **_sortedActors;

	byte *_arraySlot;
	uint16 *_inventory;
	uint16 *_newNames;
public:
	// VAR is a wrapper around scummVar, which attempts to include additional
	// useful information should an illegal var access be detected.
	#define VAR(x)	scummVar(x, #x, __FILE__, __LINE__)
	int32& scummVar(byte var, const char *varName, const char *file, int line)
	{
		if (var == 0xFF) {
			error("Illegal access to variable %s in file %s, line %d", varName, file, line);
		}
		return _scummVars[var];
	}
	int32 scummVar(byte var, const char *varName, const char *file, int line) const
	{
		if (var == 0xFF) {
			error("Illegal access to variable %s in file %s, line %d", varName, file, line);
		}
		return _scummVars[var];
	}

protected:
	int16 _varwatch;
	int32 *_roomVars;
	int32 *_scummVars;
	byte *_bitVars;

	/* Global resource tables */
	int _numVariables, _numBitVariables, _numLocalObjects;
	int _numGlobalObjects, _numArray, _numVerbs, _numFlObject;
	int _numInventory;
	int _numNewNames, _numGlobalScripts;
	int _numRoomVariables;
	int _numPalettes, _numSprites, _numTalkies, _numUnk;
	int _HEHeapSize;
public:
	int _numLocalScripts, _numImages, _numRooms, _numScripts, _numSounds;	// Used by HE games
	int _numCostumes;	// FIXME - should be protected, used by Actor::remapActorPalette
	int _numCharsets;	// FIXME - should be protected, used by CharsetRenderer

	BaseCostumeLoader *_costumeLoader;
	BaseCostumeRenderer *_costumeRenderer;

	int _NESCostumeSet;
	void NES_loadCostumeSet(int n);
	byte *_NEScostdesc, *_NEScostlens, *_NEScostoffs, *_NEScostdata;
	byte _NESPatTable[2][4096];
	byte _NESPalette[2][16];
	byte _NESBaseTiles;

	int _NESStartStrip;

protected:
	int _curPalIndex;

public:
	byte _currentRoom;	// FIXME - should be protected but Actor::isInCurrentRoom uses it
	int _roomResource;  // FIXME - should be protected but Sound::pauseSounds uses it
	bool _egoPositioned;	// Used by Actor::putActor, hence public

	FilenamePattern _filenamePattern;

	Common::String generateFilename(const int room) const;

protected:
	int _keyPressed;
	bool _keyDownMap[512]; // FIXME - 512 is a guess. it's max(kbd.ascii)

	Common::Point _mouse;
	Common::Point _virtualMouse;

	uint16 _mouseAndKeyboardStat;
	byte _leftBtnPressed, _rightBtnPressed;

	/** The bootparam, to be passed to the script 1, the bootscript. */
	int _bootParam;

	// Various options useful for debugging
	bool _dumpScripts;
	bool _hexdumpScripts;
	bool _showStack;
	uint16 _debugMode;

	// Save/Load class - some of this may be GUI
	byte _saveLoadFlag, _saveLoadSlot;
	uint32 _lastSaveTime;
	bool _saveTemporaryState;
	char _saveLoadName[32];

	bool saveState(int slot, bool compat);
	bool loadState(int slot, bool compat);
	virtual void saveOrLoad(Serializer *s);
	void saveLoadResource(Serializer *ser, int type, int index);	// "Obsolete"
	void saveResource(Serializer *ser, int type, int index);
	void loadResource(Serializer *ser, int type, int index);
	void makeSavegameName(char *out, int slot, bool temporary);

	int getKeyState(int key);

public:
	bool getSavegameName(int slot, char *desc);
	void listSavegames(bool *marks, int num);

	void requestSave(int slot, const char *name, bool temporary = false);
	void requestLoad(int slot);

// thumbnail + info stuff
public:
	Graphics::Surface *loadThumbnailFromSlot(int slot);
	bool loadInfosFromSlot(int slot, InfoStuff *stuff);

protected:
	Graphics::Surface *loadThumbnail(Common::InSaveFile *file);
	bool loadInfos(Common::InSaveFile *file, InfoStuff *stuff);
	void saveThumbnail(Common::OutSaveFile *file);
	void saveInfos(Common::OutSaveFile* file);

	int32 _engineStartTime;
	int32 _dialogStartTime;

protected:
	/* Script VM - should be in Script class */
	uint32 _localScriptOffsets[1024];
	const byte *_scriptPointer, *_scriptOrgPointer;
	byte _opcode, _currentScript;
	const byte * const *_lastCodePtr;
	int _resultVarNumber, _scummStackPos;
	int _vmStack[150];

	virtual void setupOpcodes() = 0;
	virtual void executeOpcode(byte i) = 0;
	virtual const char *getOpcodeDesc(byte i) = 0;

	void initializeLocals(int slot, int *vars);
	int	getScriptSlot();

	void startScene(int room, Actor *a, int b);
	void startManiac();

public:
	void runScript(int script, bool freezeResistant, bool recursive, int *lvarptr, int cycle = 0);
	void stopScript(int script);
	void nukeArrays(byte scriptSlot);

protected:
	void runObjectScript(int script, int entry, bool freezeResistant, bool recursive, int *vars, int slot = -1, int cycle = 0);
	void runScriptNested(int script);
	void executeScript();
	void updateScriptPtr();
	virtual void runInventoryScript(int i);
	void inventoryScript();
	void checkAndRunSentenceScript();
	void runExitScript();
	void runEntryScript();
	void runAllScripts();
	void freezeScripts(int scr);
	void unfreezeScripts();

	bool isScriptInUse(int script) const;
	bool isRoomScriptRunning(int script) const;
	bool isScriptRunning(int script) const;

	void killAllScriptsExceptCurrent();
	void killScriptsAndResources();
	void decreaseScriptDelay(int amount);

	void stopObjectCode();
	void stopObjectScript(int script);

	void getScriptBaseAddress();
	void getScriptEntryPoint();
	int getVerbEntrypoint(int obj, int entry);

	byte fetchScriptByte();
	virtual uint fetchScriptWord();
	virtual int fetchScriptWordSigned();
	uint fetchScriptDWord();
	int fetchScriptDWordSigned();
	void ignoreScriptWord() { fetchScriptWord(); }
	void ignoreScriptByte() { fetchScriptByte(); }
	virtual void getResultPos();
	void setResult(int result);
	void push(int a);
	int pop();
	virtual int readVar(uint var);
	virtual void writeVar(uint var, int value);

	void beginCutscene(int *args);
	void endCutscene();
	void abortCutscene();
	void beginOverride();
	void endOverride();

	void copyScriptString(byte *dst);
	int resStrLen(const byte *src) const;
	void doSentence(int c, int b, int a);

	/* Should be in Resource class */
	BaseScummFile *_fileHandle;
	uint32 _fileOffset;
public:
	/** The name of the (macintosh/rescumm style) container file, if any. */
	Common::String _containerFile;

	bool openFile(BaseScummFile &file, const Common::String &filename, bool resourceFile = false);

protected:
	int _resourceHeaderSize;
	byte _resourceMapper[128];
	byte *_heV7DiskOffsets;
	uint32 *_heV7RoomIntOffsets;
	const byte *_resourceLastSearchBuf; // FIXME: need to put it to savefile?
	uint32 _resourceLastSearchSize;    // FIXME: need to put it to savefile?

	virtual void allocateArrays();
	void openRoom(int room);
	void closeRoom();
	void deleteRoomOffsets();
	virtual void readRoomsOffsets();
	void askForDisk(const char *filename, int disknum);	// TODO: Use Common::String
	bool openResourceFile(const Common::String &filename, byte encByte);	// TODO: Use Common::String

	void loadPtrToResource(int type, int i, const byte *ptr);
	virtual void readResTypeList(int id);
//	void allocResTypeData(int id, uint32 tag, int num, const char *name, int mode);
//	byte *createResource(int type, int index, uint32 size);
	int loadResource(int type, int i);
//	void nukeResource(int type, int i);
	int getResourceSize(int type, int idx);

public:
	byte *getResourceAddress(int type, int i);
	virtual byte *getStringAddress(int i);
	byte *getStringAddressVar(int i);
	void ensureResourceLoaded(int type, int i);
	int getResourceRoomNr(int type, int index);

protected:
	int readSoundResource(int type, int index);
	int readSoundResourceSmallHeader(int type, int index);
	bool isResourceInUse(int type, int i) const;

	virtual void setupRoomSubBlocks();
	virtual void resetRoomSubBlocks();

	virtual void clearRoomObjects();
	virtual void resetRoomObjects();
	virtual void resetRoomObject(ObjectData *od, const byte *room, const byte *searchptr = NULL);

	virtual void readArrayFromIndexFile();
	virtual void readMAXS(int blockSize) = 0;
	virtual void readGlobalObjects();
	virtual void readIndexFile();
	virtual void readIndexBlock(uint32 block, uint32 itemsize);
	virtual void loadCharset(int i);
	void nukeCharset(int i);

	int _lastLoadedRoom;
public:
	const byte *findResourceData(uint32 tag, const byte *ptr);
	const byte *findResource(uint32 tag, const byte *ptr);
	int getResourceDataSize(const byte *ptr) const;
	void dumpResource(const char *tag, int index, const byte *ptr, int length = -1);

public:
	/* Should be in Object class */
	byte OF_OWNER_ROOM;
	int getInventorySlot();
	int findInventory(int owner, int index);
	int getInventoryCount(int owner);

protected:
	byte *_objectOwnerTable, *_objectRoomTable, *_objectStateTable;
	int _numObjectsInRoom;

public:
	uint32 *_classData;

protected:
	void markObjectRectAsDirty(int obj);
	virtual void loadFlObject(uint object, uint room);
	void nukeFlObjects(int min, int max);
	int findFlObjectSlot();
	int findLocalObjectSlot();
	void addObjectToInventory(uint obj, uint room);
	void updateObjectStates();
public:
	bool getClass(int obj, int cls) const;		// Used in actor.cpp, hence public
protected:
	void putClass(int obj, int cls, bool set);
	int getState(int obj);
	void putState(int obj, int state);
	void setObjectState(int obj, int state, int x, int y);
	int getOwner(int obj) const;
	void putOwner(int obj, int owner);
	void setOwnerOf(int obj, int owner);
	void clearOwnerOf(int obj);
	int getObjectRoom(int obj) const;
	int getObjX(int obj);
	int getObjY(int obj);
	void getObjectXYPos(int object, int &x, int &y)	{ int dir; getObjectXYPos(object, x, y, dir); }
	void getObjectXYPos(int object, int &x, int &y, int &dir);
	int getObjOldDir(int obj);
	int getObjNewDir(int obj);
	int getObjectIndex(int object) const;
	int getObjectImageCount(int object);
	int whereIsObject(int object) const;
	int findObject(int x, int y);
	void findObjectInRoom(FindObjectInRoom *fo, byte findWhat, uint object, uint room);
public:
	int getObjectOrActorXY(int object, int &x, int &y);	// Used in actor.cpp, hence public
protected:
	int getObjActToObjActDist(int a, int b); // Not sure how to handle
	const byte *getObjOrActorName(int obj);		 // these three..
	void setObjectName(int obj);

	void addObjectToDrawQue(int object);
	void removeObjectFromDrawQue(int object);
	void clearDrawObjectQueue();
	void processDrawQue();

	virtual void clearDrawQueues();

	uint32 getOBCDOffs(int object) const;
	byte *getOBCDFromObject(int obj);
	const byte *getOBIMFromObjectData(const ObjectData &od);
	const byte *getObjectImage(const byte *ptr, int state);
	virtual int getObjectIdFromOBIM(const byte *obim);

protected:
	/* Should be in Verb class */
	uint16 _verbMouseOver;
	int _inventoryOffset;
	int8 _userPut;
	uint16 _userState;

	virtual void handleMouseOver(bool updateInventory);
	virtual void redrawVerbs();
	virtual void checkExecVerbs();

	void verbMouseOver(int verb);
	int findVerbAtPos(int x, int y) const;
	virtual void drawVerb(int verb, int mode);
	virtual void runInputScript(int a, int cmd, int mode);
	void restoreVerbBG(int verb);
	void drawVerbBitmap(int verb, int x, int y);
	int getVerbSlot(int id, int mode) const;
	void killVerb(int slot);
	void setVerbObject(uint room, uint object, uint verb);

public:
	bool isValidActor(int id) const;

	/* Should be in Actor class */
	Actor *derefActor(int id, const char *errmsg = 0) const;
	Actor *derefActorSafe(int id, const char *errmsg) const;

protected:
	void walkActors();
	void playActorSounds();
	void redrawAllActors();
	void setActorRedrawFlags();
	void putActors();
	void showActors();
	void resetV1ActorTalkColor();
	void resetActorBgs();
	virtual void processActors();
	void processUpperActors();
	virtual int getActorFromPos(int x, int y);

public:
	/* Actor talking stuff */
	byte _actorToPrintStrFor, _V1TalkingActor;
	int _sentenceNum;
	SentenceTab _sentence[NUM_SENTENCE];
	StringTab _string[6];
	byte _haveMsg;
	int16 _talkDelay;
	int _NES_lastTalkingActor;
	int _NES_talkColor;

	virtual void actorTalk(const byte *msg);
	void stopTalk();
	int getTalkingActor();		// Wrapper around VAR_TALK_ACTOR for V1 Maniac
	void setTalkingActor(int variable);

	// Generic costume code
	bool isCostumeInUse(int i) const;

	Common::Rect _actorClipOverride;

protected:
	/* Should be in Graphics class? */
	uint16 _screenB, _screenH;
public:
	int _roomHeight, _roomWidth;
	int _screenHeight, _screenWidth;
	VirtScreen virtscr[4];		// Virtual screen areas
	CameraData camera;			// 'Camera' - viewport

	int _screenStartStrip, _screenEndStrip;
	int _screenTop;

	Common::RenderMode _renderMode;

protected:
	ColorCycle _colorCycle[16];	// Palette cycles
	uint8 _colorUsedByCycle[256];

	uint32 _ENCD_offs, _EXCD_offs;
	uint32 _CLUT_offs, _EPAL_offs;
	uint32 _IM00_offs, _PALS_offs;

	//ender: fullscreen
	bool _fullRedraw, _bgNeedsRedraw;
	bool _screenEffectFlag, _completeScreenRedraw;
	bool _disableFadeInEffect;

	struct {
		int hotspotX, hotspotY, width, height;
		byte animate, animateIndex;
		int8 state;
	} _cursor;
	byte _grabbedCursor[8192];
	byte _currentCursor;

	byte _newEffect, _switchRoomEffect2, _switchRoomEffect;
	bool _doEffect;

public:
	bool isLightOn() const;

	byte _currentLights;
	int getCurrentLights() const;

protected:
	void initScreens(int b, int h);
	void initVirtScreen(VirtScreenNumber slot, int top, int width, int height, bool twobufs, bool scrollable);
	void initBGBuffers(int height);
	void initCycl(const byte *ptr);	// Color cycle

	void decodeNESBaseTiles();

	void drawObject(int obj, int arg);
	void drawRoomObjects(int arg);
	void drawRoomObject(int i, int arg);
	void drawBox(int x, int y, int x2, int y2, int color);

	void moveScreen(int dx, int dy, int height);

	void restoreBackground(Common::Rect rect, byte backcolor = 0);
	void redrawBGStrip(int start, int num);
	virtual void redrawBGAreas();

	void cameraMoved();
	void setCameraAtEx(int at);
	virtual void setCameraAt(int pos_x, int pos_y);
	virtual void setCameraFollows(Actor *a, bool setCamera = false);
	virtual void moveCamera();
	virtual void panCameraTo(int x, int y);
	void clampCameraPos(Common::Point *pt);
	void actorFollowCamera(int act);

	const byte *getPalettePtr(int palindex, int room);

	void setC64Palette();
	void setNESPalette();
	void setAmigaPalette();
	void setHercPalette();
	void setCGAPalette();
	void setEGAPalette();
	void setV1Palette();

	void resetPalette();

	void setCurrentPalette(int pal);
	void setRoomPalette(int pal, int room);
	virtual void setPaletteFromPtr(const byte *ptr, int numcolor = -1);
	virtual void setPalColor(int index, int r, int g, int b);
	void setDirtyColors(int min, int max);
	const byte *findPalInPals(const byte *pal, int index);
	void swapPalColors(int a, int b);
	virtual void copyPalColor(int dst, int src);
	void cyclePalette();
	void stopCycle(int i);
	virtual void palManipulateInit(int resID, int start, int end, int time);
	void palManipulate();
public:
	int remapPaletteColor(int r, int g, int b, int threshold);		// Used by Actor::remapActorPalette
protected:
	void moveMemInPalRes(int start, int end, byte direction);
	void setShadowPalette(int slot, int redScale, int greenScale, int blueScale, int startColor, int endColor);
	void setShadowPalette(int redScale, int greenScale, int blueScale, int startColor, int endColor, int start, int end);
	virtual void darkenPalette(int redScale, int greenScale, int blueScale, int startColor, int endColor);

	void setCursorFromBuffer(const byte *ptr, int width, int height, int pitch);

public:
	void markRectAsDirty(VirtScreenNumber virt, int left, int right, int top, int bottom, int dirtybit = 0);
	void markRectAsDirty(VirtScreenNumber virt, const Common::Rect& rect, int dirtybit = 0) {
		markRectAsDirty(virt, rect.left, rect.right, rect.top, rect.bottom, dirtybit);
	}
protected:
	// Screen rendering
	byte *_compositeBuf;
	byte *_herculesBuf;
	virtual void drawDirtyScreenParts();
	void updateDirtyScreen(VirtScreenNumber slot);
	void drawStripToScreen(VirtScreen *vs, int x, int w, int t, int b);
	void ditherCGA(byte *dst, int dstPitch, int x, int y, int width, int height) const;
	void ditherHerc(byte *src, byte *hercbuf, int srcPitch, int *x, int *y, int *width, int *height) const;

public:
	VirtScreen *findVirtScreen(int y);
	byte *getMaskBuffer(int x, int y, int z);

protected:
	void fadeIn(int effect);
	void fadeOut(int effect);
	void setScrollBuffer();

	void unkScreenEffect6();
	void transitionEffect(int a);
	void dissolveEffect(int width, int height);
	void scrollEffect(int dir);

protected:
	bool _shakeEnabled;
	uint _shakeFrame;
	void setShake(int mode);

	int _drawObjectQueNr;
	byte _drawObjectQue[200];

	/* For each of the 410 screen strips, gfxUsageBits contains a
	 * bitmask. The lower 80 bits each correspond to one actor and
	 * signify if any part of that actor is currently contained in
	 * that strip.
	 *
	 * If the leftmost bit is set, the strip (background) is dirty
	 * needs to be redrawn.
	 *
	 * The second leftmost bit is set by removeBlastObject() and
	 * restoreBackground(), but I'm not yet sure why.
	 */
	uint32 gfxUsageBits[410 * 3];

	void upgradeGfxUsageBits();
	void setGfxUsageBit(int strip, int bit);
	void clearGfxUsageBit(int strip, int bit);
	bool testGfxUsageBit(int strip, int bit);
	bool testGfxAnyUsageBits(int strip);
	bool testGfxOtherUsageBits(int strip, int bit);

public:
	byte _roomPalette[256];
	byte *_shadowPalette;
	bool _skipDrawObject;
	int _voiceMode;

	// HE specific
	byte _HEV7ActorPalette[256];
	uint8 *_hePalettes;

protected:
	int _shadowPaletteSize;
	byte _currentPalette[3 * 256];
	byte _darkenPalette[3 * 256];

	int _palDirtyMin, _palDirtyMax;

	byte _palManipStart, _palManipEnd;
	uint16 _palManipCounter;
	byte *_palManipPalette;
	byte *_palManipIntermediatePal;

	bool _haveActorSpeechMsg;
	bool _useTalkAnims;
	uint16 _defaultTalkDelay;
	int _tempMusic;
	int _saveSound;
	bool _native_mt32;
	bool _enable_gs;
	MidiDriverFlags _musicType;
	bool _copyProtection;

public:
	uint16 _extraBoxFlags[65];

	byte getNumBoxes();
	byte *getBoxMatrixBaseAddr();
	int getNextBox(byte from, byte to);

	void setBoxFlags(int box, int val);
	void setBoxScale(int box, int b);

	bool checkXYInBoxBounds(int box, int x, int y);

	BoxCoords getBoxCoordinates(int boxnum);

	byte getMaskFromBox(int box);
	Box *getBoxBaseAddr(int box);
	byte getBoxFlags(int box);
	int getBoxScale(int box);

	int getScale(int box, int x, int y);
	int getScaleFromSlot(int slot, int x, int y);

protected:
	// Scaling slots/items
	struct ScaleSlot {
		int x1, y1, scale1;
		int x2, y2, scale2;
	};
	ScaleSlot _scaleSlots[20];
	void setScaleSlot(int slot, int x1, int y1, int scale1, int x2, int y2, int scale2);
	void setBoxScaleSlot(int box, int slot);
	void convertScaleTableToScaleSlot(int slot);

	void createBoxMatrix();
	bool areBoxesNeighbours(int i, int j);

	/* String class */
public:
	CharsetRenderer *_charset;
	byte _charsetColorMap[16];

	/**
	 * All text is normally rendered into this overlay surface. Then later
	 * drawStripToScreen() composits it over the game graphics.
	 */
	Graphics::Surface _textSurface;

protected:
	byte _charsetColor;
	byte _charsetData[23][16];

	int _charsetBufPos;
	byte _charsetBuffer[512];

	bool _keepText;

	int _nextLeft, _nextTop;

	void restoreCharsetBg();
	void clearCharsetMask();
	void clearTextSurface();

	virtual void initCharset(int charset);

	virtual void printString(int m, const byte *msg);

	virtual bool handleNextCharsetCode(Actor *a, int *c);
	virtual void CHARSET_1();
	void drawString(int a, const byte *msg);
	void debugMessage(const byte *msg);
	void showMessageDialog(const byte *msg);

	virtual int convertMessageToString(const byte *msg, byte *dst, int dstSize);
	int convertIntMessage(byte *dst, int dstSize, int var);
	int convertVerbMessage(byte *dst, int dstSize, int var);
	int convertNameMessage(byte *dst, int dstSize, int var);
	int convertStringMessage(byte *dst, int dstSize, int var);

public:
	Common::Language _language;	// Accessed by a hack in NutRenderer::loadFont

	// Used by class ScummDialog:
	virtual void translateText(const byte *text, byte *trans_buff);

	// Somewhat hackish stuff for 2 byte support (Chinese/Japanese/Korean)
	bool _useCJKMode;
	int _2byteHeight;
	int _2byteWidth;
	byte *get2byteCharPtr(int idx);

protected:
	byte *_2byteFontPtr;

public:

	/* Scumm Vars */
	byte VAR_KEYPRESS;
	byte VAR_SYNC;
	byte VAR_EGO;
	byte VAR_CAMERA_POS_X;
	byte VAR_HAVE_MSG;
	byte VAR_ROOM;
	byte VAR_OVERRIDE;
	byte VAR_MACHINE_SPEED;
	byte VAR_ME;
	byte VAR_NUM_ACTOR;
	byte VAR_CURRENT_LIGHTS;
	byte VAR_CURRENTDRIVE;
	byte VAR_CURRENTDISK;
	byte VAR_TMR_1;
	byte VAR_TMR_2;
	byte VAR_TMR_3;
	byte VAR_MUSIC_TIMER;
	byte VAR_ACTOR_RANGE_MIN;
	byte VAR_ACTOR_RANGE_MAX;
	byte VAR_CAMERA_MIN_X;
	byte VAR_CAMERA_MAX_X;
	byte VAR_TIMER_NEXT;
	byte VAR_VIRT_MOUSE_X;
	byte VAR_VIRT_MOUSE_Y;
	byte VAR_ROOM_RESOURCE;
	byte VAR_LAST_SOUND;
	byte VAR_CUTSCENEEXIT_KEY;
	byte VAR_OPTIONS_KEY;
	byte VAR_TALK_ACTOR;
	byte VAR_CAMERA_FAST_X;
	byte VAR_SCROLL_SCRIPT;
	byte VAR_ENTRY_SCRIPT;
	byte VAR_ENTRY_SCRIPT2;
	byte VAR_EXIT_SCRIPT;
	byte VAR_EXIT_SCRIPT2;
	byte VAR_VERB_SCRIPT;
	byte VAR_SENTENCE_SCRIPT;
	byte VAR_INVENTORY_SCRIPT;
	byte VAR_CUTSCENE_START_SCRIPT;
	byte VAR_CUTSCENE_END_SCRIPT;
	byte VAR_CHARINC;
	byte VAR_WALKTO_OBJ;
	byte VAR_DEBUGMODE;
	byte VAR_HEAPSPACE;
	byte VAR_RESTART_KEY;
	byte VAR_PAUSE_KEY;
	byte VAR_MOUSE_X;
	byte VAR_MOUSE_Y;
	byte VAR_TIMER;
	byte VAR_TMR_4;
	byte VAR_SOUNDCARD;
	byte VAR_VIDEOMODE;
	byte VAR_MAINMENU_KEY;
	byte VAR_FIXEDDISK;
	byte VAR_CURSORSTATE;
	byte VAR_USERPUT;
	byte VAR_SOUNDRESULT;
	byte VAR_TALKSTOP_KEY;
	byte VAR_FADE_DELAY;
	byte VAR_NOSUBTITLES;

	// V5+
	byte VAR_SOUNDPARAM;
	byte VAR_SOUNDPARAM2;
	byte VAR_SOUNDPARAM3;
	byte VAR_INPUTMODE;
	byte VAR_MEMORY_PERFORMANCE;
	byte VAR_VIDEO_PERFORMANCE;
	byte VAR_ROOM_FLAG;
	byte VAR_GAME_LOADED;
	byte VAR_NEW_ROOM;

	// V4/V5
	byte VAR_V5_TALK_STRING_Y;

	// V6+
	byte VAR_ROOM_WIDTH;
	byte VAR_ROOM_HEIGHT;
	byte VAR_SUBTITLES;
	byte VAR_V6_EMSSPACE;

	// V7/V8 specific variables
	byte VAR_CAMERA_POS_Y;
	byte VAR_CAMERA_MIN_Y;
	byte VAR_CAMERA_MAX_Y;
	byte VAR_CAMERA_THRESHOLD_X;
	byte VAR_CAMERA_THRESHOLD_Y;
	byte VAR_CAMERA_SPEED_X;
	byte VAR_CAMERA_SPEED_Y;
	byte VAR_CAMERA_ACCEL_X;
	byte VAR_CAMERA_ACCEL_Y;
	byte VAR_CAMERA_DEST_X;
	byte VAR_CAMERA_DEST_Y;
	byte VAR_CAMERA_FOLLOWED_ACTOR;

	// V7/V8 specific variables
	byte VAR_VERSION_KEY;
	byte VAR_DEFAULT_TALK_DELAY;
	byte VAR_CUSTOMSCALETABLE;
	byte VAR_BLAST_ABOVE_TEXT;
	byte VAR_VOICE_MODE;
	byte VAR_MUSIC_BUNDLE_LOADED;
	byte VAR_VOICE_BUNDLE_LOADED;

	byte VAR_LEFTBTN_DOWN;	// V7/V8
	byte VAR_RIGHTBTN_DOWN;	// V7/V8
	byte VAR_LEFTBTN_HOLD;	// V6/V72HE/V7/V8
	byte VAR_RIGHTBTN_HOLD;	// V6/V72HE/V7/V8
	byte VAR_SAVELOAD_SCRIPT;	// V6/V7 (not HE)
	byte VAR_SAVELOAD_SCRIPT2;	// V6/V7 (not HE)

	// V6/V7 specific variables (FT & Sam & Max specific)
	byte VAR_CHARSET_MASK;

	// V6 specific variables
	byte VAR_V6_SOUNDMODE;

	// V1/V2 specific variables
	byte VAR_CHARCOUNT;
	byte VAR_VERB_ALLOWED;
	byte VAR_ACTIVE_VERB;
	byte VAR_ACTIVE_OBJECT1;
	byte VAR_ACTIVE_OBJECT2;

	// HE specific variables
	byte VAR_REDRAW_ALL_ACTORS;		// Used in setActorRedrawFlags()
	byte VAR_SKIP_RESET_TALK_ACTOR;		// Used in setActorCostume()

	byte VAR_SOUND_CHANNEL;			// Used in o_startSound()
	byte VAR_TALK_CHANNEL;			// Used in startHETalkSound()
	byte VAR_SOUNDCODE_TMR;			// Used in processSoundCode()
	byte VAR_RESERVED_SOUND_CHANNELS;	// Used in findFreeSoundChannel()

	byte VAR_MAIN_SCRIPT;			// Used in scummLoop()

	byte VAR_SCRIPT_CYCLE;			// Used in runScript()/runObjectScript()
	byte VAR_NUM_SCRIPT_CYCLES;		// Used in runAllScripts()

	byte VAR_KEY_STATE;			// Used in parseEvents()
	byte VAR_MOUSE_STATE;			// Used in checkExecVerbs();

	// Exists both in V7 and in V72HE:
	byte VAR_NUM_GLOBAL_OBJS;
};

} // End of namespace Scumm

#endif
