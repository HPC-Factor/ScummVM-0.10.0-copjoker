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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/backends/events/default/default-events.cpp $
 * $Id: default-events.cpp 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

#if !defined(DISABLE_DEFAULT_EVENTMANAGER)

#include "common/stdafx.h"
#include "common/system.h"
#include "backends/events/default/default-events.h"

DefaultEventManager::DefaultEventManager(OSystem *boss) :
	_boss(boss),
	_buttonState(0),
	_modifierState(0),
	_shouldQuit(false) {

 	assert(_boss);

	// Reset key repeat
	_currentKeyDown.keycode = 0;
}

bool DefaultEventManager::pollEvent(Common::Event &event) {
	uint32 time = _boss->getMillis();
	bool result;
	
	result = _boss->pollEvent(event);
	
	if (result) {
		event.synthetic = false;
		switch (event.type) {
		case Common::EVENT_KEYDOWN:
			_modifierState = event.kbd.flags;

			// init continuous event stream
			// not done on PalmOS because keyboard is emulated and keyup is not generated
#if !defined(PALMOS_MODE)
			_currentKeyDown.ascii = event.kbd.ascii;
			_currentKeyDown.keycode = event.kbd.keycode;
			_currentKeyDown.flags = event.kbd.flags;
			_keyRepeatTime = time + kKeyRepeatInitialDelay;
#endif
			break;
		case Common::EVENT_KEYUP:
			_modifierState = event.kbd.flags;
			if (event.kbd.keycode == _currentKeyDown.keycode) {
				// Only stop firing events if it's the current key
				_currentKeyDown.keycode = 0;
			}
			break;

		case Common::EVENT_MOUSEMOVE:
			_mousePos = event.mouse;
			break;

		case Common::EVENT_LBUTTONDOWN:
			_mousePos = event.mouse;
			_buttonState |= LBUTTON;
			break;
		case Common::EVENT_LBUTTONUP:
			_mousePos = event.mouse;
			_buttonState &= ~LBUTTON;
			break;

		case Common::EVENT_RBUTTONDOWN:
			_mousePos = event.mouse;
			_buttonState |= RBUTTON;
			break;
		case Common::EVENT_RBUTTONUP:
			_mousePos = event.mouse;
			_buttonState &= ~RBUTTON;
			break;

		case Common::EVENT_QUIT:
			_shouldQuit = true;
			break;

		default:
			break;
		}
	} else {
		// Check if event should be sent again (keydown)
		if (_currentKeyDown.keycode != 0 && _keyRepeatTime < time) {
			// fire event
			event.type = Common::EVENT_KEYDOWN;
			event.synthetic = true;
			event.kbd.ascii = _currentKeyDown.ascii;
			event.kbd.keycode = _currentKeyDown.keycode;
			event.kbd.flags = _currentKeyDown.flags;
			_keyRepeatTime = time + kKeyRepeatSustainDelay;
			result = true;
		}
	}
	
	return result;
}

#endif // !defined(DISABLE_DEFAULT_EVENTMANAGER)
