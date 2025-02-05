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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/engines/queen/queen.cpp $
 * $Id: queen.cpp 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

#include "common/stdafx.h"

#include "base/plugins.h"

#include "common/config-manager.h"
#include "common/file.h"
#include "common/fs.h"
#include "common/savefile.h"
#include "common/system.h"

#include "queen/queen.h"
#include "queen/bankman.h"
#include "queen/command.h"
#include "queen/cutaway.h"
#include "queen/debug.h"
#include "queen/display.h"
#include "queen/graphics.h"
#include "queen/grid.h"
#include "queen/input.h"
#include "queen/logic.h"
#include "queen/resource.h"
#include "queen/sound.h"
#include "queen/talk.h"
#include "queen/walk.h"

static const PlainGameDescriptor queenGameDescriptor = {
	"queen", "Flight of the Amazon Queen"
};

GameList Engine_QUEEN_gameIDList() {
	GameList games;
	games.push_back(queenGameDescriptor);
	return games;
}

GameDescriptor Engine_QUEEN_findGameID(const char *gameid) {
	if (0 == scumm_stricmp(gameid, queenGameDescriptor.gameid)) {
		return queenGameDescriptor;
	}
	return GameDescriptor();
}

GameList Engine_QUEEN_detectGames(const FSList &fslist) {
	GameList detectedGames;

	// Iterate over all files in the given directory
	for (FSList::const_iterator file = fslist.begin(); file != fslist.end(); ++file) {
		if (file->isDirectory()) {
			continue;
		}
		if (file->name().equalsIgnoreCase("queen.1") || file->name().equalsIgnoreCase("queen.1c")) {
			Common::File dataFile;
			if (!dataFile.open(*file)) {
				continue;
			}
			Queen::DetectedGameVersion version;
			if (Queen::Resource::detectVersion(&version, &dataFile)) {
				GameDescriptor dg(queenGameDescriptor.gameid, queenGameDescriptor.description, version.language, version.platform);
				if (version.features & Queen::GF_DEMO) {
					dg.updateDesc("Demo");
				} else if (version.features & Queen::GF_INTERVIEW) {
					dg.updateDesc("Interview");
				} else if (version.features & Queen::GF_FLOPPY) {
					dg.updateDesc("Floppy");
				} else if (version.features & Queen::GF_TALKIE) {
					dg.updateDesc("Talkie");
				}
				detectedGames.push_back(dg);
				break;
			}
		}
	}
	return detectedGames;
}

PluginError Engine_QUEEN_create(OSystem *syst, Engine **engine) {
	assert(engine);
	*engine = new Queen::QueenEngine(syst);
	return kNoError;
}

REGISTER_PLUGIN(QUEEN, "Flight of the Amazon Queen", "Flight of the Amazon Queen (C) John Passfield and Steve Stamatiadis");

