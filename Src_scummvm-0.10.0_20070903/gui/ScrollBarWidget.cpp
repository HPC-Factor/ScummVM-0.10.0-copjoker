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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/gui/ScrollBarWidget.cpp $
 * $Id: ScrollBarWidget.cpp 27024 2007-05-30 21:56:52Z fingolfin $
 */

#include "common/stdafx.h"
#include "ScrollBarWidget.h"
#include "gui/dialog.h"
#include "gui/newgui.h"

namespace GUI {

#define UP_DOWN_BOX_HEIGHT	(_w+1)

ScrollBarWidget::ScrollBarWidget(GuiObject *boss, int x, int y, int w, int h)
	: Widget (boss, x, y, w, h), CommandSender(boss) {
	_flags = WIDGET_ENABLED | WIDGET_TRACK_MOUSE | WIDGET_CLEARBG | WIDGET_WANT_TICKLE;
	_type = kScrollBarWidget;

	_part = kNoPart;
	_sliderHeight = 0;
	_sliderPos = 0;

	_draggingPart = kNoPart;
	_sliderDeltaMouseDownPos = 0;

	_numEntries = 0;
	_entriesPerPage = 0;
	_currentPos = 0;
}

void ScrollBarWidget::handleMouseDown(int x, int y, int button, int clickCount) {
	int old_pos = _currentPos;

	// Do nothing if there are less items than fit on one page
	if (_numEntries <= _entriesPerPage)
		return;

	if (y <= UP_DOWN_BOX_HEIGHT) {
		// Up arrow
		_currentPos--;
		_draggingPart = kUpArrowPart;
	} else if (y >= _h - UP_DOWN_BOX_HEIGHT) {
		// Down arrow
		_currentPos++;
		_draggingPart = kDownArrowPart;
	} else if (y < _sliderPos) {
		_currentPos -= _entriesPerPage;
	} else if (y >= _sliderPos + _sliderHeight) {
		_currentPos += _entriesPerPage;
	} else {
		_draggingPart = kSliderPart;
		_sliderDeltaMouseDownPos = y - _sliderPos;
	}

	// Make sure that _currentPos is still inside the bounds
	checkBounds(old_pos);
}

void ScrollBarWidget::handleMouseUp(int x, int y, int button, int clickCount) {
	_draggingPart = kNoPart;
}

void ScrollBarWidget::handleMouseWheel(int x, int y, int direction) {
	int old_pos = _currentPos;

	if (_numEntries < _entriesPerPage)
		return;

	if (direction < 0) {
		_currentPos--;
	} else {
		_currentPos++;
	}

	// Make sure that _currentPos is still inside the bounds
	checkBounds(old_pos);
}

void ScrollBarWidget::handleMouseMoved(int x, int y, int button) {
	// Do nothing if there are less items than fit on one page
	if (_numEntries <= _entriesPerPage)
		return;

	if (_draggingPart == kSliderPart) {
		int old_pos = _currentPos;
		_sliderPos = y - _sliderDeltaMouseDownPos;

		if (_sliderPos < UP_DOWN_BOX_HEIGHT)
			_sliderPos = UP_DOWN_BOX_HEIGHT;

		if (_sliderPos > _h - UP_DOWN_BOX_HEIGHT - _sliderHeight)
			_sliderPos = _h - UP_DOWN_BOX_HEIGHT - _sliderHeight;

		_currentPos =
			(_sliderPos - UP_DOWN_BOX_HEIGHT) * (_numEntries - _entriesPerPage) / (_h - 2 * UP_DOWN_BOX_HEIGHT - _sliderHeight);
		checkBounds(old_pos);
	} else {
		int old_part = _part;

		if (y <= UP_DOWN_BOX_HEIGHT)	// Up arrow
			_part = kUpArrowPart;
		else if (y >= _h - UP_DOWN_BOX_HEIGHT)	// Down arrow
			_part = kDownArrowPart;
		else if (y < _sliderPos)
			_part = kPageUpPart;
		else if (y >= _sliderPos + _sliderHeight)
			_part = kPageDownPart;
		else
			_part = kSliderPart;

		if (old_part != _part)
			draw();
	}
}

void ScrollBarWidget::handleTickle() {
/*
	// FIXME/TODO - this code is supposed to allow for "click-repeat" (like key repeat),
	// i.e. if you click on one of the arrows and keep clicked, it will scroll
	// continuously. However, just like key repeat, this requires two delays:
	// First an "initial" delay that has to pass before repeating starts (otherwise
	// it is near to impossible to achieve single clicks). Secondly, a repeat delay
	// that determines how often per second a click is simulated.
	int old_pos = _currentPos;

	if (_draggingPart == kUpArrowPart)
		_currentPos--;
	else if (_draggingPart == kDownArrowPart)
		_currentPos++;

	// Make sure that _currentPos is still inside the bounds
	checkBounds(old_pos);
*/
}

void ScrollBarWidget::checkBounds(int old_pos) {
	if (_numEntries <= _entriesPerPage || _currentPos < 0)
		_currentPos = 0;
	else if (_currentPos > _numEntries - _entriesPerPage)
		_currentPos = _numEntries - _entriesPerPage;

	if (old_pos != _currentPos) {
		recalc();
		draw();
		sendCommand(kSetPositionCmd, _currentPos);
	}
}

void ScrollBarWidget::recalc() {
	if (_numEntries > _entriesPerPage) {
		_sliderHeight = (_h - 2 * UP_DOWN_BOX_HEIGHT) * _entriesPerPage / _numEntries;
		if (_sliderHeight < UP_DOWN_BOX_HEIGHT)
			_sliderHeight = UP_DOWN_BOX_HEIGHT;

		_sliderPos =
			UP_DOWN_BOX_HEIGHT + (_h - 2 * UP_DOWN_BOX_HEIGHT - _sliderHeight) * _currentPos / (_numEntries - _entriesPerPage);
		if (_sliderPos < 0)
			_sliderPos = 0;
		clearFlags(WIDGET_INVISIBLE);
	} else {
		_sliderHeight = _h - 2 * UP_DOWN_BOX_HEIGHT;
		_sliderPos = UP_DOWN_BOX_HEIGHT;
		setFlags(WIDGET_INVISIBLE);
	}
}

void ScrollBarWidget::drawWidget(bool hilite) {
	if (_draggingPart != kNoPart)
		_part = _draggingPart;

	Theme::ScrollbarState state = Theme::kScrollbarStateNo;
	if (_numEntries <= _entriesPerPage) {
		state = Theme::kScrollbarStateSinglePage;
	} else if (_part == kUpArrowPart) {
		state = Theme::kScrollbarStateUp;
	} else if (_part == kDownArrowPart) {
		state = Theme::kScrollbarStateDown;
	} else if (_part == kSliderPart) {
		state = Theme::kScrollbarStateSlider;
	}

	g_gui.theme()->drawScrollbar(Common::Rect(_x, _y, _x+_w, _y+_h), _sliderPos, _sliderHeight, state,
								isEnabled() ? (hilite ? Theme::kStateHighlight : Theme::kStateEnabled) : Theme::kStateDisabled);
}

} // End of namespace GUI
