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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/engines/agi/cycle.cpp $
 * $Id: cycle.cpp 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

#include "common/stdafx.h"

#include "agi/agi.h"
#include "agi/sprite.h"
#include "agi/graphics.h"
#include "agi/keyboard.h"
#include "agi/menu.h"

namespace Agi {

/**
 * Set up new room.
 * This function is called when ego enters a new room.
 * @param n room number
 */
void AgiEngine::newRoom(int n) {
	VtEntry *v;
	int i;

	debugC(4, kDebugLevelMain, "*** room %d ***", n);
	_sound->stopSound();

	i = 0;
	for (v = _game.viewTable; v < &_game.viewTable[MAX_VIEWTABLE]; v++) {
		v->entry = i++;
		v->flags &= ~(ANIMATED | DRAWN);
		v->flags |= UPDATE;
		v->stepTime = 1;
		v->stepTimeCount = 1;
		v->cycleTime = 1;
		v->cycleTimeCount = 1;
		v->stepSize = 1;
	}
	agiUnloadResources();

	_game.playerControl = true;
	_game.block.active = false;
	_game.horizon = 36;
	_game.vars[vPrevRoom] = _game.vars[vCurRoom];
	_game.vars[vCurRoom] = n;
	_game.vars[vBorderTouchObj] = 0;
	_game.vars[vBorderCode] = 0;
	_game.vars[vEgoViewResource] = _game.viewTable[0].currentView;

	agiLoadResource(rLOGIC, n);

	/* Reposition ego in the new room */
	switch (_game.vars[vBorderTouchEgo]) {
	case 1:
		_game.viewTable[0].yPos = _HEIGHT - 1;
		break;
	case 2:
		_game.viewTable[0].xPos = 0;
		break;
	case 3:
		_game.viewTable[0].yPos = HORIZON + 1;
		break;
	case 4:
		_game.viewTable[0].xPos = _WIDTH - _game.viewTable[0].xSize;
		break;
	}

	_game.vars[vBorderTouchEgo] = 0;
	setflag(fNewRoomExec, true);

	_game.exitAllLogics = true;

	writeStatus();
	writePrompt();
}

void AgiEngine::resetControllers() {
	int i;

	for (i = 0; i < MAX_DIRS; i++) {
		_game.evKeyp[i].occured = false;
	}
}

void AgiEngine::interpretCycle() {
	int oldSound, oldScore;

	if (_game.playerControl)
		_game.vars[vEgoDir] = _game.viewTable[0].direction;
	else
		_game.viewTable[0].direction = _game.vars[vEgoDir];

	checkAllMotions();

	oldScore = _game.vars[vScore];
	oldSound = getflag(fSoundOn);

	_game.exitAllLogics = false;
	while (runLogic(0) == 0 && !_game.quitProgNow) {
		_game.vars[vWordNotFound] = 0;
		_game.vars[vBorderTouchObj] = 0;
		_game.vars[vBorderCode] = 0;
		oldScore = _game.vars[vScore];
		setflag(fEnteredCli, false);
		_game.exitAllLogics = false;
		resetControllers();
	}
	resetControllers();

	_game.viewTable[0].direction = _game.vars[vEgoDir];

	if (_game.vars[vScore] != oldScore || getflag(fSoundOn) != oldSound)
		writeStatus();

	_game.vars[vBorderTouchObj] = 0;
	_game.vars[vBorderCode] = 0;
	setflag(fNewRoomExec, false);
	setflag(fRestartGame, false);
	setflag(fRestoreJustRan, false);

	if (_game.gfxMode) {
		updateViewtable();
		_gfx->doUpdate();
	}
}

/**
 * Update AGI interpreter timer.
 */
void AgiEngine::updateTimer() {
	_clockCount++;
	if (_clockCount <= TICK_SECONDS)
		return;

	_clockCount -= TICK_SECONDS;

	if (!_game.clockEnabled)
		return;

	setvar(vSeconds, getvar(vSeconds) + 1);
	if (getvar(vSeconds) < 60)
		return;

	setvar(vSeconds, 0);
	setvar(vMinutes, getvar(vMinutes) + 1);
	if (getvar(vMinutes) < 60)
		return;

	setvar(vMinutes, 0);
	setvar(vHours, getvar(vHours) + 1);
	if (getvar(vHours) < 24)
		return;

	setvar(vHours, 0);
	setvar(vDays, getvar(vDays) + 1);
}

void AgiEngine::newInputMode(int i) {
	_oldMode = _game.inputMode;
	_game.inputMode = i;
}

void AgiEngine::oldInputMode() {
	_game.inputMode = _oldMode;
}

/* If main_cycle returns false, don't process more events! */
int AgiEngine::mainCycle() {
	unsigned int key, kascii;
	VtEntry *v = &_game.viewTable[0];

	_gfx->pollTimer();		/* msdos driver -> does nothing */
	updateTimer();

	if (_game.ver == 0) {
		messageBox("Warning: game CRC not listed, assuming AGI version 2.917.");
		_game.ver = -1;
	}

	key = doPollKeyboard();

	/* In AGI Mouse emulation mode we must update the mouse-related
	 * vars in every interpreter cycle.
	 */
	if (getFeatures() & GF_AGIMOUSE) {
		_game.vars[28] = g_mouse.x / 2;
		_game.vars[29] = g_mouse.y;
	}
	if (key == KEY_PRIORITY) {
		_sprites->eraseBoth();
		_debug.priority = !_debug.priority;
		_picture->showPic();
		_sprites->blitBoth();
		_sprites->commitBoth();
		key = 0;
	}

	if (key == KEY_STATUSLN) {
		_debug.statusline = !_debug.statusline;
		writeStatus();
		key = 0;
	}

	/* Click-to-walk mouse interface */
	if (_game.playerControl && v->flags & ADJ_EGO_XY) {
		v->direction = getDirection(v->xPos, v->yPos, v->parm1, v->parm2, v->stepSize);

		if (v->direction == 0)
			inDestination(v);
	}

	kascii = KEY_ASCII(key);

	if (kascii)
		setvar(vKey, kascii);

process_key:

	switch (_game.inputMode) {
	case INPUT_NORMAL:
		if (!handleController(key)) {
			if (key == 0 || !_game.inputEnabled)
				break;
			handleKeys(key);

			/* if ESC pressed, activate menu before
			 * accept.input from the interpreter cycle
			 * sets the input mode to normal again
			 * (closes: #540856)
			 */
			if (key == KEY_ESCAPE) {
				key = 0;
				goto process_key;
			}

			/* commented out to close Sarien bug #438872
			 * if (key) game.keypress = key;
			 */
		}
		break;
	case INPUT_GETSTRING:
		handleController(key);
		handleGetstring(key);
		setvar(vKey, 0);	/* clear ENTER key */
		break;
	case INPUT_MENU:
		_menu->keyhandler(key);
		_gfx->doUpdate();
		return false;
	case INPUT_NONE:
		handleController(key);
		if (key)
			_game.keypress = key;
		break;
	}
	_gfx->doUpdate();

	if (_game.msgBoxTicks > 0)
		_game.msgBoxTicks--;

	return true;
}

int AgiEngine::playGame() {
	int ec = errOK;

	debugC(2, kDebugLevelMain, "initializing...");
	debugC(2, kDebugLevelMain, "game.ver = 0x%x", _game.ver);

	_sound->stopSound();
	_gfx->clearScreen(0);

	_game.horizon = HORIZON;
	_game.playerControl = false;

	setflag(fLogicZeroFirsttime, true);	/* not in 2.917 */
	setflag(fNewRoomExec, true);	/* needed for MUMG and SQ2! */
	setflag(fSoundOn, true);	/* enable sound */
	setvar(vTimeDelay, 2);	/* "normal" speed */

	_game.gfxMode = true;
	_game.quitProgNow = false;
	_game.clockEnabled = true;
	_game.lineUserInput = 22;

	if (getFeatures() & GF_AGIMOUSE)
		report("Using AGI Mouse 1.0 protocol\n");

	if (getFeatures() & GF_AGIPAL)
		debug(1, "Running AGIPAL game");

	report("Running AGI script.\n");

	setflag(fEnteredCli, false);
	setflag(fSaidAcceptedInput, false);
	_game.vars[vWordNotFound] = 0;
	_game.vars[vKey] = 0;

	debugC(2, kDebugLevelMain, "Entering main loop");
	do {

		if (!mainCycle())
			continue;

		if (getvar(vTimeDelay) == 0 || (1 + _clockCount) % getvar(vTimeDelay) == 0) {
			if (!_game.hasPrompt && _game.inputMode == INPUT_NORMAL) {
				writePrompt();
				_game.hasPrompt = 1;
			} else if (_game.hasPrompt && _game.inputMode == INPUT_NONE) {
				writePrompt();
				_game.hasPrompt = 0;
			}

			interpretCycle();

			setflag(fEnteredCli, false);
			setflag(fSaidAcceptedInput, false);
			_game.vars[vWordNotFound] = 0;
			_game.vars[vKey] = 0;
		}

		if (_game.quitProgNow == 0xff)
			ec = errRestartGame;

	} while (_game.quitProgNow == 0);

	_sound->stopSound();

	return ec;
}

int AgiEngine::runGame() {
	int i, ec = errOK;

	for (i = 0; i < MAX_DIRS; i++)
		memset(&_game.evKeyp[i], 0, sizeof(struct AgiEvent));

	/* Execute the game */
	do {
		debugC(2, kDebugLevelMain, "game loop");
		debugC(2, kDebugLevelMain, "game.ver = 0x%x", _game.ver);

		if (agiInit() != errOK)
			break;
		if (ec == errRestartGame)
			setflag(fRestartGame, true);

		setvar(vComputer, 0);	/* IBM PC (4 = Atari ST) */
		setvar(vSoundgen, 1);	/* IBM PC SOUND */
		setvar(vMonitor, 0x3);	/* EGA monitor */
		setvar(vMaxInputChars, 38);
		_game.inputMode = INPUT_NONE;
		_game.inputEnabled = 0;
		_game.hasPrompt = 0;

		_game.state = STATE_RUNNING;
		ec = playGame();
		_game.state = STATE_LOADED;
		agiDeinit();
	} while (ec == errRestartGame);

	delete _menu;
	_menu = NULL;

	releaseImageStack();

	return ec;
}

} // End of namespace Agi
