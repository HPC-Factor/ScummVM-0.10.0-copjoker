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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/engines/scumm/scumm.cpp $
 * $Id: scumm.cpp 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

#include "common/stdafx.h"

#include "common/config-manager.h"
#include "common/md5.h"
#include "common/events.h"
#include "common/system.h"

#include "gui/message.h"
#include "gui/newgui.h"

#include "graphics/cursorman.h"

#include "scumm/akos.h"
#include "scumm/charset.h"
#include "scumm/costume.h"
#include "scumm/debugger.h"
#include "scumm/dialogs.h"
#include "scumm/file.h"
#include "scumm/imuse/imuse.h"
#include "scumm/imuse_digi/dimuse.h"
#include "scumm/smush/smush_mixer.h"
#include "scumm/smush/smush_player.h"
#include "scumm/insane/insane.h"
#include "scumm/intern.h"
#include "scumm/he/animation_he.h"
#include "scumm/he/intern_he.h"
#include "scumm/he/logic_he.h"
#include "scumm/he/sound_he.h"
#include "scumm/object.h"
#include "scumm/player_nes.h"
#include "scumm/player_v1.h"
#include "scumm/player_v2.h"
#include "scumm/player_v2a.h"
#include "scumm/player_v3a.h"
#include "scumm/he/resource_he.h"
#include "scumm/scumm.h"
#include "scumm/sound.h"
#include "scumm/imuse/sysex.h"
#include "scumm/he/sprite_he.h"
#include "scumm/he/cup_player_he.h"
#include "scumm/util.h"
#include "scumm/verbs.h"

#include "sound/mixer.h"

#if (defined(PALMOS_ARM) || defined(PALMOS_DEBUG) || defined(__GP32__))
namespace Graphics {
	extern void initfonts();
}
#endif

using Common::File;

