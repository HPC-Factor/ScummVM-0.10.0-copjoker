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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/engines/engine.cpp $
 * $Id: engine.cpp 27024 2007-05-30 21:56:52Z fingolfin $
 */

#if defined(WIN32)
#include <windows.h>
#include <direct.h>
// winnt.h defines ARRAYSIZE, but we want our own one...
#undef ARRAYSIZE
#endif

#include "common/stdafx.h"
#include "engines/engine.h"
#include "common/config-manager.h"
#include "common/file.h"
#include "common/timer.h"
#include "common/savefile.h"
#include "common/system.h"
#include "gui/message.h"
#include "sound/mixer.h"

#ifdef _WIN32_WCE
extern bool isSmartphone(void);
#endif

// FIXME - BIG HACK for MidiEmu & error()
Engine *g_engine = 0;


Engine::Engine(OSystem *syst)
	: _system(syst),
		_mixer(_system->getMixer()),
		_timer(_system->getTimerManager()),
		_eventMan(_system->getEventManager()),
		_saveFileMan(_system->getSavefileManager()),
		_targetName(ConfMan.getActiveDomainName()),
		_gameDataPath(ConfMan.get("path")) {

	g_engine = this;
	_autosavePeriod = ConfMan.getInt("autosave_period");
}

Engine::~Engine() {
	_mixer->stopAll();

	g_engine = NULL;
}

void Engine::initCommonGFX(bool defaultTo1XScaler) {
	const Common::ConfigManager::Domain *transientDomain = ConfMan.getDomain(Common::ConfigManager::kTransientDomain);
	const Common::ConfigManager::Domain *gameDomain = ConfMan.getActiveDomain();

	assert(transientDomain);

	const bool useDefaultGraphicsMode =
		!transientDomain->contains("gfx_mode") &&
		(
		!gameDomain ||
		!gameDomain->contains("gfx_mode") ||
		!scumm_stricmp(gameDomain->get("gfx_mode").c_str(), "normal") ||
		!scumm_stricmp(gameDomain->get("gfx_mode").c_str(), "default")
		);

	// See if the game should default to 1x scaler
	if (useDefaultGraphicsMode && defaultTo1XScaler) {
		// FIXME: As a hack, we use "1x" here. Would be nicer to use
		// getDefaultGraphicsMode() instead, but right now, we do not specify
		// whether that is a 1x scaler or not...
		_system->setGraphicsMode("1x");
	} else {
		// Override global scaler with any game-specific define
		if (ConfMan.hasKey("gfx_mode")) {
			_system->setGraphicsMode(ConfMan.get("gfx_mode").c_str());
		}
	}

	// Note: The following code deals with the fullscreen / ASR settings. This
	// is a bit tricky, because there are three ways the user can affect these
	// settings: Via the config file, via the command line, and via in-game
	// hotkeys.
	// Any global or command line settings already have been applied at the time
	// we get here. Hence we only do something

	// (De)activate aspect-ratio correction as determined by the config settings
	if (gameDomain && gameDomain->contains("aspect_ratio"))
		_system->setFeatureState(OSystem::kFeatureAspectRatioCorrection, ConfMan.getBool("aspect_ratio"));

	// (De)activate fullscreen mode as determined by the config settings
	if (gameDomain && gameDomain->contains("fullscreen"))
		_system->setFeatureState(OSystem::kFeatureFullscreenMode, ConfMan.getBool("fullscreen"));
}

void Engine::checkCD() {
#if defined (WIN32) && !defined(_WIN32_WCE) && !defined(__SYMBIAN32__)
	// It is a known bug under Windows that games that play CD audio cause
	// ScummVM to crash if the data files are read from the same CD. Check
	// if this appears to be the case and issue a warning.

	// If we can find a compressed audio track, then it should be ok even
	// if it's running from CD.

#ifdef USE_VORBIS
	if (Common::File::exists("track1.ogg"))
		return;
#endif
#ifdef USE_FLAC
	if (Common::File::exists("track1.fla") || Common::File::exists("track1.flac"))
		return;
#endif
#ifdef USE_MAD
	if (Common::File::exists("track1.mp3"))
		return;
#endif

	char buffer[MAXPATHLEN];
	int i;

	if (strlen(_gameDataPath.c_str()) == 0) {
		// That's it! I give up!
		if (getcwd(buffer, MAXPATHLEN) == NULL)
			return;
	} else
		strncpy(buffer, _gameDataPath.c_str(), MAXPATHLEN);

	for (i = 0; i < MAXPATHLEN - 1; i++) {
		if (buffer[i] == '\\')
			break;
	}

	buffer[i + 1] = 0;

	if (GetDriveType(buffer) == DRIVE_CDROM) {
		GUI::MessageDialog dialog(
			"You appear to be playing this game directly\n"
			"from the CD. This is known to cause problems,\n"
			"and it's therefore recommended that you copy\n"
			"the data files to your hard disk instead.\n"
			"See the README file for details.", "OK");
		dialog.runModal();
	}
#endif
}

bool Engine::shouldPerformAutoSave(int lastSaveTime) {
	const int diff = _system->getMillis() - lastSaveTime;
	return _autosavePeriod != 0 && diff > _autosavePeriod * 1000;
}

void Engine::errorString(const char *buf1, char *buf2) {
	strcpy(buf2, buf1);
}

void Engine::GUIErrorMessage(const Common::String msg) {
	_system->setWindowCaption("Error");
	_system->beginGFXTransaction();
		initCommonGFX(false);
		_system->initSize(320, 200);
	_system->endGFXTransaction();

	GUI::MessageDialog dialog(msg);
	dialog.runModal();
}
