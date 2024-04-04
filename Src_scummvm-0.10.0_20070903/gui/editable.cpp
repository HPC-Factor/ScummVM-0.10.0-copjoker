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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/gui/editable.cpp $
 * $Id: editable.cpp 27024 2007-05-30 21:56:52Z fingolfin $
 */

#include "common/stdafx.h"
#include "gui/editable.h"
#include "gui/newgui.h"

namespace GUI {

EditableWidget::EditableWidget(GuiObject *boss, int x, int y, int w, int h)
 : Widget(boss, x, y, w, h) {
	init();
}

EditableWidget::EditableWidget(GuiObject *boss, const String &name)
 : Widget(boss, name) {
	init();
}

void EditableWidget::init() {
	_caretVisible = false;
	_caretTime = 0;
	_caretPos = 0;	// FIXME

	_caretInverse = false;

	_editScrollOffset = 0;

	_font = Theme::kFontStyleBold;
}

EditableWidget::~EditableWidget() {
}

void EditableWidget::reflowLayout() {
	Widget::reflowLayout();

	_editScrollOffset = g_gui.getStringWidth(_editString, _font) - getEditRect().width();
	if (_editScrollOffset < 0)
		_editScrollOffset = 0;
}

void EditableWidget::setEditString(const String &str) {
	// TODO: We probably should filter the input string here,
	// e.g. using tryInsertChar.
	_editString = str;
	_caretPos = _editString.size();
}

bool EditableWidget::tryInsertChar(byte c, int pos) {
	if ((c >= 32 && c <= 127) || c >= 160) {
		_editString.insertChar(c, pos);
		return true;
	}
	return false;
}

void EditableWidget::handleTickle() {
	uint32 time = getMillis();
	if (_caretTime < time) {
		_caretTime = time + kCaretBlinkTime;
		drawCaret(_caretVisible);
	}
}

bool EditableWidget::handleKeyDown(uint16 ascii, int keycode, int modifiers) {
	bool handled = true;
	bool dirty = false;
	bool forcecaret = false;

	// First remove caret
	if (_caretVisible)
		drawCaret(true);

	switch (keycode) {
	case '\n':	// enter/return
	case '\r':
		// confirm edit and exit editmode
		endEditMode();
		dirty = true;
		break;
	case 27:	// escape
		abortEditMode();
		dirty = true;
		break;
	case 8:		// backspace
		if (_caretPos > 0) {
			_caretPos--;
			_editString.deleteChar(_caretPos);
			dirty = true;
		}
		forcecaret = true;
		break;
	case 127:	// delete
		if (_caretPos < (int)_editString.size()) {
			_editString.deleteChar(_caretPos);
			dirty = true;
		}
		forcecaret = true;
		break;
	case 256 + 20:	// left arrow
		if (_caretPos > 0) {
			dirty = setCaretPos(_caretPos - 1);
		}
		forcecaret = true;
		dirty = true;
		break;
	case 256 + 19:	// right arrow
		if (_caretPos < (int)_editString.size()) {
			dirty = setCaretPos(_caretPos + 1);
		}
		forcecaret = true;
		dirty = true;
		break;
	case 256 + 22:	// home
		dirty = setCaretPos(0);
		forcecaret = true;
		break;
	case 256 + 23:	// end
		dirty = setCaretPos(_editString.size());
		forcecaret = true;
		break;
	default:
		if (tryInsertChar((byte)ascii, _caretPos)) {
			_caretPos++;
			dirty = true;
			forcecaret = true;
		} else {
			handled = false;
		}
	}

	if (dirty)
		draw();

	if (forcecaret)
		makeCaretVisible();

	return handled;
}

int EditableWidget::getCaretOffset() const {
	int caretpos = 0;
	for (int i = 0; i < _caretPos; i++)
		caretpos += g_gui.getCharWidth(_editString[i], _font);

	caretpos -= _editScrollOffset;

	return caretpos;
}

void EditableWidget::drawCaret(bool erase) {
	// Only draw if item is visible
	if (!isVisible() || !_boss->isVisible())
		return;

	Common::Rect editRect = getEditRect();

	int x = editRect.left;
	int y = editRect.top + 1;

	x += getCaretOffset();

	if (y < 0 || y + editRect.height() - 2 >= _h)
		return;

	x += getAbsX();
	y += getAbsY();

	g_gui.theme()->drawCaret(Common::Rect(x, y, x + 1, y + editRect.height() - 2), erase);
	
	_caretVisible = !erase;
}

bool EditableWidget::setCaretPos(int newPos) {
	assert(newPos >= 0 && newPos <= (int)_editString.size());
	_caretPos = newPos;
	return adjustOffset();
}

bool EditableWidget::adjustOffset() {
	// check if the caret is still within the textbox; if it isn't,
	// adjust _editScrollOffset

	int caretpos = getCaretOffset();
	const int editWidth = getEditRect().width();

	if (caretpos < 0) {
		// scroll left
		_editScrollOffset += caretpos;
		return true;
	} else if (caretpos >= editWidth) {
		// scroll right
		_editScrollOffset -= (editWidth - caretpos);
		return true;
	} else if (_editScrollOffset > 0) {
		const int strWidth = g_gui.getStringWidth(_editString, _font);
		if (strWidth - _editScrollOffset < editWidth) {
			// scroll right
			_editScrollOffset = (strWidth - editWidth);
			if (_editScrollOffset < 0)
				_editScrollOffset = 0;
		}
	}

	return false;
}

void EditableWidget::makeCaretVisible() {
	_caretTime = getMillis() + kCaretBlinkTime;
	_caretVisible = true;
	drawCaret(false);
}

} // End of namespace GUI