namespace Scumm {

// Use g_scumm from error() ONLY
ScummEngine *g_scumm = 0;


struct dbgChannelDesc {
	const char *channel, *desc;
	uint32 flag;
};


// Debug channel lookup table for Debugger console
static const dbgChannelDesc debugChannels[] = {
	{"SCRIPTS", "Track script execution", DEBUG_SCRIPTS},
	{"OPCODES", "Track opcode execution", DEBUG_OPCODES},
	{"IMUSE", "Track iMUSE events", DEBUG_IMUSE},
	{"RESOURCE", "Track resource loading/management", DEBUG_RESOURCE},
	{"VARS", "Track variable changes", DEBUG_VARS},
	{"ACTORS", "Actor-related debug", DEBUG_ACTORS},
	{"SOUND", "Sound related debug", DEBUG_SOUND},
	{"INSANE", "Track INSANE", DEBUG_INSANE},
	{"SMUSH", "Track SMUSH", DEBUG_SMUSH}
};

ScummEngine::ScummEngine(OSystem *syst, const DetectorResult &dr)
	: Engine(syst),
	  _game(dr.game),
	  _filenamePattern(dr.fp),
	  _language(dr.language),
	  _debugger(0),
	  _currentScript(0xFF), // Let debug() work on init stage
	  _pauseDialog(0), _mainMenuDialog(0), _versionDialog(0) {

	if (_game.platform == Common::kPlatformNES) {
		_gdi = new GdiNES(this);
	} else if (_game.version <= 1) {
		_gdi = new GdiV1(this);
	} else if (_game.version == 2) {
		_gdi = new GdiV2(this);
	} else {
		_gdi = new Gdi(this);
	}
	_res = new ResourceManager(this);

	// Convert MD5 checksum back into a digest
	for (int i = 0; i < 16; ++i) {
		char tmpStr[3] = "00";
		uint tmpVal;
		tmpStr[0] = dr.md5[2*i];
		tmpStr[1] = dr.md5[2*i+1];
		sscanf(tmpStr, "%x", &tmpVal);
		_gameMD5[i] = (byte)tmpVal;
	}
	
	_fileHandle = 0;
	

	// Init all vars
	_imuse = NULL;
	_imuseDigital = NULL;
	_musicEngine = NULL;
	_verbs = NULL;
	_objs = NULL;
	_sound = NULL;
	memset(&vm, 0, sizeof(vm));
	_quit = false;
	_pauseDialog = NULL;
	_mainMenuDialog = NULL;
	_versionDialog = NULL;
	_fastMode = 0;
	_actors = NULL;
	_arraySlot = NULL;
	_inventory = NULL;
	_newNames = NULL;
	_scummVars = NULL;
	_roomVars = NULL;
	_varwatch = 0;
	_bitVars = NULL;
	_numVariables = 0;
	_numBitVariables = 0;
	_numRoomVariables = 0;
	_numLocalObjects = 0;
	_numGlobalObjects = 0;
	_numArray = 0;
	_numVerbs = 0;
	_numFlObject = 0;
	_numInventory = 0;
	_numRooms = 0;
	_numScripts = 0;
	_numSounds = 0;
	_numCharsets = 0;
	_numNewNames = 0;
	_numGlobalScripts = 0;
	_numCostumes = 0;
	_numImages = 0;
	_numLocalScripts = 60;
	_numSprites = 0;
	_numTalkies = 0;
	_numPalettes = 0;
	_numUnk = 0;
	_curPalIndex = 0;
	_currentRoom = 0;
	_egoPositioned = false;
	_keyPressed = 0;
	_mouseAndKeyboardStat = 0;
	_leftBtnPressed = 0;
	_rightBtnPressed = 0;
	_bootParam = 0;
	_dumpScripts = false;
	_debugMode = 0;
	_heV7DiskOffsets = NULL;
	_heV7RoomIntOffsets = NULL;
	_objectOwnerTable = NULL;
	_objectRoomTable = NULL;
	_objectStateTable = NULL;
	_numObjectsInRoom = 0;
	_userPut = 0;
	_userState = 0;
	_resourceHeaderSize = 8;
	_saveLoadFlag = 0;
	_saveLoadSlot = 0;
	_lastSaveTime = 0;
	_saveTemporaryState = false;
	memset(_saveLoadName, 0, sizeof(_saveLoadName));
	memset(_localScriptOffsets, 0, sizeof(_localScriptOffsets));
	_scriptPointer = NULL;
	_scriptOrgPointer = NULL;
	_opcode = 0;
	vm.numNestedScripts = 0;
	_lastCodePtr = NULL;
	_resultVarNumber = 0;
	_scummStackPos = 0;
	memset(_vmStack, 0, sizeof(_vmStack));
	_fileOffset = 0;
	memset(_resourceMapper, 0, sizeof(_resourceMapper));
	_lastLoadedRoom = 0;
	_roomResource = 0;
	OF_OWNER_ROOM = 0;
	_verbMouseOver = 0;
	_inventoryOffset = 0;
	_classData = NULL;
	_actorToPrintStrFor = 0;
	_sentenceNum = 0;
	memset(_sentence, 0, sizeof(_sentence));
	memset(_string, 0, sizeof(_string));
	_screenB = 0;
	_screenH = 0;
	_roomHeight = 0;
	_roomWidth = 0;
	_screenHeight = 0;
	_screenWidth = 0;
	memset(virtscr, 0, sizeof(virtscr));
	memset(&camera, 0, sizeof(CameraData));
	memset(_colorCycle, 0, sizeof(_colorCycle));
	memset(_colorUsedByCycle, 0, sizeof(_colorUsedByCycle));
	_ENCD_offs = 0;
	_EXCD_offs = 0;
	_CLUT_offs = 0;
	_EPAL_offs = 0;
	_IM00_offs = 0;
	_PALS_offs = 0;
	_fullRedraw = false;
	_bgNeedsRedraw = false;
	_screenEffectFlag = false;
	_completeScreenRedraw = false;
	_disableFadeInEffect = false;
	memset(&_cursor, 0, sizeof(_cursor));
	memset(_grabbedCursor, 0, sizeof(_grabbedCursor));
	_currentCursor = 0;
	_newEffect = 0;
	_switchRoomEffect2 = 0;
	_switchRoomEffect = 0;

	_doEffect = false;
	_currentLights = 0;
	_shakeEnabled = false;
	_shakeFrame = 0;
	_screenStartStrip = 0;
	_screenEndStrip = 0;
	_screenTop = 0;
	_drawObjectQueNr = 0;
	memset(_drawObjectQue, 0, sizeof(_drawObjectQue));
	_palManipStart = 0;
	_palManipEnd = 0;
	_palManipCounter = 0;
	_palManipPalette = NULL;
	_palManipIntermediatePal = NULL;
	memset(gfxUsageBits, 0, sizeof(gfxUsageBits));
	_hePalettes = NULL;
	_shadowPalette = NULL;
	_shadowPaletteSize = 0;
	memset(_currentPalette, 0, sizeof(_currentPalette));
	memset(_darkenPalette, 0, sizeof(_darkenPalette));
	memset(_HEV7ActorPalette, 0, sizeof(_HEV7ActorPalette));
	_palDirtyMin = 0;
	_palDirtyMax = 0;
	_haveMsg = 0;
	_haveActorSpeechMsg = false;
	_useTalkAnims = false;
	_defaultTalkDelay = 0;
	_musicType = MDT_NONE;
	_tempMusic = 0;
	_saveSound = 0;
	memset(_extraBoxFlags, 0, sizeof(_extraBoxFlags));
	memset(_scaleSlots, 0, sizeof(_scaleSlots));
	_charset = NULL;
	_charsetColor = 0;
	memset(_charsetColorMap, 0, sizeof(_charsetColorMap));
	memset(_charsetData, 0, sizeof(_charsetData));
	_charsetBufPos = 0;
	memset(_charsetBuffer, 0, sizeof(_charsetBuffer));
	_copyProtection = false;
	_voiceMode = 0;
	_talkDelay = 0;
	_NES_lastTalkingActor = 0;
	_NES_talkColor = 0;
	_keepText = false;
	_costumeLoader = NULL;
	_costumeRenderer = NULL;
	_2byteFontPtr = 0;
	_V1TalkingActor = 0;
	_NESStartStrip = 0;

	_actorClipOverride.top = 0;
	_actorClipOverride.bottom = 480;
	_actorClipOverride.left = 0;
	_actorClipOverride.right = 640;

	_skipDrawObject = 0;

	//
	// Init all VARS to 0xFF
	//
	VAR_KEYPRESS = 0xFF;
	VAR_SYNC = 0xFF;
	VAR_EGO = 0xFF;
	VAR_CAMERA_POS_X = 0xFF;
	VAR_HAVE_MSG = 0xFF;
	VAR_ROOM = 0xFF;
	VAR_OVERRIDE = 0xFF;
	VAR_MACHINE_SPEED = 0xFF;
	VAR_ME = 0xFF;
	VAR_NUM_ACTOR = 0xFF;
	VAR_CURRENT_LIGHTS = 0xFF;
	VAR_CURRENTDRIVE = 0xFF;	// How about merging this with VAR_CURRENTDISK?
	VAR_CURRENTDISK = 0xFF;
	VAR_TMR_1 = 0xFF;
	VAR_TMR_2 = 0xFF;
	VAR_TMR_3 = 0xFF;
	VAR_MUSIC_TIMER = 0xFF;
	VAR_ACTOR_RANGE_MIN = 0xFF;
	VAR_ACTOR_RANGE_MAX = 0xFF;
	VAR_CAMERA_MIN_X = 0xFF;
	VAR_CAMERA_MAX_X = 0xFF;
	VAR_TIMER_NEXT = 0xFF;
	VAR_VIRT_MOUSE_X = 0xFF;
	VAR_VIRT_MOUSE_Y = 0xFF;
	VAR_ROOM_RESOURCE = 0xFF;
	VAR_LAST_SOUND = 0xFF;
	VAR_CUTSCENEEXIT_KEY = 0xFF;
	VAR_OPTIONS_KEY = 0xFF;
	VAR_TALK_ACTOR = 0xFF;
	VAR_CAMERA_FAST_X = 0xFF;
	VAR_SCROLL_SCRIPT = 0xFF;
	VAR_ENTRY_SCRIPT = 0xFF;
	VAR_ENTRY_SCRIPT2 = 0xFF;
	VAR_EXIT_SCRIPT = 0xFF;
	VAR_EXIT_SCRIPT2 = 0xFF;
	VAR_VERB_SCRIPT = 0xFF;
	VAR_SENTENCE_SCRIPT = 0xFF;
	VAR_INVENTORY_SCRIPT = 0xFF;
	VAR_CUTSCENE_START_SCRIPT = 0xFF;
	VAR_CUTSCENE_END_SCRIPT = 0xFF;
	VAR_CHARINC = 0xFF;
	VAR_CHARCOUNT = 0xFF;
	VAR_WALKTO_OBJ = 0xFF;
	VAR_DEBUGMODE = 0xFF;
	VAR_HEAPSPACE = 0xFF;
	VAR_RESTART_KEY = 0xFF;
	VAR_PAUSE_KEY = 0xFF;
	VAR_MOUSE_X = 0xFF;
	VAR_MOUSE_Y = 0xFF;
	VAR_TIMER = 0xFF;
	VAR_TMR_4 = 0xFF;
	VAR_SOUNDCARD = 0xFF;
	VAR_VIDEOMODE = 0xFF;
	VAR_MAINMENU_KEY = 0xFF;
	VAR_FIXEDDISK = 0xFF;
	VAR_CURSORSTATE = 0xFF;
	VAR_USERPUT = 0xFF;
	VAR_SOUNDRESULT = 0xFF;
	VAR_TALKSTOP_KEY = 0xFF;
	VAR_FADE_DELAY = 0xFF;
	VAR_NOSUBTITLES = 0xFF;

	VAR_SOUNDPARAM = 0xFF;
	VAR_SOUNDPARAM2 = 0xFF;
	VAR_SOUNDPARAM3 = 0xFF;
	VAR_INPUTMODE = 0xFF;
	VAR_MEMORY_PERFORMANCE = 0xFF;

	VAR_VIDEO_PERFORMANCE = 0xFF;
	VAR_ROOM_FLAG = 0xFF;
	VAR_GAME_LOADED = 0xFF;
	VAR_NEW_ROOM = 0xFF;
	VAR_VERSION_KEY = 0xFF;

	VAR_V5_TALK_STRING_Y = 0xFF;

	VAR_ROOM_WIDTH = 0xFF;
	VAR_ROOM_HEIGHT = 0xFF;
	VAR_SUBTITLES = 0xFF;
	VAR_V6_EMSSPACE = 0xFF;

	VAR_CAMERA_POS_Y = 0xFF;
	VAR_CAMERA_MIN_Y = 0xFF;
	VAR_CAMERA_MAX_Y = 0xFF;
	VAR_CAMERA_THRESHOLD_X = 0xFF;
	VAR_CAMERA_THRESHOLD_Y = 0xFF;
	VAR_CAMERA_SPEED_X = 0xFF;
	VAR_CAMERA_SPEED_Y = 0xFF;
	VAR_CAMERA_ACCEL_X = 0xFF;
	VAR_CAMERA_ACCEL_Y = 0xFF;
	VAR_CAMERA_DEST_X = 0xFF;
	VAR_CAMERA_DEST_Y = 0xFF;
	VAR_CAMERA_FOLLOWED_ACTOR = 0xFF;

	VAR_LEFTBTN_DOWN = 0xFF;
	VAR_RIGHTBTN_DOWN = 0xFF;
	VAR_LEFTBTN_HOLD = 0xFF;
	VAR_RIGHTBTN_HOLD = 0xFF;

	VAR_SAVELOAD_SCRIPT = 0xFF;
	VAR_SAVELOAD_SCRIPT2 = 0xFF;

	VAR_DEFAULT_TALK_DELAY = 0xFF;
	VAR_CHARSET_MASK = 0xFF;

	VAR_CUSTOMSCALETABLE = 0xFF;
	VAR_V6_SOUNDMODE = 0xFF;

	VAR_ACTIVE_VERB = 0xFF;
	VAR_ACTIVE_OBJECT1 = 0xFF;
	VAR_ACTIVE_OBJECT2 = 0xFF;
	VAR_VERB_ALLOWED = 0xFF;

	VAR_BLAST_ABOVE_TEXT = 0xFF;
	VAR_VOICE_MODE = 0xFF;
	VAR_MUSIC_BUNDLE_LOADED = 0xFF;
	VAR_VOICE_BUNDLE_LOADED = 0xFF;

	VAR_REDRAW_ALL_ACTORS = 0xFF;
	VAR_SKIP_RESET_TALK_ACTOR = 0xFF;

	VAR_SOUND_CHANNEL = 0xFF;
	VAR_TALK_CHANNEL = 0xFF;
	VAR_SOUNDCODE_TMR = 0xFF;
	VAR_RESERVED_SOUND_CHANNELS = 0xFF;

	VAR_MAIN_SCRIPT = 0xFF;

	VAR_NUM_SCRIPT_CYCLES = 0xFF;
	VAR_SCRIPT_CYCLE = 0xFF;

	VAR_NUM_GLOBAL_OBJS = 0xFF;
	VAR_KEY_STATE = 0xFF;
	VAR_MOUSE_STATE = 0xFF;

	// Use g_scumm from error() ONLY
	g_scumm = this;

	// Read settings from the detector & config manager
	_debugMode = (gDebugLevel >= 0);
	_dumpScripts = ConfMan.getBool("dump_scripts");
	_bootParam = ConfMan.getInt("boot_param");
	// Boot params often need debugging switched on to work
	if (_bootParam)
		_debugMode = true;

	_copyProtection = ConfMan.getBool("copy_protection");
	if (ConfMan.getBool("demo_mode"))
		_game.features |= GF_DEMO;
	if (ConfMan.hasKey("nosubtitles")) {
		printf("Configuration key 'nosubtitles' is deprecated. Use 'subtitles' instead\n");
		if (!ConfMan.hasKey("subtitles"))
			ConfMan.setBool("subtitles", !ConfMan.getBool("nosubtitles"));
	}

	// Make sure that at least subtitles are enabled
	if (ConfMan.getBool("speech_mute") && !ConfMan.getBool("subtitles"))
		ConfMan.setBool("subtitles", true);

	// TODO Detect subtitle only versions of scumm6 games
	if (ConfMan.getBool("speech_mute"))
		_voiceMode = 2;
	else
		_voiceMode = ConfMan.getBool("subtitles");

	if (ConfMan.hasKey("render_mode")) {
		_renderMode = Common::parseRenderMode(ConfMan.get("render_mode"));
	} else {
		_renderMode = Common::kRenderDefault;
	}

	// Do some render mode restirctions
	if (_game.version <= 1)
		_renderMode = Common::kRenderDefault;

	switch (_renderMode) {
	case Common::kRenderHercA:
	case Common::kRenderHercG:
		if (_game.version > 2 && _game.id != GID_MONKEY_EGA)
			_renderMode = Common::kRenderDefault;
		break;

	case Common::kRenderCGA:
	case Common::kRenderEGA:
	case Common::kRenderAmiga:
		if (!(_game.features & GF_16COLOR))
			_renderMode = Common::kRenderDefault;
		break;

	default:
		break;
	}

	_hexdumpScripts = false;
	_showStack = false;

	if (_game.platform == Common::kPlatformFMTowns && _game.version == 3) {	// FM-TOWNS V3 games use 320x240
		_screenWidth = 320;
		_screenHeight = 240;
	} else if (_game.version == 8 || _game.heversion >= 71) {
		// COMI uses 640x480. Likewise starting from version 7.1, HE games use
		// 640x480, too.
		_screenWidth = 640;
		_screenHeight = 480;
	} else if (_game.platform == Common::kPlatformNES) {
		_screenWidth = 256;
		_screenHeight = 240;
	} else {
		_screenWidth = 320;
		_screenHeight = 200;
	}

	_compositeBuf = (byte *)malloc(_screenWidth * _screenHeight);

	_herculesBuf = 0;
	if (_renderMode == Common::kRenderHercA || _renderMode == Common::kRenderHercG) {
		_herculesBuf = (byte *)malloc(Common::kHercW * Common::kHercH);
	}

	// Add debug levels	
	for (int i = 0; i < ARRAYSIZE(debugChannels); ++i)
		Common::addSpecialDebugLevel(debugChannels[i].flag,  debugChannels[i].channel, debugChannels[i].desc);
}


ScummEngine::~ScummEngine() {
	Common::clearAllSpecialDebugLevels();

	if (_musicEngine) {
		_musicEngine->terminate();
		delete _musicEngine;
	}

	_mixer->stopAll();

	for (int i = 0; i < _numActors; ++i)
		delete _actors[i];
	delete [] _actors;
	delete [] _sortedActors;

	delete _2byteFontPtr;
	delete _charset;
	delete _pauseDialog;
	delete _mainMenuDialog;
	delete _versionDialog;
	delete _fileHandle;

	delete _sound;

	delete _costumeLoader;
	delete _costumeRenderer;

	_textSurface.free();

	free(_shadowPalette);

	free(_palManipPalette);
	free(_palManipIntermediatePal);

	free(_objectStateTable);
	free(_objectRoomTable);
	free(_objectOwnerTable);
	free(_inventory);
	free(_verbs);
	free(_objs);
	free(_roomVars);
	free(_scummVars);
	free(_bitVars);
	free(_newNames);
	free(_classData);
	free(_arraySlot);

	free(_compositeBuf);
	free(_herculesBuf);

	delete _debugger;
	
	delete _res;
	delete _gdi;
}


ScummEngine_v5::ScummEngine_v5(OSystem *syst, const DetectorResult &dr)
 : ScummEngine(syst, dr) {

	// All "classic" games (V5 and older) encrypted their data files
	// with exception of the GF_OLD256 games and the PC-Engine version
	// of Loom.
	if (!(_game.features & GF_OLD256) && _game.platform != Common::kPlatformPCEngine) 
		_game.features |= GF_USE_KEY;

	static const uint16 default_cursor_images[4][16] = {
		/* cross-hair */
		{ 0x0080, 0x0080, 0x0080, 0x0080, 0x0080, 0x0080, 0x0000, 0x7e3f,
		  0x0000, 0x0080, 0x0080, 0x0080, 0x0080, 0x0080, 0x0080, 0x0000 },
		/* hourglass */
		{ 0x0000, 0x7ffe, 0x6006, 0x300c, 0x1818, 0x0c30, 0x0660, 0x03c0,
		  0x0660, 0x0c30, 0x1998, 0x33cc, 0x67e6, 0x7ffe, 0x0000, 0x0000 },
		/* arrow */
		{ 0x0000, 0x4000, 0x6000, 0x7000, 0x7800, 0x7c00, 0x7e00, 0x7f00,
		  0x7f80, 0x78c0, 0x7c00, 0x4600, 0x0600, 0x0300, 0x0300, 0x0180 },
		/* hand */
		{ 0x1e00, 0x1200, 0x1200, 0x1200, 0x1200, 0x13ff, 0x1249, 0x1249,
		  0xf249, 0x9001, 0x9001, 0x9001, 0x8001, 0x8001, 0x8001, 0xffff },
	};
	
	static const byte default_cursor_hotspots[10] = {
		8, 7,   8, 7,   1, 1,   5, 0,
		8, 7, //zak256
	};
	

	for (int i = 0; i < 4; i++) {
		memcpy(_cursorImages[i], default_cursor_images[i], 32);
	}
	memcpy(_cursorHotspots, default_cursor_hotspots, 8);

	// Setup flashlight
	memset(&_flashlight, 0, sizeof(_flashlight));
	_flashlight.xStrips = 7;
	_flashlight.yStrips = 7;
	_flashlight.buffer = NULL;
}

ScummEngine_v4::ScummEngine_v4(OSystem *syst, const DetectorResult &dr)
	: ScummEngine_v5(syst, dr) {
	_resourceHeaderSize = 6;
	_game.features |= GF_SMALL_HEADER;
}

ScummEngine_v3::ScummEngine_v3(OSystem *syst, const DetectorResult &dr)
	: ScummEngine_v4(syst, dr) {
	// All v3 and older games only used 16 colors with exception of the GF_OLD256 games.
	if (!(_game.features & GF_OLD256))
		_game.features |= GF_16COLOR;
}

ScummEngine_v3old::ScummEngine_v3old(OSystem *syst, const DetectorResult &dr)
	: ScummEngine_v3(syst, dr) {
	_resourceHeaderSize = 4;
	_game.features |= GF_OLD_BUNDLE;
}

ScummEngine_v2::ScummEngine_v2(OSystem *syst, const DetectorResult &dr)
	: ScummEngine_v3old(syst, dr) {

	_activeInventory = 0;
	_activeObject = 0;
	_activeVerb = 0;

	VAR_SENTENCE_VERB = 0xFF;
	VAR_SENTENCE_OBJECT1 = 0xFF;
	VAR_SENTENCE_OBJECT2 = 0xFF;
	VAR_SENTENCE_PREPOSITION = 0xFF;
	VAR_BACKUP_VERB = 0xFF;

	VAR_CLICK_AREA = 0xFF;
	VAR_CLICK_VERB = 0xFF;
	VAR_CLICK_OBJECT = 0xFF;
}

ScummEngine_v0::ScummEngine_v0(OSystem *syst, const DetectorResult &dr)
	: ScummEngine_v2(syst, dr) {

	_currentMode = 0;
}

ScummEngine_v6::ScummEngine_v6(OSystem *syst, const DetectorResult &dr)
	: ScummEngine(syst, dr) {
	_blastObjectQueuePos = 0;
	memset(_blastObjectQueue, 0, sizeof(_blastObjectQueue));
	_blastTextQueuePos = 0;
	memset(_blastTextQueue, 0, sizeof(_blastTextQueue));

	memset(_akosQueue, 0, sizeof(_akosQueue));
	_akosQueuePos = 0;

	_curActor = 0;
	_curVerb = 0;
	_curVerbSlot = 0;

	VAR_VIDEONAME = 0xFF;
	VAR_RANDOM_NR = 0xFF;
	VAR_STRING2DRAW = 0xFF;

	VAR_TIMEDATE_YEAR = 0xFF;
	VAR_TIMEDATE_MONTH = 0xFF;
	VAR_TIMEDATE_DAY = 0xFF;
	VAR_TIMEDATE_HOUR = 0xFF;
	VAR_TIMEDATE_MINUTE = 0xFF;
	VAR_TIMEDATE_SECOND = 0xFF;
}

ScummEngine_v60he::ScummEngine_v60he(OSystem *syst, const DetectorResult &dr)
	: ScummEngine_v6(syst, dr) {
	memset(_hInFileTable, 0, sizeof(_hInFileTable));
	memset(_hOutFileTable, 0, sizeof(_hOutFileTable));
	memset(_heTimers, 0, sizeof(_heTimers));

	if (_game.heversion >= 61)
		_game.features |= GF_NEW_COSTUMES;
}

ScummEngine_v60he::~ScummEngine_v60he() {
	for (int i = 0; i < 17; ++i) {
		delete _hInFileTable[i];
		delete _hOutFileTable[i];
	}
}

#ifndef DISABLE_HE
ScummEngine_v70he::ScummEngine_v70he(OSystem *syst, const DetectorResult &dr)
	: ScummEngine_v60he(syst, dr) {
	if (_game.platform == Common::kPlatformMacintosh && (_game.heversion >= 72 && _game.heversion <= 73))
		_resExtractor = new MacResExtractor(this);
	else
		_resExtractor = new Win32ResExtractor(this);

	_wiz = new Wiz(this);

	_heV7RoomOffsets = NULL;

	_heSndSoundId = 0;
	_heSndOffset = 0;
	_heSndChannel = 0;
	_heSndFlags = 0;
	_heSndSoundFreq = 0;

	_skipProcessActors = 0;

	_numStoredFlObjects = 0;
	_storedFlObjects = (ObjectData *)calloc(100, sizeof(ObjectData));

	VAR_NUM_SOUND_CHANNELS = 0xFF;
	VAR_WIZ_TCOLOR = 0xFF;
}

ScummEngine_v70he::~ScummEngine_v70he() {
	delete _resExtractor;
	delete _wiz;
	free(_heV7DiskOffsets);
	free(_heV7RoomIntOffsets);
	free(_heV7RoomOffsets);
	free(_storedFlObjects);
}

ScummEngine_v71he::ScummEngine_v71he(OSystem *syst, const DetectorResult &dr)
	: ScummEngine_v70he(syst, dr) {
	_auxBlocksNum = 0;
	memset(_auxBlocks, 0, sizeof(_auxBlocks));
	_auxEntriesNum = 0;
	memset(_auxEntries, 0, sizeof(_auxEntries));
}

ScummEngine_v72he::ScummEngine_v72he(OSystem *syst, const DetectorResult &dr)
	: ScummEngine_v71he(syst, dr) {
	VAR_NUM_ROOMS = 0xFF;
	VAR_NUM_SCRIPTS = 0xFF;
	VAR_NUM_SOUNDS = 0xFF;
	VAR_NUM_COSTUMES = 0xFF;
	VAR_NUM_IMAGES = 0xFF;
	VAR_NUM_CHARSETS = 0xFF;
	VAR_POLYGONS_ONLY = 0xFF;
}

ScummEngine_v80he::ScummEngine_v80he(OSystem *syst, const DetectorResult &dr)
	: ScummEngine_v72he(syst, dr) {
	_heSndResId = 0;
	_curSndId = 0;
	_sndPtrOffs = 0;
	_sndTmrOffs = 0;

	VAR_PLATFORM = 0xFF;
	VAR_PLATFORM_VERSION = 0xFF;
	VAR_CURRENT_CHARSET = 0xFF;
	VAR_COLOR_DEPTH = 0xFF;
}

ScummEngine_v90he::ScummEngine_v90he(OSystem *syst, const DetectorResult &dr)
	: ScummEngine_v80he(syst, dr) {
	_moviePlay = new MoviePlayer(this, _mixer);
	_sprite = new Sprite(this);

	memset(_videoParams.filename, 0, sizeof(_videoParams.filename));
	_videoParams.status = 0;
	_videoParams.flags = 0;
	_videoParams.unk2 = 0;
	_videoParams.wizResNum = 0;

	VAR_NUM_SPRITE_GROUPS = 0xFF;
	VAR_NUM_SPRITES = 0xFF;
	VAR_NUM_PALETTES = 0xFF;
	VAR_NUM_UNK = 0xFF;

	VAR_U32_VERSION = 0xFF;
	VAR_U32_ARRAY_UNK = 0xFF;
}

ScummEngine_v90he::~ScummEngine_v90he() {
	delete _moviePlay;
	delete _sprite;
	if (_game.heversion >= 98) {
		delete _logicHE;
	}
	if (_game.heversion >= 99) {
		free(_hePalettes);
	}
}

ScummEngine_vCUPhe::ScummEngine_vCUPhe(OSystem *syst, const DetectorResult &dr) : Engine(syst){
	_syst = syst;
	_game = dr.game;
	_filenamePattern = dr.fp,
	_quit = false;
	
	_cupPlayer = new CUP_Player(syst, this, _mixer);
}

ScummEngine_vCUPhe::~ScummEngine_vCUPhe() {
	delete _cupPlayer;
}

int ScummEngine_vCUPhe::init() {
	_system->beginGFXTransaction();
		_system->initSize(CUP_Player::kDefaultVideoWidth, CUP_Player::kDefaultVideoHeight);
		initCommonGFX(true);
	_system->endGFXTransaction();

	return 0;
}

int ScummEngine_vCUPhe::go() {
	if (_cupPlayer->open(_filenamePattern.pattern)) {
		_cupPlayer->play();
		_cupPlayer->close();
	}
	return 0;
}

void ScummEngine_vCUPhe::parseEvents() {
	Common::Event event;
	
	while (_eventMan->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_QUIT:
			_quit = true;
			break;

		default:
			break;
		}
	}
}

