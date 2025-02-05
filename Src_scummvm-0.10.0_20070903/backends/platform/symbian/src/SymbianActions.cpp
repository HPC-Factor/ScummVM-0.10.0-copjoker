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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/backends/platform/symbian/src/SymbianActions.cpp $
 * $Id: SymbianActions.cpp 27255 2007-06-09 21:13:32Z anotherguest $
 *
 */

#include "common/stdafx.h"
#include "backends/platform/symbian/src/SymbianActions.h"

#include "gui/message.h"
#include "scumm/scumm.h"
#include "common/config-manager.h"

#include <sdl.h>

namespace GUI {

// SumthinWicked says: we either split our Actions like WinCE did with Pocket/Smartphone
// or we put them in this file separated by #ifdefs, this one is up to you, AnotherGuest :)
 
const Common::String actionNames[] = { 
	"Up", 
	"Down", 
	"Left",
	"Right",
	"Left Click",
	"Right Click",
	"Save",
	"Skip",
	"Zone",
	"Multi Function",
	"Swap character",
	"Skip text",	
	"Pause", 
	"Fast mode",
	"Quit",
	"Debugger"
};

#ifdef UIQ
static const int ACTIONS_DEFAULT[ACTION_LAST] = { 0, 0, 0, 0, SDLK_F1, SDLK_F2, SDLK_F5, SDLK_PAGEDOWN, 0, 0, 0, SDLK_PAGEUP, 0, 0, 0, 0};
#elif defined (S60)
const int ACTIONS_DEFAULT[ACTION_LAST] = { 0, 0, 0, 0, 0, 0, '*', '#', '9', 0, 0, 0, 0, 0, 0, 0};
#elif defined (S90)
const int ACTIONS_DEFAULT[ACTION_LAST] = { SDLK_UP, SDLK_DOWN, SDLK_LEFT, SDLK_RIGHT, 0, 0, SDLK_MENU, SDLK_ESCAPE, 0, 0 , 0, 0, 0, 0, 0, 0};
#else
const int ACTIONS_DEFAULT[ACTION_LAST] = { SDLK_UP, SDLK_DOWN, SDLK_LEFT, SDLK_RIGHT, SDLK_F1, SDLK_F2, SDLK_MENU, SDLK_ESCAPE, 0, 0, 0, 0, 0, 0, 0, 0};
#endif

// creator function according to Factory Pattern
void SymbianActions::init() {
	_instance = new SymbianActions();
}


Common::String SymbianActions::actionName(ActionType action) {
	return actionNames[action];
}

int SymbianActions::size() {
	return ACTION_LAST;
}

Common::String SymbianActions::domain() {
	return Common::ConfigManager::kApplicationDomain;
}

int SymbianActions::version() {
	return ACTION_VERSION;
}

SymbianActions::SymbianActions()
 : Actions() {
	int i;

	for (i = 0; i < ACTION_LAST; i++) {
		_action_mapping[i] = ACTIONS_DEFAULT[i];
		_action_enabled[i] = false;
	}

}

void SymbianActions::initInstanceMain(OSystem *mainSystem) {
	Actions::initInstanceMain(mainSystem);
	
	// Mouse Up
	_action_enabled[ACTION_UP] = true;

	// Mouse Down
	_action_enabled[ACTION_DOWN] = true;

	// Mouse Left
	_action_enabled[ACTION_LEFT] = true;

	// Mouse Right
	_action_enabled[ACTION_RIGHT] = true;

	// Left Click
	_action_enabled[ACTION_LEFTCLICK] = true;

	// Right Click
	_action_enabled[ACTION_RIGHTCLICK] = true;

	// Skip
	_action_enabled[ACTION_SKIP] = true;
	_key_action[ACTION_SKIP].setAscii(SDLK_ESCAPE);
}

void SymbianActions::initInstanceGame() {
	Common::String gameid(ConfMan.get("gameid"));
	bool is_simon = (strncmp(gameid.c_str(), "simon", 5) == 0);
	bool is_sky = (strncmp(gameid.c_str(), "sky", 3) == 0);
	bool is_saga = (gameid == "saga");
	bool is_comi = (strncmp(gameid.c_str(), "comi", 4) == 0);
	bool is_queen = (strncmp(gameid.c_str(), "queen", 5) == 0);
	bool is_gob = (strncmp(gameid.c_str(), "gob", 3) == 0);
	bool is_kyra = (gameid == "kyra1");
	bool is_samnmax = (gameid == "samnmax");
	bool is_cine = (gameid == "cine");
	bool is_touche = (gameid == "touche");
	bool is_agi = (gameid == "agi");
	bool is_parallaction = (gameid == "parallaction");
	
	Actions::initInstanceGame();

	// Initialize keys for different actions
	// Save
	if (is_simon || is_gob || is_kyra || is_touche) 
		_action_enabled[ACTION_SAVE] = false;
	else {
		_action_enabled[ACTION_SAVE] = true;
		
		if (is_queen) {			
			_key_action[ACTION_SAVE].setAscii(SDLK_F1); // F1 key for FOTAQ
		} else if (is_sky) {		
			_key_action[ACTION_SAVE].setAscii(63); 
		} else if (is_cine) {			
			_key_action[ACTION_SAVE].setAscii(SDLK_F10); // F10
		} else if (is_agi) {		
			_key_action[ACTION_SAVE].setAscii(SDLK_ESCAPE);
		} else if (is_parallaction) {
			_key_action[ACTION_SAVE].setAscii(SDLK_s);
		} else {		
			_key_action[ACTION_SAVE].setAscii(SDLK_F5); // F5 key
		}
	}

	// Enable fast mode
	_action_enabled[ACTION_FASTMODE] = true;
	_key_action[ACTION_FASTMODE].setAscii('f');
	_key_action[ACTION_FASTMODE].setFlags(KMOD_CTRL);
	
	// Swap character
	_action_enabled[ACTION_SWAPCHAR] = true;
	_key_action[ACTION_SWAPCHAR].setAscii('b'); // b

	// Zone
	_action_enabled[ACTION_ZONE] = true;

	// Multi function key
	_action_enabled[ACTION_MULTI] = true;
	if (is_agi)
		_key_action[ACTION_MULTI].setAscii(SDLK_PAUSE); // agi: show predictive dialog
	else if (is_gob)
		_key_action[ACTION_MULTI].setAscii(315); // bargon : F1 to start
	else if (gameid == "atlantis")
		_key_action[ACTION_MULTI].setAscii(SDLK_KP0); // fate of atlantis : Ins to sucker-punch
	else
		_key_action[ACTION_MULTI].setAscii(86); // FT cheat : shift-V

	// Enable debugger
	_action_enabled[ACTION_DEBUGGER] = true;
	_key_action[ACTION_DEBUGGER].setAscii('d');
	_key_action[ACTION_DEBUGGER].setFlags(KMOD_CTRL);

	// Skip text
	if (!is_cine)
		_action_enabled[ACTION_SKIP_TEXT] = true;

	if (is_queen) {
		_key_action[ACTION_SKIP_TEXT].setAscii(SDLK_SPACE);
	} else {
		_key_action[ACTION_SKIP_TEXT].setAscii(SDLK_PERIOD);
	}

	// Pause
	_key_action[ACTION_PAUSE].setAscii(' ');
	_action_enabled[ACTION_PAUSE] = true;

	// Quit
	_action_enabled[ACTION_QUIT] = true;
}


SymbianActions::~SymbianActions() {
}

bool SymbianActions::perform(ActionType /*action*/, bool /*pushed*/) {

	return false;
}

} // namespace GUI
