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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/engines/queen/input.h $
 * $Id: input.h 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

#ifndef QUEEN_INPUT_H
#define QUEEN_INPUT_H

#include "common/util.h"
#include "common/rect.h"
#include "queen/defs.h"

class OSystem;

namespace Common {
	class EventManager;
}

namespace Queen {

class Input {
public:

	//! Adjust here to change delays!
	enum {
		DELAY_SHORT  =  10,
		DELAY_NORMAL = 100, // 5 * 20ms
		DELAY_SCREEN_BLANKER = 5 * 60 * 1000
	};
	enum {
		MOUSE_LBUTTON = 1,
		MOUSE_RBUTTON = 2
	};

	Input(Common::Language language, OSystem *system);

	void delay(uint amount);

	//! convert input to verb
	int checkKeys();

	//! use instead of KEYVERB=0
	void clearKeyVerb()  { _keyVerb = VERB_NONE; }

	void canQuit(bool cq)             { _canQuit = cq; }

	bool cutawayRunning() const       { return _cutawayRunning; }
	void cutawayRunning(bool running) { _cutawayRunning = running; }

	bool cutawayQuit() const  { return _cutawayQuit; }
	void cutawayQuitReset()   { _cutawayQuit = false; }

	void dialogueRunning(bool running) { _dialogueRunning = running; }

	bool talkQuit() const { return _talkQuit; }
	void talkQuitReset()  { _talkQuit = false; }

	bool quickSave() const { return _quickSave; }
	void quickSaveReset()  { _quickSave = false; }
	bool quickLoad() const { return _quickLoad; }
	void quickLoadReset()  { _quickLoad = false; }
	bool debugger() const { return _debugger; }
	void debuggerReset() { _debugger = false; }

	bool fastMode() const { return _fastMode; }
	void fastMode(bool fm)	{ _fastMode = fm; }

	Verb keyVerb() const { return _keyVerb; }

	Common::Point getMousePos() const;

	int mouseButton() const { return _mouseButton; }
	void clearMouseButton() { _mouseButton = 0; }

	//! returns user idle time (used by Display, to trigger the screensaver)
	uint32 idleTime() const { return _idleTime; }

private:

	enum KeyCode {
		KEY_SPACE = ' ',
		KEY_COMMA = ',',
		KEY_DOT   = '.',

		KEY_DIGIT_1 = '1',
		KEY_DIGIT_2 = '2',
		KEY_DIGIT_3 = '3',
		KEY_DIGIT_4 = '4',

		KEY_ESCAPE    = 27,
		KEY_RETURN    = 13,
		KEY_BACKSPACE = 8,

		KEY_F1 = 282,
		KEY_F11 = KEY_F1 + 10,
		KEY_F5 = KEY_F1 + 4,
		KEY_F12
	};

	//! used to get keyboard and mouse events
	OSystem *_system;

	Common::EventManager *_eventMan;

	//! some cutaways require update() run faster
	bool _fastMode;

	//! the current verb received from keyboard
	Verb _keyVerb;

	//! set if a cutaway is running
	bool _cutawayRunning;

	//! set this if we can quit
	bool _canQuit;

	//! moved Cutaway::_quit here
	bool _cutawayQuit;

	//! set if a dialogue is running
	bool _dialogueRunning;

	//! moved Talk::_quit here
	bool _talkQuit;

	//! set if quicksave requested
	bool _quickSave;

	//! set if quickload requested
	bool _quickLoad;

	//! set if debugger requested
	bool _debugger;

	//! set by delay();
	int _inKey;

	//! set by delay();
	int _mouseButton;

	//! user idle time
	uint32 _idleTime;

	//! command keys for current language
	const char *_currentCommandKeys;

	//! command keys for all languages
	static const char *_commandKeys[];

	//! verbs matching the command keys
	static const Verb _verbKeys[];
};

} // End of namespace Queen

#endif