#endif

#ifndef DISABLE_SCUMM_7_8
ScummEngine_v7::ScummEngine_v7(OSystem *syst, const DetectorResult &dr)
	: ScummEngine_v6(syst, dr) {
	_verbLineSpacing = 10;

	_smushFrameRate = 0;
	_smushVideoShouldFinish = false;
	_smushActive = false;
	_insaneRunning = false;
	_smixer = NULL;
	_splayer = NULL;

	_existLanguageFile = false;
	_languageBuffer = NULL;
	_languageIndex = NULL;
	clearSubtitleQueue();

	_game.features |= GF_NEW_COSTUMES;
}

ScummEngine_v7::~ScummEngine_v7() {
	if (_smixer) {
		_smixer->stop();
		delete _smixer;
	}
	if (_splayer) {
		_splayer->release();
		delete _splayer;
	}

	free(_languageBuffer);
	free(_languageIndex);
}

ScummEngine_v8::ScummEngine_v8(OSystem *syst, const DetectorResult &dr)
	: ScummEngine_v7(syst, dr) {
	_objectIDMap = 0;
	_keyScriptKey = 0;
	_keyScriptNo = 0;

	VAR_LANGUAGE = 0xFF;
}

ScummEngine_v8::~ScummEngine_v8() {
	delete [] _objectIDMap;
}
#endif