namespace Queen {

QueenEngine::QueenEngine(OSystem *syst)
	: Engine(syst), _debugger(0) {
}

QueenEngine::~QueenEngine() {
	delete _bam;
	delete _resource;
	delete _bankMan;
	delete _command;
	delete _debugger;
	delete _display;
	delete _graphics;
	delete _grid;
	delete _input;
	delete _logic;
	delete _sound;
	delete _walk;
}

void QueenEngine::registerDefaultSettings() {
	ConfMan.registerDefault("talkspeed", Logic::DEFAULT_TALK_SPEED);
	ConfMan.registerDefault("subtitles", true);
	_subtitles = true;
}

void QueenEngine::checkOptionSettings() {
	// check talkspeed value
	if (_talkSpeed < MIN_TEXT_SPEED) {
		_talkSpeed = MIN_TEXT_SPEED;
	} else if (_talkSpeed > MAX_TEXT_SPEED) {
		_talkSpeed = MAX_TEXT_SPEED;
	}

	// demo and interview versions don't have speech at all
	if (_sound->speechOn() && (_resource->isDemo() || _resource->isInterview())) {
		_sound->speechToggle(false);
	}

	// ensure text is always on when voice is off
	if (!_sound->speechOn()) {
		_subtitles = true;
	}
}

void QueenEngine::readOptionSettings() {
	_sound->setVolume(ConfMan.getInt("music_volume"));
	_sound->musicToggle(!ConfMan.getBool("music_mute"));
	_sound->sfxToggle(!ConfMan.getBool("sfx_mute"));
	_sound->speechToggle(!ConfMan.getBool("speech_mute"));
	_talkSpeed = (ConfMan.getInt("talkspeed") * (MAX_TEXT_SPEED - MIN_TEXT_SPEED) + 255 / 2) / 255 + MIN_TEXT_SPEED;
	_subtitles = ConfMan.getBool("subtitles");
	checkOptionSettings();
}

void QueenEngine::writeOptionSettings() {
	ConfMan.setInt("music_volume", _sound->getVolume());
	ConfMan.setBool("music_mute", !_sound->musicOn());
	ConfMan.setBool("sfx_mute", !_sound->sfxOn());
	ConfMan.setInt("talkspeed", ((_talkSpeed - MIN_TEXT_SPEED) * 255 + (MAX_TEXT_SPEED - MIN_TEXT_SPEED) / 2) / (MAX_TEXT_SPEED - MIN_TEXT_SPEED));
	ConfMan.setBool("speech_mute", !_sound->speechOn());
	ConfMan.setBool("subtitles", _subtitles);
	ConfMan.flushToDisk();
}

void QueenEngine::update(bool checkPlayerInput) {
	if (_debugger->isAttached()) {
		_debugger->onFrame();
	}

	_graphics->update(_logic->currentRoom());
	_logic->update();

	int frameDelay = (_lastUpdateTime + Input::DELAY_NORMAL - _system->getMillis());
	if (frameDelay <= 0) {
		frameDelay = 1;
	}
	_input->delay(frameDelay);
	_lastUpdateTime = _system->getMillis();

	if (!_resource->isInterview()) {
		_display->palCustomScroll(_logic->currentRoom());
	}
	BobSlot *joe = _graphics->bob(0);
	_display->update(joe->active, joe->x, joe->y);

	_input->checkKeys();
	if (_input->debugger()) {
		_input->debuggerReset();
		_debugger->attach();
	}
	if (canLoadOrSave()) {
		if (_input->quickSave()) {
			_input->quickSaveReset();
			saveGameState(0, "Quicksave");
		}
		if (_input->quickLoad()) {
			_input->quickLoadReset();
			loadGameState(0);
		}
		if (shouldPerformAutoSave(_lastSaveTime)) {
			saveGameState(AUTOSAVE_SLOT, "Autosave");
			_lastSaveTime = _system->getMillis();
		}
	}
	if (!_input->cutawayRunning()) {
		if (checkPlayerInput) {
			_command->updatePlayer();
		}
		if (_input->idleTime() >= Input::DELAY_SCREEN_BLANKER) {
			_display->blankScreen();
		}
	}
	_sound->updateMusic();
}

bool QueenEngine::canLoadOrSave() const {
	return !_input->cutawayRunning() && !(_resource->isDemo() || _resource->isInterview());
}

void QueenEngine::saveGameState(uint16 slot, const char *desc) {
	debug(3, "Saving game to slot %d", slot);
	char name[20];
	makeGameStateName(slot, name);
	Common::OutSaveFile *file = _saveFileMan->openForSaving(name);
	if (file) {
		// save data
		byte *saveData = new byte[SAVESTATE_MAX_SIZE];
		byte *p = saveData;
		_bam->saveState(p);
		_grid->saveState(p);
		_logic->saveState(p);
		_sound->saveState(p);
		uint32 dataSize = p - saveData;
		assert(dataSize < SAVESTATE_MAX_SIZE);

		// write header
		file->writeUint32BE('SCVM');
		file->writeUint32BE(SAVESTATE_CUR_VER);
		file->writeUint32BE(0);
		file->writeUint32BE(dataSize);
		char description[32];
		memset(description, 0, 32);
		strncpy(description, desc, 31);
		file->write(description, 32);

		// write save data
		file->write(saveData, dataSize);
		file->finalize();

		// check for errors
		if (file->ioFailed()) {
			warning("Can't write file '%s'. (Disk full?)", name);
		}
		delete[] saveData;
		delete file;
	} else {
		warning("Can't create file '%s', game not saved", name);
	}
}

void QueenEngine::loadGameState(uint16 slot) {
	debug(3, "Loading game from slot %d", slot);
	GameStateHeader header;
	Common::InSaveFile *file = readGameStateHeader(slot, &header);
	if (file && header.dataSize != 0) {
		byte *saveData = new byte[header.dataSize];
		byte *p = saveData;
		if (file->read(saveData, header.dataSize) != header.dataSize) {
			warning("Error reading savegame file");
		} else {
			_bam->loadState(header.version, p);
			_grid->loadState(header.version, p);
			_logic->loadState(header.version, p);
			_sound->loadState(header.version, p);
			if (header.dataSize != (uint32)(p - saveData)) {
				warning("Corrupted savegame file");
			} else {
				_logic->setupRestoredGame();
			}
		}
		delete[] saveData;
		delete file;
	}
}

Common::InSaveFile *QueenEngine::readGameStateHeader(uint16 slot, GameStateHeader *gsh) {
	char name[20];
	makeGameStateName(slot, name);
	Common::InSaveFile *file = _saveFileMan->openForLoading(name);
	if (file && file->readUint32BE() == MKID_BE('SCVM')) {
		gsh->version = file->readUint32BE();
		gsh->flags = file->readUint32BE();
		gsh->dataSize = file->readUint32BE();
		file->read(gsh->description, sizeof(gsh->description));
	} else {
		memset(gsh, 0, sizeof(GameStateHeader));
	}
	return file;
}

void QueenEngine::makeGameStateName(uint16 slot, char *buf) {
	if (slot == AUTOSAVE_SLOT) {
		strcpy(buf, "queen.asd");
	} else {
		sprintf(buf, "queen.s%02d", slot);
	}
}

void QueenEngine::findGameStateDescriptions(char descriptions[100][32]) {
	char filename[20];
	makeGameStateName(0, filename);
	filename[strlen(filename) - 2] = 0;
	bool marks[SAVESTATE_MAX_NUM];
	_saveFileMan->listSavefiles(filename, marks, SAVESTATE_MAX_NUM);
	for (int i = 0; i < SAVESTATE_MAX_NUM; ++i) {
		if (marks[i]) {
			GameStateHeader header;
			Common::InSaveFile *f = readGameStateHeader(i, &header);
			strcpy(descriptions[i], header.description);
			delete f;
		}
	}
}

GUI::Debugger *QueenEngine::getDebugger() {
	return _debugger;
}

int QueenEngine::go() {
	_logic->start();
	if (ConfMan.hasKey("save_slot") && canLoadOrSave()) {
		loadGameState(ConfMan.getInt("save_slot"));
	}
	_lastSaveTime = _lastUpdateTime = _system->getMillis();
	_quit = false;
	while (!_quit) {
		if (_logic->newRoom() > 0) {
			_logic->update();
			_logic->oldRoom(_logic->currentRoom());
			_logic->currentRoom(_logic->newRoom());
			_logic->changeRoom();
			_display->fullscreen(false);
			if (_logic->currentRoom() == _logic->newRoom()) {
				_logic->newRoom(0);
			}
		} else if (_logic->joeWalk() == JWM_EXECUTE) {
			_logic->joeWalk(JWM_NORMAL);
			_command->executeCurrentAction();
		} else {
			_logic->joeWalk(JWM_NORMAL);
			update(true);
		}
	}
	return 0;
}

int QueenEngine::init() {
	_system->beginGFXTransaction();
		initCommonGFX(false);
		_system->initSize(GAME_SCREEN_WIDTH, GAME_SCREEN_HEIGHT);
	_system->endGFXTransaction();

	_resource = new Resource();

	_bam = new BamScene(this);
	_bankMan = new BankManager(_resource);
	_command = new Command(this);
	_debugger = new Debugger(this);
	_display = new Display(this, _system);
	_graphics = new Graphics(this);
	_grid = new Grid(this);
	_input = new Input(_resource->getLanguage(), _system);

	if (_resource->isDemo()) {
		_logic = new LogicDemo(this);
	} else if (_resource->isInterview()) {
		_logic = new LogicInterview(this);
	} else {
		_logic = new LogicGame(this);
	}

	if (!_mixer->isReady())
		warning("Sound initialisation failed");
	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, ConfMan.getInt("sfx_volume"));
	// Set mixer music volume to maximum, since music volume is regulated by MusicPlayer's MIDI messages
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, Audio::Mixer::kMaxMixerVolume);

	_sound = Sound::makeSoundInstance(_mixer, this, _resource->getCompression());
	_walk = new Walk(this);
	//_talkspeedScale = (MAX_TEXT_SPEED - MIN_TEXT_SPEED) / 255.0;

	registerDefaultSettings();
	readOptionSettings();

	return 0;
}

} // End of namespace Queen
