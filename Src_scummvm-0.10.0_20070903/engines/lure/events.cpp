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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/engines/lure/events.cpp $
 * $Id: events.cpp 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

#include "common/stdafx.h"
#include "common/events.h"

#include "graphics/cursorman.h"

#include "lure/events.h"
#include "lure/res.h"

namespace Lure {

static Mouse *int_mouse = NULL;

Mouse &Mouse::getReference() { 
	return *int_mouse; 
}

Mouse::Mouse() {
	int_mouse = this;

	_lButton = false; 
	_rButton = false;
	_cursorNum = 0;
	_x = 0;
	_y = 0;
	setCursorNum(0);
}

Mouse::~Mouse() {
}

void Mouse::handleEvent(Common::Event event) {
	_x = (int16) event.mouse.x;
	_y = (int16) event.mouse.y;

	switch (event.type) {
	case Common::EVENT_LBUTTONDOWN:
		_lButton = true;
		break;
	case Common::EVENT_LBUTTONUP:
		_lButton = false;
		break;
	case Common::EVENT_RBUTTONDOWN:
		_rButton = true;
		break;
	case Common::EVENT_RBUTTONUP:
		_rButton = false;
		break;
	default:
		break;
	}
}


void Mouse::cursorOn() {
	CursorMan.showMouse(true);
}

void Mouse::cursorOff() {
	CursorMan.showMouse(false);
}

void Mouse::setCursorNum(uint8 cursorNum) {
	int hotspotX = 7, hotspotY = 7;
	if ((cursorNum == CURSOR_ARROW) || (cursorNum == CURSOR_MENUBAR)) {
		hotspotX = 0; 
		hotspotY = 0;
	}

	setCursorNum(cursorNum, hotspotX, hotspotY);
}

void Mouse::setCursorNum(uint8 cursorNum, int hotspotX, int hotspotY) {
	Resources &res = Resources::getReference();

	_cursorNum = cursorNum;
	byte *cursorAddr = res.getCursor(cursorNum);
	CursorMan.replaceCursor(cursorAddr, CURSOR_WIDTH, CURSOR_HEIGHT, hotspotX, hotspotY, 0);
}

void Mouse::pushCursorNum(uint8 cursorNum) {
	int hotspotX = 7, hotspotY = 7;
	if ((cursorNum == CURSOR_ARROW) || (cursorNum == CURSOR_MENUBAR)) {
		hotspotX = 0; 
		hotspotY = 0;
	}

	pushCursorNum(cursorNum, hotspotX, hotspotY);
}

void Mouse::pushCursorNum(uint8 cursorNum, int hotspotX, int hotspotY) {
	Resources &res = Resources::getReference();

	_cursorNum = cursorNum;
	byte *cursorAddr = res.getCursor(cursorNum);
	CursorMan.pushCursor(cursorAddr, CURSOR_WIDTH, CURSOR_HEIGHT, hotspotX, hotspotY, 0);
}

void Mouse::popCursor() {
	CursorMan.popCursor();
}

void Mouse::setPosition(int newX, int newY) {
	g_system->warpMouse(newX, newY);
}

void Mouse::waitForRelease() {
	Events &e = Events::getReference();

	do {
		e.pollEvent();
		g_system->delayMillis(20);
	} while (!e.quitFlag && (lButton() || rButton()));
}

/*--------------------------------------------------------------------------*/

static Events *int_events = NULL;

Events::Events() {
	int_events = this;
	quitFlag = false;
}

Events &Events::getReference() {
	return *int_events;
}


bool Events::pollEvent() {
	if (!g_system->getEventManager()->pollEvent(_event)) return false;

	// Handle keypress
	switch (_event.type) {
	case Common::EVENT_QUIT:
		quitFlag = true;
		break;

	case Common::EVENT_LBUTTONDOWN:
	case Common::EVENT_LBUTTONUP:
	case Common::EVENT_RBUTTONDOWN:
	case Common::EVENT_RBUTTONUP:
	case Common::EVENT_MOUSEMOVE:
	case Common::EVENT_WHEELUP:
	case Common::EVENT_WHEELDOWN:
		Mouse::getReference().handleEvent(_event);
		break;

	default:
 		break;
	}

	return true;
}

void Events::waitForPress() {
	bool keyButton = false;
	while (!keyButton) {
		if (pollEvent()) {
			if (_event.type == Common::EVENT_QUIT) return;
			else if (_event.type == Common::EVENT_KEYDOWN) keyButton = true;
			else if ((_event.type == Common::EVENT_LBUTTONDOWN) ||
				(_event.type == Common::EVENT_RBUTTONDOWN)) {
				keyButton = true;
				Mouse::getReference().waitForRelease();				
			}
		}
		g_system->delayMillis(20);
	}
}

} // end of namespace Lure