#pragma mark -
#pragma mark --- Initialization ---
#pragma mark -

int ScummEngine::init() {

	// Add default file directories.
	if (((_game.platform == Common::kPlatformAmiga) || (_game.platform == Common::kPlatformAtariST)) && (_game.version <= 4)) {
		// This is for the Amiga version of Indy3/Loom/Maniac/Zak
		File::addDefaultDirectory(_gameDataPath + "ROOMS/");
		File::addDefaultDirectory(_gameDataPath + "rooms/");
	}

	if ((_game.platform == Common::kPlatformMacintosh) && (_game.version == 3)) {
		// This is for the Mac version of Indy3/Loom
		File::addDefaultDirectory(_gameDataPath + "Rooms 1/");
		File::addDefaultDirectory(_gameDataPath + "Rooms 2/");
		File::addDefaultDirectory(_gameDataPath + "Rooms 3/");
	}

#ifndef DISABLE_SCUMM_7_8
#ifdef MACOSX
	if (_game.version == 8 && !memcmp(_gameDataPath.c_str(), "/Volumes/MONKEY3_", 17)) {
		// Special case for COMI on Mac OS X. The mount points on OS X depend
		// on the volume name. Hence if playing from CD, we'd get a problem.
		// So if loading of a resource file fails, we fall back to the (fixed)
		// CD mount points (/Volumes/MONKEY3_1 and /Volumes/MONKEY3_2).
		//
		// This check for whether we play from CD is very crude, though.

		File::addDefaultDirectory("/Volumes/MONKEY3_1/RESOURCE/");
		File::addDefaultDirectory("/Volumes/MONKEY3_1/resource/");
		File::addDefaultDirectory("/Volumes/MONKEY3_2/");
		File::addDefaultDirectory("/Volumes/MONKEY3_2/RESOURCE/");
		File::addDefaultDirectory("/Volumes/MONKEY3_2/resource/");
	} else
#endif
	if (_game.version == 8) {
		// This is for COMI
		File::addDefaultDirectory(_gameDataPath + "RESOURCE/");
		File::addDefaultDirectory(_gameDataPath + "resource/");
	}

	if (_game.version == 7) {
		// This is for Full Throttle & The Dig
		File::addDefaultDirectory(_gameDataPath + "VIDEO/");
		File::addDefaultDirectory(_gameDataPath + "video/");
		File::addDefaultDirectory(_gameDataPath + "DATA/");
		File::addDefaultDirectory(_gameDataPath + "data/");
	}
#endif


	// The 	kGenUnchanged method is only used for 'container files', i.e. files
	// that contain the real game files bundled together in an archive format.
	// This is the case of the NES, C64 and Mac versions of certain games.
	// Note: All of these can also occur in 'extracted' form, in which case they
	// are treated like any other SCUMM game.
	if (_filenamePattern.genMethod == kGenUnchanged) {

		if (_game.platform == Common::kPlatformNES) {
			// We read data directly from NES ROM instead of extracting it with
			// external tool
			assert(_game.id == GID_MANIAC);
			_fileHandle = new ScummNESFile();
			_containerFile = _filenamePattern.pattern;
			
			_filenamePattern.pattern = "%.2d.LFL";
			_filenamePattern.genMethod = kGenRoomNum;
		} else if (_game.platform == Common::kPlatformApple2GS) {
			// Read data from Apple II disk images.
			const char *tmpBuf1, *tmpBuf2;
			assert(_game.id == GID_MANIAC);
			tmpBuf1 = "maniac1.dsk";
			tmpBuf2 = "maniac2.dsk";
	
			_fileHandle = new ScummDiskImage(tmpBuf1, tmpBuf2, _game);
			_containerFile = tmpBuf1;

			_filenamePattern.pattern = "%.2d.LFL";
			_filenamePattern.genMethod = kGenRoomNum;
		} else if (_game.platform == Common::kPlatformC64) {
			// Read data from C64 disk images.
			const char *tmpBuf1, *tmpBuf2;
			assert(_game.id == GID_MANIAC || _game.id == GID_ZAK);
			if (_game.id == GID_MANIAC) {
				tmpBuf1 = "maniac1.d64";
				tmpBuf2 = "maniac2.d64";
			} else {
				tmpBuf1 = "zak1.d64";
				tmpBuf2 = "zak2.d64";
			}
	
			_fileHandle = new ScummDiskImage(tmpBuf1, tmpBuf2, _game);
			_containerFile = tmpBuf1;

			_filenamePattern.pattern = "%.2d.LFL";
			_filenamePattern.genMethod = kGenRoomNum;
		} else if (_game.platform == Common::kPlatformMacintosh) {
			// The mac versions of Indy4, Sam&Max, DOTT, FT and The Dig used a
			// special meta (container) file format to store the actual SCUMM data
			// files. The rescumm utility used to be used to extract those files. 
			// While that is still possible, we now support reading those files 
			// directly. The first step is to check whether one of them is present
			// (we do that here); the rest is handled by the  ScummFile class and 
			// code in openResourceFile() (and in the Sound class, for MONSTER.SOU
			// handling).
			assert(_game.version >= 5 && _game.heversion == 0);
			_fileHandle = new ScummFile();
			_containerFile = _filenamePattern.pattern;
			
			
			// We now have to determine the correct _filenamePattern. To do this
			// we simply hardcode the possibilites. 
			const char *p1 = 0, *p2 = 0;
			switch (_game.id) {
			case GID_INDY4:
				p1 = "atlantis.%03d";
				break;
			case GID_TENTACLE:
				p1 = "tentacle.%03d";
				p2 = "dottdemo.%03d";
				break;
			case GID_SAMNMAX:
				p1 = "samnmax.%03d";
				p2 = "samdemo.%03d";
				break;
			case GID_FT:
				p1 = "ft.la%d";
				p2 = "ftdemo.la%d";
				break;
			case GID_DIG:
				p1 = "dig.la%d";
				break;
			default:
				break;
			}

			// Test which file name to use
			_filenamePattern.genMethod = kGenDiskNum;
			if (!_fileHandle->open(_containerFile))
				error("Couldn't open container file '%s'", _containerFile.c_str());

			if ((_filenamePattern.pattern = p1) && _fileHandle->openSubFile(generateFilename(0))) {
				// Found regular version
			} else if ((_filenamePattern.pattern = p2) && _fileHandle->openSubFile(generateFilename(0))) {
				// Found demo
				_game.features |= GF_DEMO;
			} else
				error("Couldn't find known subfile inside container file '%s'", _containerFile.c_str());
			
			_fileHandle->close();
			
		} else {
			error("kGenUnchanged used with unsupported platform");
		}
	} else {
		// Regular access, no container file involved
		_fileHandle = new ScummFile();
	}
	
	// Initialize backend
	_system->beginGFXTransaction();
		bool defaultTo1XScaler = false;
		if (_renderMode == Common::kRenderHercA || _renderMode == Common::kRenderHercG) {
			_system->initSize(Common::kHercW, Common::kHercH);
			defaultTo1XScaler = true;
		} else {
			_system->initSize(_screenWidth, _screenHeight);
			defaultTo1XScaler = (_screenWidth > 320);
		}
		initCommonGFX(defaultTo1XScaler);
	_system->endGFXTransaction();

	setupScumm();

	readIndexFile();

	// Create the debugger now that _numVariables has been set
	_debugger = new ScummDebugger(this);

	resetScumm();
	resetScummVars();

	if (_imuse) {
		_imuse->setBase(_res->address[rtSound]);
	}

	if (_game.version >= 5 && _game.version <= 7)
		_sound->setupSound();

	updateSoundSettings();

	return 0;
}

void ScummEngine::setupScumm() {
	// On some systems it's not safe to run CD audio games from the CD.
	if (_game.features & GF_AUDIOTRACKS) {
		checkCD();
	
		int cd_num = ConfMan.getInt("cdrom");
		if (cd_num >= 0)
			_system->openCD(cd_num);
	}

	// Create the sound manager
	if (_game.heversion > 0)
		_sound = new SoundHE(this, _mixer);
	else
		_sound = new Sound(this, _mixer);

	// Setup the music engine
	setupMusic(_game.midi);

	// Load localization data, if present
	loadLanguageBundle();

	// Load CJK font, if present
	loadCJKFont();

	// Create the charset renderer
	setupCharsetRenderer();

	// Create and clear the text surface
	_textSurface.create(_screenWidth, _screenHeight, 1);
	clearTextSurface();

	// Create the costume renderer
	setupCostumeRenderer();

	// Load game from specified slot, if any
	if (ConfMan.hasKey("save_slot")) {
		requestLoad(ConfMan.getInt("save_slot"));
	}

	_res->allocResTypeData(rtBuffer, 0, 10, "buffer", 0);

	setupScummVars();

	setupOpcodes();

	if (_game.version == 8)
		_numActors = 80;
	else if (_game.version == 7)
		_numActors = 30;
	else if (_game.id == GID_SAMNMAX)
		_numActors = 30;
	else if (_game.id == GID_MANIAC)
		_numActors = 25;
	else if (_game.heversion >= 80)
		_numActors = 62;
	else if (_game.heversion >= 72)
		_numActors = 30;
	else
		_numActors = 13;

	if (_game.version >= 7)
		OF_OWNER_ROOM = 0xFF;
	else
		OF_OWNER_ROOM = 0x0F;

	// if (_game.id==GID_MONKEY2 && _bootParam == 0)
	//	_bootParam = 10001;

	if (!_copyProtection && _game.id == GID_INDY4 && _bootParam == 0) {
		_bootParam = -7873;
	}

	if (!_copyProtection && _game.id == GID_SAMNMAX && _bootParam == 0) {
		_bootParam = -1;
	}

	int maxHeapThreshold = -1;
#ifdef PALMOS_68K
	if (_game.features & GF_NEW_COSTUMES)
		maxHeapThreshold = gVars->memory[kMemScummNewCostGames];
	else
		maxHeapThreshold = gVars->memory[kMemScummOldCostGames];
#else
	if (_game.features & GF_NEW_COSTUMES) {
		// Since the new costumes are very big, we increase the heap limit, to avoid having
		// to constantly reload stuff from the data files.
		maxHeapThreshold = 6 * 1024 * 1024;
	} else {
		maxHeapThreshold = 550000;
	}
#endif
	_res->setHeapThreshold(400000, maxHeapThreshold);

#if (defined(PALMOS_ARM) || defined(PALMOS_DEBUG) || defined(__GP32__))
	Graphics::initfonts();
#endif
}

#ifndef DISABLE_SCUMM_7_8
void ScummEngine_v7::setupScumm() {

	_musicEngine = _imuseDigital = new IMuseDigital(this, _mixer, 10);

	ScummEngine::setupScumm();

	// Create FT INSANE object
	if (_game.id == GID_FT)
		_insane = new Insane(this);
	else
		_insane = 0;

	_smixer = new SmushMixer(_mixer);

	_splayer = new SmushPlayer(this);
}
#endif

void ScummEngine::setupCharsetRenderer() {
	if (_game.platform == Common::kPlatformNES)
		_charset = new CharsetRendererNES(this);
	else if (_game.version <= 2)
		_charset = new CharsetRendererV2(this, _language);
	else if (_game.version == 3)
		_charset = new CharsetRendererV3(this);
#ifndef DISABLE_SCUMM_7_8
	else if (_game.version == 8)
		_charset = new CharsetRendererNut(this);
#endif
	else
		_charset = new CharsetRendererClassic(this);
}

void ScummEngine::setupCostumeRenderer() {
	if (_game.features & GF_NEW_COSTUMES) {
		_costumeRenderer = new AkosRenderer(this);
		_costumeLoader = new AkosCostumeLoader(this);
	} else if (_game.version == 0) {
		_costumeRenderer = new C64CostumeRenderer(this);
		_costumeLoader = new C64CostumeLoader(this);
	} else if (_game.platform == Common::kPlatformNES) {
		_costumeRenderer = new NESCostumeRenderer(this);
		_costumeLoader = new NESCostumeLoader(this);
	} else {
		_costumeRenderer = new ClassicCostumeRenderer(this);
		_costumeLoader = new ClassicCostumeLoader(this);
	}
}

void ScummEngine::resetScumm() {
	int i;

	_tempMusic = 0;
	debug(9, "resetScumm");

	if (_game.version == 0) {
		initScreens(8, 144);
	} else if ((_game.id == GID_MANIAC) && (_game.version <= 1) && !(_game.platform == Common::kPlatformNES)) {
		initScreens(16, 152);
	} else if (_game.version >= 7 || _game.heversion >= 71) {
		initScreens(0, _screenHeight);
	} else {
		initScreens(16, 144);
	}

	_palManipCounter = 0;

	for (i = 0; i < 256; i++)
		_roomPalette[i] = i;

	resetPalette();
	if (_game.version == 1) {
	} else if (_game.features & GF_16COLOR) {
		for (i = 0; i < 16; i++)
			_shadowPalette[i] = i;
	}

	if (_game.version >= 4 && _game.version <= 7)
		loadCharset(1);

	if (_game.features & GF_OLD_BUNDLE)
		loadCharset(0);

	setShake(0);
	_cursor.animate = 1;

	// Allocate and Initialize actors
	Actor::kInvalidBox = ((_game.features & GF_SMALL_HEADER) ? kOldInvalidBox : kNewInavlidBox);
	_actors = new Actor * [_numActors];
	_sortedActors = new Actor * [_numActors];
	for (i = 0; i < _numActors; ++i) {
		if (_game.version == 0)
			_actors[i] = new ActorC64(this, i);
		else if (_game.version <= 2)
			_actors[i] = new Actor_v2(this, i);
		else if (_game.version == 3)
			_actors[i] = new Actor_v3(this, i);
		else
			_actors[i] = new Actor(this, i);
		_actors[i]->initActor(-1);

		// this is from IDB
		if ((_game.version <= 1) || (_game.id == GID_MANIAC && (_game.features & GF_DEMO)))
			_actors[i]->setActorCostume(i);
	}

	if (_game.id == GID_MANIAC && _game.version <= 1) {
		resetV1ActorTalkColor();
	} else if (_game.id == GID_MANIAC && _game.version == 2 && (_game.features & GF_DEMO)) {
		// HACK Some palette changes needed for demo script
		// in Maniac Mansion (Enhanced)
		_actors[3]->setPalette(3, 1);
		_actors[9]->_talkColor = 15;
		_actors[10]->_talkColor = 7;
		_actors[11]->_talkColor = 2;
		_actors[13]->_talkColor = 5;
		_actors[23]->_talkColor = 14;
	}

	vm.numNestedScripts = 0;
	vm.cutSceneStackPointer = 0;

	memset(vm.cutScenePtr, 0, sizeof(vm.cutScenePtr));
	memset(vm.cutSceneData, 0, sizeof(vm.cutSceneData));

	for (i = 0; i < _numVerbs; i++) {
		_verbs[i].verbid = 0;
		_verbs[i].curRect.right = _screenWidth - 1;
		_verbs[i].oldRect.left = -1;
		_verbs[i].type = 0;
		_verbs[i].color = 2;
		_verbs[i].hicolor = 0;
		_verbs[i].charset_nr = 1;
		_verbs[i].curmode = 0;
		_verbs[i].saveid = 0;
		_verbs[i].center = 0;
		_verbs[i].key = 0;
	}

	if (_game.version >= 7) {
		VAR(VAR_CAMERA_THRESHOLD_X) = 100;
		VAR(VAR_CAMERA_THRESHOLD_Y) = 70;
		VAR(VAR_CAMERA_ACCEL_X) = 100;
		VAR(VAR_CAMERA_ACCEL_Y) = 100;
	} else {
		if (_game.platform == Common::kPlatformNES) {
			camera._leftTrigger = 6;	// 6
			camera._rightTrigger = 21;	// 25
		} else {
			camera._leftTrigger = 10;
			camera._rightTrigger = (_game.heversion >= 71) ? 70 : 30;
		}
		camera._mode = 0;
	}
	camera._follows = 0;

	virtscr[0].xstart = 0;

	_mouse.x = 104;
	_mouse.y = 56;

	_ENCD_offs = 0;
	_EXCD_offs = 0;

	_currentScript = 0xFF;
	_sentenceNum = 0;

	_currentRoom = 0;
	_numObjectsInRoom = 0;
	_actorToPrintStrFor = 0;

	_charsetBufPos = 0;
	_haveMsg = 0;
	_haveActorSpeechMsg = false;

	_varwatch = -1;
	_screenStartStrip = 0;

	_defaultTalkDelay = 3;
	_talkDelay = 0;
	_keepText = false;
	_nextLeft = 0;
	_nextTop = 0;

	_currentCursor = 0;
	_cursor.state = 0;
	_userPut = 0;

	_newEffect = 129;
	_fullRedraw = true;

	clearDrawObjectQueue();

	if (_game.platform == Common::kPlatformNES)
		decodeNESBaseTiles();

	for (i = 0; i < 6; i++) {
		if (_game.version == 3) { // FIXME - what is this?
			_string[i]._default.xpos = 0;
			_string[i]._default.ypos = 0;
		} else {
			_string[i]._default.xpos = 2;
			_string[i]._default.ypos = 5;
		}
		_string[i]._default.right = _screenWidth - 1;
		_string[i]._default.height = 0;
		_string[i]._default.color = 0xF;
		_string[i]._default.center = 0;
		_string[i]._default.charset = 0;
	}

	// all keys are released
	for (i = 0; i < 512; i++)
		_keyDownMap[i] = false;

	_lastSaveTime = _system->getMillis();
}

void ScummEngine_v0::resetScumm() {
	ScummEngine_v2::resetScumm();
	resetVerbs();
}

void ScummEngine_v2::resetScumm() {
	ScummEngine::resetScumm();

	if (_game.platform == Common::kPlatformNES) {
		initNESMouseOver();
		_switchRoomEffect2 = _switchRoomEffect = 6;
	} else {
		initV2MouseOver();
		// Seems in V2 there was only a single room effect (iris),
		// so we set that here.
		_switchRoomEffect2 = 1;
		_switchRoomEffect = 5;
	}

	_inventoryOffset = 0;
}

void ScummEngine_v4::resetScumm() {
	ScummEngine::resetScumm();

	// WORKAROUND for bug in boot script of Loom (CD)
	// The boot script sets the characters of string 21, 
	// before creating the string.resource.
	if (_game.id == GID_LOOM) {
		_res->createResource(rtString, 21, 12);
	}
}

void ScummEngine_v6::resetScumm() {
	ScummEngine::resetScumm();
	setDefaultCursor();
}

void ScummEngine_v60he::resetScumm() {
	ScummEngine_v6::resetScumm();

	// HACK cursor hotspot is wrong
	// Original games used
	// setCursorHotspot(8, 7);
	if (_game.id == GID_FUNPACK)
		setCursorHotspot(16, 16);
}

#ifndef DISABLE_HE
void ScummEngine_v72he::resetScumm() {
	ScummEngine_v60he::resetScumm();

	_stringLength = 1;
	memset(_stringBuffer, 0, sizeof(_stringBuffer));
}

void ScummEngine_v90he::resetScumm() {
	ScummEngine_v72he::resetScumm();

	_heObject = 0;
	_heObjectNum = 0;
	_hePaletteNum = 0;

	_sprite->resetTables(0);
	memset(&_wizParams, 0, sizeof(_wizParams));

	if (_game.heversion >= 98) {
		switch (_game.id) {
		case GID_PUTTRACE:
			_logicHE = new LogicHErace(this);
			break;

		case GID_FUNSHOP:
			_logicHE = new LogicHEfunshop(this);
			break;

		case GID_FOOTBALL:
			_logicHE = new LogicHEfootball(this);
			break;

		case GID_SOCCER:
			_logicHE = new LogicHEsoccer(this);
			break;

		default:
			_logicHE = new LogicHE(this);
			break;
		}
	}
}

void ScummEngine_v99he::resetScumm() {
	ScummEngine_v90he::resetScumm();

	_hePalettes = (uint8 *)malloc((_numPalettes + 1) * 1024);
	memset(_hePalettes, 0, (_numPalettes + 1) * 1024);

	// Array 129 is set to base name
	int len = strlen(_filenamePattern.pattern);
	byte *data = defineArray(129, kStringArray, 0, 0, 0, len);
	memcpy(data, _filenamePattern.pattern, len);
}
#endif

void ScummEngine::setupMusic(int midi) {
	int midiDriver = MidiDriver::detectMusicDriver(midi);
	_native_mt32 = ((midiDriver == MD_MT32) || ConfMan.getBool("native_mt32"));
	
	switch (midiDriver) {
	case MD_NULL:
		_musicType = MDT_NONE;
		break;
	case MD_PCSPK:
	case MD_PCJR:
		_musicType = MDT_PCSPK;
		break;
	case MD_TOWNS:
		_musicType = MDT_TOWNS;
		break;
	case MD_ADLIB:
		_musicType = MDT_ADLIB;
		break;
	default:
		_musicType = MDT_MIDI;
		break;
	}

	if ((_game.id == GID_MONKEY_EGA || (_game.id == GID_LOOM && _game.version == 3))
	   &&  (_game.platform == Common::kPlatformPC) && _musicType == MDT_MIDI) {
		Common::String fileName;
		bool missingFile = false;
		if (_game.id == GID_LOOM) {
			Common::File f;
			// The Roland Update does have an 85.LFL, but we don't
			// test for it since the demo doesn't have it.
			for (char c = '2'; c <= '4'; c++) {
				fileName = "8";
				fileName += c;
				fileName += ".LFL";
				if (!Common::File::exists(fileName)) {
					missingFile = true;
					break;
				}
			}
		} else if (_game.id == GID_MONKEY_EGA) {
			fileName = "DISK09.LEC";
			if (!Common::File::exists(fileName)) {
				missingFile = true;
			}
		}

		if (missingFile) {
			GUI::MessageDialog dialog(
				"Native MIDI support requires the Roland Upgrade from LucasArts,\n"
				"but " + fileName + " is missing. Using Adlib instead.", "Ok");
			dialog.runModal();
			_musicType = MDT_ADLIB;
		}
	}
	
	// DOTT + SAM use General MIDI, so they shouldn't use GS settings
	if ((_game.id == GID_TENTACLE) || (_game.id == GID_SAMNMAX))
		_enable_gs = false;
	else
		_enable_gs = ConfMan.getBool("enable_gs");

	/* Bind the mixer to the system => mixer will be invoked
	 * automatically when samples need to be generated */
	if (!_mixer->isReady()) {
		warning("Sound mixer initialization failed");
		if (_musicType == MDT_ADLIB || _musicType == MDT_PCSPK)	{
			midiDriver = MD_NULL;
			_musicType = MDT_NONE;
			warning("MIDI driver depends on sound mixer, switching to null MIDI driver\n");
		}
	}

	// Init iMuse
	if (_game.version >= 7) {
		// Setup for digital iMuse is performed in another place
	} else if (_game.platform == Common::kPlatformApple2GS || _game.platform == Common::kPlatformC64 ||
		_game.platform == Common::kPlatformPCEngine) {
		// TODO
		_musicEngine = NULL;
	} else if (_game.platform == Common::kPlatformNES) {
		_musicEngine = new Player_NES(this, _mixer);
	} else if ((_game.platform == Common::kPlatformAmiga) && (_game.version == 2)) {
		_musicEngine = new Player_V2A(this, _mixer);
	} else if ((_game.platform == Common::kPlatformAmiga) && (_game.version == 3)) {
		_musicEngine = new Player_V3A(this, _mixer);
	} else if ((_game.platform == Common::kPlatformAmiga) && (_game.version <= 4)) {
		_musicEngine = NULL;
	} else if (_game.id == GID_MANIAC && (_game.version == 1)) {
		_musicEngine = new Player_V1(this, _mixer, midiDriver != MD_PCSPK);
	} else if (_game.version <= 2) {
		_musicEngine = new Player_V2(this, _mixer, midiDriver != MD_PCSPK);
	} else if ((_musicType == MDT_PCSPK) && ((_game.version > 2) && (_game.version <= 4))) {
		_musicEngine = new Player_V2(this, _mixer, midiDriver != MD_PCSPK);
	} else if (_game.version >= 3 && _game.heversion <= 61 && _game.platform != Common::kPlatform3DO) {
		MidiDriver *nativeMidiDriver = 0;
		MidiDriver *adlibMidiDriver = 0;

		if (_musicType != MDT_ADLIB)
			nativeMidiDriver = MidiDriver::createMidi(midiDriver);
		if (nativeMidiDriver != NULL && _native_mt32)
			nativeMidiDriver->property(MidiDriver::PROP_CHANNEL_MASK, 0x03FE);
		bool multi_midi = ConfMan.getBool("multi_midi") && _musicType != MDT_NONE && (midi & MDT_ADLIB);
		if (_musicType == MDT_ADLIB || multi_midi) {
			adlibMidiDriver = MidiDriver_ADLIB_create(_mixer);
			adlibMidiDriver->property(MidiDriver::PROP_OLD_ADLIB, (_game.features & GF_SMALL_HEADER) ? 1 : 0);
		}

		_musicEngine = _imuse = IMuse::create(_system, nativeMidiDriver, adlibMidiDriver);
		if (_imuse) {
			_imuse->addSysexHandler
				(/*IMUSE_SYSEX_ID*/ 0x7D,
				 (_game.id == GID_SAMNMAX) ? sysexHandler_SamNMax : sysexHandler_Scumm);
			_imuse->property(IMuse::PROP_GAME_ID, _game.id);
			if (ConfMan.hasKey("tempo"))
				_imuse->property(IMuse::PROP_TEMPO_BASE, ConfMan.getInt("tempo"));
			// YM2162 driver can't handle midi->getPercussionChannel(), NULL shouldn't init MT-32/GM/GS
			if ((midi != MDT_TOWNS) && (midi != MDT_NONE)) { 
				_imuse->property(IMuse::PROP_NATIVE_MT32, _native_mt32);
				if (midiDriver != MD_MT32) // MT-32 Emulation shouldn't be GM/GS initialized
					_imuse->property(IMuse::PROP_GS, _enable_gs);
			}
			if (_game.heversion >= 60 || midi == MDT_TOWNS) {
				_imuse->property(IMuse::PROP_LIMIT_PLAYERS, 1);
				_imuse->property(IMuse::PROP_RECYCLE_PLAYERS, 1);
			}
			if (midi == MDT_TOWNS)
				_imuse->property(IMuse::PROP_DIRECT_PASSTHROUGH, 1);
		}
	}
}

void ScummEngine::updateSoundSettings() {

	// Sync the engine with the config manager
	int soundVolumeMusic = ConfMan.getInt("music_volume");
	int soundVolumeSfx = ConfMan.getInt("sfx_volume");
	int soundVolumeSpeech = ConfMan.getInt("speech_volume");

	if (_musicEngine) {
		_musicEngine->setMusicVolume(soundVolumeMusic);
	}

	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, soundVolumeSfx);
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, soundVolumeMusic);
	_mixer->setVolumeForSoundType(Audio::Mixer::kSpeechSoundType, soundVolumeSpeech);

	if (ConfMan.getBool("speech_mute"))
		_voiceMode = 2;
	else
		_voiceMode = ConfMan.getBool("subtitles");

	if (VAR_VOICE_MODE != 0xFF)
		VAR(VAR_VOICE_MODE) = _voiceMode;

	_defaultTalkDelay = getTalkspeed();
	if (VAR_CHARINC != 0xFF)
		VAR(VAR_CHARINC) = _defaultTalkDelay;
}

void ScummEngine::setTalkspeed(int talkspeed) {
	ConfMan.setInt("talkspeed", (talkspeed * 255 + 9 / 2) / 9);
}

int ScummEngine::getTalkspeed() {
	return (ConfMan.getInt("talkspeed") * 9 + 255 / 2) / 255;
}


#pragma mark -
#pragma mark --- Main loop ---
#pragma mark -

int ScummEngine::go() {
	_engineStartTime = _system->getMillis() / 1000;

	// If requested, load a save game instead of running the boot script
	if (_saveLoadFlag != 2 || !loadState(_saveLoadSlot, _saveTemporaryState)) {
		_saveLoadFlag = 0;
		runBootscript();
	} else {
		_saveLoadFlag = 0;
	}

	int delta = 0;
	int diff = _system->getMillis();

	while (!_quit) {

		if (_debugger->isAttached())
			_debugger->onFrame();
	
		// Randomize the PRNG by calling it at regular intervals. This ensures
		// that it will be in a different state each time you run the program.
		_rnd.getRandomNumber(2);
	
		diff -= _system->getMillis();
		waitForTimer(delta * 15 + diff);
		diff = _system->getMillis();
		delta = scummLoop(delta);

		if (delta < 1)	// Ensure we don't get into an endless loop
			delta = 1;  // by not decreasing sleepers.

		if (_quit) {
			// TODO: Maybe perform an autosave on exit?
		}
	}

	return 0;
}

void ScummEngine::waitForTimer(int msec_delay) {
	uint32 start_time;

	if (_fastMode & 2)
		msec_delay = 0;
	else if (_fastMode & 1)
		msec_delay = 10;

	start_time = _system->getMillis();

	while (!_quit) {
		_sound->updateCD(); // Loop CD Audio if needed
		parseEvents();
		_system->updateScreen();
		if (_system->getMillis() >= start_time + msec_delay)
			break;
		_system->delayMillis(10);
	}
}

int ScummEngine::scummLoop(int delta) {
	if (_game.version >= 3) {
		VAR(VAR_TMR_1) += delta;
		VAR(VAR_TMR_2) += delta;
		VAR(VAR_TMR_3) += delta;
		if ((_game.id == GID_INDY3 && _game.platform != Common::kPlatformMacintosh) ||
			_game.id == GID_ZAK) {
			// Amiga/PC versions of Indy3 set three extra timers
			// FM-TOWNS version of Zak sets three extra timers
			VAR(39) += delta;
			VAR(40) += delta;
			VAR(41) += delta;
		}
	}
	if (VAR_TMR_4 != 0xFF)
		VAR(VAR_TMR_4) += delta;

	if (delta > 15)
		delta = 15;

	decreaseScriptDelay(delta);

	_talkDelay -= delta;
	if (_talkDelay < 0)
		_talkDelay = 0;

	// Record the current ego actor before any scripts (including input scripts)
	// get a chance to run.
	int oldEgo = 0;
	if (VAR_EGO != 0xFF)
		oldEgo = VAR(VAR_EGO);

	// In V1-V3 games, CHARSET_1 is called much earlier than in newer games.
	// See also bug #770042 for a case were this makes a difference.
	if (_game.version <= 3)
		CHARSET_1();

	processInput();
	
	scummLoop_updateScummVars();

	if (_game.features & GF_AUDIOTRACKS) {
		// Covered automatically by the Sound class
	} else if (VAR_MUSIC_TIMER != 0xFF) {
		if (_musicEngine) {
			// The music engine generates the timer data for us.
			VAR(VAR_MUSIC_TIMER) = _musicEngine->getMusicTimer();
		} else {
			// Used for Money Island 1 (Amiga)
			// TODO: The music delay (given in milliseconds) might have to be tuned a little
			// to get it correct for all games. Without the ability to watch/listen to the
			// original games, I can't do that myself.
			const int MUSIC_DELAY = 350;
			_tempMusic += delta * 15;	// Convert delta to milliseconds
			if (_tempMusic >= MUSIC_DELAY) {
				_tempMusic -= MUSIC_DELAY;
				VAR(VAR_MUSIC_TIMER) += 1;
			}
		}
	}

	// Trigger autosave if necessary.
	if (!_saveLoadFlag && shouldPerformAutoSave(_lastSaveTime)) {
		_saveLoadSlot = 0;
		sprintf(_saveLoadName, "Autosave %d", _saveLoadSlot);
		_saveLoadFlag = 1;
		_saveTemporaryState = false;
	}

	if (VAR_GAME_LOADED != 0xFF)
		VAR(VAR_GAME_LOADED) = 0;
load_game:
	scummLoop_handleSaveLoad();

	if (_completeScreenRedraw) {
		clearCharsetMask();
		_charset->_hasMask = false;

		// HACK as in game save stuff isn't supported currently
		if (_game.id == GID_LOOM) {
			int args[16];
			uint var;
			memset(args, 0, sizeof(args));
			args[0] = 2;

			if (_game.platform == Common::kPlatformMacintosh)
				var = 105;
			else if (_game.version == 4)	// 256 color CD version
				var = 150;
			else
 				var = 100;
			byte restoreScript = (_game.platform == Common::kPlatformFMTowns) ? 17 : 18;
			// if verbs should be shown restore them
			if (VAR(var) == 2)
				runScript(restoreScript, 0, 0, args);
		} else if (_game.version > 3) {
			for (int i = 0; i < _numVerbs; i++)
				drawVerb(i, 0);
		} else {
			redrawVerbs();
		}

		handleMouseOver(false);

		_completeScreenRedraw = false;
		_fullRedraw = true;
	}

	if (_game.heversion >= 80) {
		((SoundHE *)_sound)->processSoundCode();
	}
	runAllScripts();
	checkExecVerbs();
	checkAndRunSentenceScript();

	if (_quit)
		return 0;

	// HACK: If a load was requested, immediately perform it. This avoids
	// drawing the current room right after the load is request but before
	// it is performed. That was annoying esp. if you loaded while a SMUSH
	// cutscene was playing.
	if (_saveLoadFlag && _saveLoadFlag != 1) {
		goto load_game;
	}

	if (_currentRoom == 0) {
		if (_game.version > 3)
			CHARSET_1();
		drawDirtyScreenParts();
	} else {
		walkActors();
		moveCamera();
		updateObjectStates();
		if (_game.version > 3)
			CHARSET_1();

		scummLoop_handleDrawing();

		scummLoop_handleActors();

		_fullRedraw = false;

		scummLoop_handleEffects();

		if (VAR_MAIN_SCRIPT != 0xFF && VAR(VAR_MAIN_SCRIPT) != 0) {
			runScript(VAR(VAR_MAIN_SCRIPT), 0, 0, 0);
		}

		// Handle mouse over effects (for verbs).
		handleMouseOver(oldEgo != VAR(VAR_EGO));

		// Render everything to the screen.
		updatePalette();
		drawDirtyScreenParts();

		// FIXME / TODO: Try to move the following to scummLoop_handleSound or
		// scummLoop_handleActors (but watch out for regressions!)
		if (_game.version <= 5)
			playActorSounds();
	}

	scummLoop_handleSound();

	camera._last = camera._cur;

	_res->increaseExpireCounter();

	animateCursor();

	/* show or hide mouse */
	CursorMan.showMouse(_cursor.state > 0);

	if (VAR_TIMER != 0xFF)
		VAR(VAR_TIMER) = 0;
	return (VAR_TIMER_NEXT != 0xFF) ? VAR(VAR_TIMER_NEXT) : 4;

}

#ifndef DISABLE_HE
int ScummEngine_v90he::scummLoop(int delta) {
	_moviePlay->handleNextFrame();
	if (_game.heversion >= 98) {
		_logicHE->startOfFrame();
	}

	int ret = ScummEngine::scummLoop(delta);
	
	_sprite->updateImages();
	if (_game.heversion >= 98) {
		_logicHE->endOfFrame();
	}
	
	return ret;
}
#endif

void ScummEngine::scummLoop_updateScummVars() {
	if (_game.version >= 7) {
		VAR(VAR_CAMERA_POS_X) = camera._cur.x;
		VAR(VAR_CAMERA_POS_Y) = camera._cur.y;
	} else if (_game.version <= 2) {
		VAR(VAR_CAMERA_POS_X) = camera._cur.x >> V12_X_SHIFT;
	} else {
		VAR(VAR_CAMERA_POS_X) = camera._cur.x;
	}
	if (_game.version <= 7)
		VAR(VAR_HAVE_MSG) = _haveMsg;

	if (_game.version >= 3) {
		VAR(VAR_VIRT_MOUSE_X) = _virtualMouse.x;
		VAR(VAR_VIRT_MOUSE_Y) = _virtualMouse.y;
		VAR(VAR_MOUSE_X) = _mouse.x;
		VAR(VAR_MOUSE_Y) = _mouse.y;
		if (VAR_DEBUGMODE != 0xFF) {
			// This is NOT for the Mac version of Indy3/Loom
			VAR(VAR_DEBUGMODE) = _debugMode;
		}
	} else if (_game.version >= 1) {
		// We use shifts below instead of dividing by V12_X_MULTIPLIER resp.
		// V12_Y_MULTIPLIER to handle negative coordinates correctly.
		// This fixes e.g. bugs #1328131 and #1537595.
		VAR(VAR_VIRT_MOUSE_X) = _virtualMouse.x >> V12_X_SHIFT;
		VAR(VAR_VIRT_MOUSE_Y) = _virtualMouse.y >> V12_Y_SHIFT;

		// Adjust mouse coordinates as narrow rooms in NES are centered
		if (_game.platform == Common::kPlatformNES && _NESStartStrip > 0) {
			VAR(VAR_VIRT_MOUSE_X) -= 2;
			if (VAR(VAR_VIRT_MOUSE_X) < 0)
				VAR(VAR_VIRT_MOUSE_X) = 0;
		}
	}
}

void ScummEngine::scummLoop_handleSaveLoad() {
	if (_saveLoadFlag) {
		bool success;
		const char *errMsg = 0;
		char filename[256];

		if (_game.version == 8 && _saveTemporaryState)
			VAR(VAR_GAME_LOADED) = 0;

		if (_saveLoadFlag == 1) {
			success = saveState(_saveLoadSlot, _saveTemporaryState);
			if (!success)
				errMsg = "Failed to save game state to file:\n\n%s";

			if (success && _saveTemporaryState && VAR_GAME_LOADED != 0xFF && _game.version <= 7)
				VAR(VAR_GAME_LOADED) = 201;
		} else {
			success = loadState(_saveLoadSlot, _saveTemporaryState);
			if (!success)
				errMsg = "Failed to load game state from file:\n\n%s";

			if (success && _saveTemporaryState && VAR_GAME_LOADED != 0xFF)
				VAR(VAR_GAME_LOADED) = (_game.version == 8) ? 1 : 203;
		}

		makeSavegameName(filename, _saveLoadSlot, _saveTemporaryState);
		if (!success) {
			displayMessage(0, errMsg, filename);
		} else if (_saveLoadFlag == 1 && _saveLoadSlot != 0 && !_saveTemporaryState) {
			// Display "Save successful" message, except for auto saves
			char buf[256];
			snprintf(buf, sizeof(buf), "Successfully saved game state in file:\n\n%s", filename);

			GUI::TimedMessageDialog dialog(buf, 1500);
			runDialog(dialog);
		}
		if (success && _saveLoadFlag != 1)
			clearClickedStatus();

		_saveLoadFlag = 0;
		_lastSaveTime = _system->getMillis();
	}
}

#ifndef DISABLE_SCUMM_7_8
void ScummEngine_v8::scummLoop_handleSaveLoad() {
	ScummEngine::scummLoop_handleSaveLoad();

	removeBlastObjects();
}
#endif

void ScummEngine::scummLoop_handleDrawing() {
	if (camera._cur != camera._last || _bgNeedsRedraw || _fullRedraw) {
		redrawBGAreas();
	}

	processDrawQue();
}

#ifndef DISABLE_SCUMM_7_8
void ScummEngine_v7::scummLoop_handleDrawing() {
	ScummEngine_v6::scummLoop_handleDrawing();

	// Full Throttle always redraws verbs and draws verbs before actors
	if (_game.version >= 7)
		redrawVerbs();
}
#endif

#ifndef DISABLE_HE
void ScummEngine_v90he::scummLoop_handleDrawing() {
	ScummEngine_v80he::scummLoop_handleDrawing();

	if (_game.heversion >= 99)
		_fullRedraw = false;

	if (_game.heversion >= 90) {
		_sprite->resetBackground();
		_sprite->sortActiveSprites();
	}
}
#endif

void ScummEngine_v6::scummLoop_handleActors() {
	setActorRedrawFlags();
	resetActorBgs();
	processActors();
}

void ScummEngine_v5::scummLoop_handleActors() {
	setActorRedrawFlags();
	resetActorBgs();

	if (!(getCurrentLights() & LIGHTMODE_room_lights_on) &&
		  getCurrentLights() & LIGHTMODE_flashlight_on) {
		drawFlashlight();
		setActorRedrawFlags();
	}

	processActors();
}

void ScummEngine::scummLoop_handleEffects() {
	if (_game.version >= 4 && _game.heversion <= 61)
		cyclePalette();
	palManipulate();
	if (_doEffect) {
		_doEffect = false;
		fadeIn(_newEffect);
		clearClickedStatus();
	}
}

void ScummEngine::scummLoop_handleSound() {
	_sound->processSound();
}

#ifndef DISABLE_SCUMM_7_8
void ScummEngine_v7::scummLoop_handleSound() {
	ScummEngine_v6::scummLoop_handleSound();
	if (_imuseDigital) {
		_imuseDigital->flushTracks();
		// In CoMI and the full (non-demo) version of The Dig, also invoke IMuseDigital::refreshScripts 
		if ( ((_game.id == GID_DIG) && (!(_game.features & GF_DEMO))) || (_game.id == GID_CMI) )
			_imuseDigital->refreshScripts();
	}
	if (_smixer) {
		_smixer->flush();
	}
}
#endif


#pragma mark -
#pragma mark --- SCUMM ---
#pragma mark -

int ScummEngine_v60he::getHETimer(int timer) {
	assertRange(1, timer, 15, "getHETimer: Timer");
	int time = _system->getMillis() - _heTimers[timer];
	return time;
}

void ScummEngine_v60he::setHETimer(int timer) {
	assertRange(1, timer, 15, "setHETimer: Timer");
	_heTimers[timer] = _system->getMillis();
}

void ScummEngine::pauseGame() {
	pauseDialog();
}

void ScummEngine::shutDown() {
	_quit = true;
}

void ScummEngine::restart() {
// TODO: Check this function - we should probably be reinitting a lot more stuff, and I suspect
//	 this leaks memory like a sieve

// Fingolfing seez: An alternate way to implement restarting would be to create
// a save state right after startup ... to this end we could introduce a SaveFile
// subclass which is implemented using a memory buffer (i.e. no actual file is
// created). Then to restart we just have to load that pseudo save state.


	int i;

	// Reset some stuff
	_currentRoom = 0;
	_currentScript = 0xFF;
	killAllScriptsExceptCurrent();
	setShake(0);
	_sound->stopAllSounds();

	// Clear the script variables
	for (i = 0; i < _numVariables; i++)
		_scummVars[i] = 0;

	// Empty inventory
	for (i = 0; i < _numGlobalObjects; i++)
		clearOwnerOf(i);

	// Reallocate arrays
	// FIXME: This should already be called by readIndexFile.
	// FIXME: regardless of that, allocateArrays and allocResTypeData leaks
	// heavily, which should be fixed.
	allocateArrays();

	// Reread index (reset objectstate etc)
	readIndexFile();

	// Reinit scumm variables
	resetScumm();
	resetScummVars();

	if (_imuse) {
		_imuse->setBase(_res->address[rtSound]);
	}

	// Reinit sound engine
	if (_game.version >= 5 && _game.version <= 7)
		_sound->setupSound();

	// Re-run bootscript
	runBootscript();
}

void ScummEngine::runBootscript() {
	int args[16];
	memset(args, 0, sizeof(args));
	args[0] = _bootParam;
	if (_game.id == GID_MANIAC && (_game.features & GF_DEMO))
		runScript(9, 0, 0, args);
	else
		runScript(1, 0, 0, args);
}

#ifndef DISABLE_HE
void ScummEngine_v90he::runBootscript() {
	if (_game.heversion >= 98) {
		_logicHE->initOnce();
		_logicHE->beforeBootScript();
	}

	ScummEngine::runBootscript();
}
#endif

void ScummEngine::startManiac() {
	debug(0, "stub startManiac()");
	displayMessage(0, "Usually, Maniac Mansion would start now. But ScummVM doesn't do that yet. To play it, go to 'Add Game' in the ScummVM start menu and select the 'Maniac' directory inside the Tentacle game directory.");
}

#pragma mark -
#pragma mark --- GUI ---
#pragma mark -

int ScummEngine::runDialog(Dialog &dialog) {
	_dialogStartTime = _system->getMillis() / 1000;

	// Pause sound & video
	bool old_soundsPaused = _sound->_soundsPaused;
	_sound->pauseSounds(true);

	bool visible = CursorMan.isVisible();

	// Open & run the dialog
	int result = dialog.runModal();

	// Restore old cursor
	updateCursor();
	CursorMan.showMouse(visible);

	// Update the screen to make it less likely that the player will see a
	// brief cursor palette glitch when the GUI is disabled.
	_system->updateScreen();

	// Resume sound & video
	_sound->pauseSounds(old_soundsPaused);

	_engineStartTime += (_system->getMillis() / 1000) - _dialogStartTime;
	_dialogStartTime = 0;

	// Return the result
	return result;
}

#ifndef DISABLE_SCUMM_7_8
int ScummEngine_v7::runDialog(Dialog &dialog) {
	_splayer->pause();
	int result = ScummEngine::runDialog(dialog);
	_splayer->unpause();
	return result;
}
#endif

void ScummEngine::pauseDialog() {
	if (!_pauseDialog)
		_pauseDialog = new PauseDialog(this, 4);
	runDialog(*_pauseDialog);
}

void ScummEngine::versionDialog() {
	if (!_versionDialog)
		_versionDialog = new PauseDialog(this, 1);
	runDialog(*_versionDialog);
}

void ScummEngine::mainMenuDialog() {
	if (!_mainMenuDialog)
		_mainMenuDialog = new MainMenuDialog(this);
	runDialog(*_mainMenuDialog);
	updateSoundSettings();
}

void ScummEngine::confirmExitDialog() {
	ConfirmDialog d(this, 6);

	if (runDialog(d)) {
		_quit = true;
	}
}

void ScummEngine::confirmRestartDialog() {
	ConfirmDialog d(this, 5);

	if (runDialog(d)) {
		restart();
	}
}

char ScummEngine::displayMessage(const char *altButton, const char *message, ...) {
	char buf[STRINGBUFLEN];
	va_list va;

	va_start(va, message);
	vsnprintf(buf, STRINGBUFLEN, message, va);
	va_end(va);

	GUI::MessageDialog dialog(buf, "OK", altButton);
	return runDialog(dialog);
}


#pragma mark -
#pragma mark --- Miscellaneous ---
#pragma mark -

GUI::Debugger *ScummEngine::getDebugger() {
	return _debugger;
}

void ScummEngine::errorString(const char *buf1, char *buf2) {
	if (_currentScript != 0xFF) {
		sprintf(buf2, "(%d:%d:0x%lX): %s", _roomResource,
			vm.slot[_currentScript].number, (long)(_scriptPointer - _scriptOrgPointer), buf1);
	} else {
		strcpy(buf2, buf1);
	}
}


} // End of namespace Scumm
