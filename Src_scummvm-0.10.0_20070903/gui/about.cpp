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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/gui/about.cpp $
 * $Id: about.cpp 27392 2007-06-13 20:37:44Z fingolfin $
 */

#include "common/stdafx.h"
#include "engines/engine.h"
#include "base/plugins.h"
#include "base/version.h"
#include "common/events.h"
#include "common/system.h"
#include "common/util.h"
#include "gui/about.h"
#include "gui/eval.h"
#include "gui/newgui.h"
#include "gui/widget.h"

namespace GUI {

enum {
	kScrollStartDelay = 1500,
	kScrollMillisPerPixel = 60
};

// The following commands can be put at the start of a line (all subject to change):
//   \C, \L, \R  -- set center/left/right alignment
//   \c0 - \c4   -- set a custom color:
//                  0 normal text (green)
//                  1 highlighted text (light green)
//                  2 light border (light gray)
//                  3 dark border (dark gray)
//                  4 background (black)
// TODO: Maybe add a tab/indent feature; that is, make it possible to specify
// an amount by which that line shall be indented (the indent of course would have
// to be considered while performing any word wrapping, too).
//
// TODO: Add different font sizes (for bigger headlines)
// TODO: Allow color change in the middle of a line...

static const char *copyright_text[] = {
"\\C""",
"\\C""Copyright (C) 2002-2007 The ScummVM project",
"\\C""http://www.scummvm.org",
"\\C""",
"\\C""ScummVM is the legal property of its developers, whose names are too numerous to list here. Please refer to the COPYRIGHT file distributed with this binary.",
"\\C""",
};

static const char *gpl_text[] = {
"\\C""",
"\\C""This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.",
"\\C""",
"\\C""This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.",
"\\C""",
"\\C""You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.",
"\\C""",
};

#include "gui/credits.h"

AboutDialog::AboutDialog()
	: Dialog(10, 20, 300, 174),
	_scrollPos(0), _scrollTime(0), _willClose(false) {

	int i;

	const int screenW = g_system->getOverlayWidth();
	const int screenH = g_system->getOverlayHeight();

	_xOff = g_gui.evaluator()->getVar("aboutXOff");
	_yOff = g_gui.evaluator()->getVar("aboutYOff");
	int outerBorder = g_gui.evaluator()->getVar("aboutOuterBorder");

	_w = screenW - 2 * outerBorder;
	_h = screenH - 2 * outerBorder;

	_lineHeight = g_gui.getFontHeight() + 3;

	// Heuristic to compute 'optimal' dialog width
	int maxW = _w - 2*_xOff;
	_w = 0;
	for (i = 0; i < ARRAYSIZE(credits); i++) {
		int tmp = g_gui.getStringWidth(credits[i] + 5);
		if ( _w < tmp && tmp <= maxW) {
			_w = tmp;
		}
	}
	_w += 2*_xOff;

	for (i = 0; i < 1; i++)
		_lines.push_back("");

	Common::String version("\\C\\c0""ScummVM ");
	version += gScummVMVersion;
	_lines.push_back(version);

	Common::String date("\\C\\c2""(built on ");
	date += gScummVMBuildDate;
	date += ')';
	_lines.push_back(date);

	for (i = 0; i < ARRAYSIZE(copyright_text); i++)
		addLine(copyright_text[i]);

	addLine("\\C\\c1""Features compiled in:");
	Common::String features("\\C");
	features += gScummVMFeatures;
	addLine(features.c_str());

	_lines.push_back("");

	addLine("\\C\\c1""Available engines:");
	const PluginList &plugins = PluginManager::instance().getPlugins();
	PluginList::const_iterator iter = plugins.begin();
	for (; iter != plugins.end(); ++iter) {
	  Common::String str;
	  str = "\\C";
	  str += (**iter).getName();
	  addLine(str.c_str());

	  str = "\\C\\c2";
	  str += (**iter).getCopyright();
	  addLine(str.c_str());

	  //addLine("");
	}

	for (i = 0; i < ARRAYSIZE(gpl_text); i++)
		addLine(gpl_text[i]);

	_lines.push_back("");

	for (i = 0; i < ARRAYSIZE(credits); i++)
		addLine(credits[i]);

	// Center the dialog
	_x = (screenW - _w) / 2;
	_y = (screenH - _h) / 2;
}

void AboutDialog::addLine(const char *str) {
	// Extract formatting instructions
	Common::String format;
	while (*str == '\\') {
		format += *str++;
		switch (*str) {
		case 'C':
		case 'L':
		case 'R':
			format += *str++;
			break;
		case 'c':
			format += *str++;
			format += *str++;
			break;
		default:
			error("Unknown scroller opcode '%c'\n", *str);
			break;
		}
	}

	if (*str == 0) {
		_lines.push_back(format);
	} else {
		Common::StringList wrappedLines;
		g_gui.getFont().wordWrapText(str, _w - 2 * _xOff, wrappedLines);

		for (Common::StringList::const_iterator i = wrappedLines.begin(); i != wrappedLines.end(); ++i) {
			_lines.push_back(format + *i);
		}
	}
}


void AboutDialog::open() {
	_scrollTime = getMillis() + kScrollStartDelay;
	_scrollPos = 0;
	_willClose = false;

	Dialog::open();
}

void AboutDialog::close() {
	Dialog::close();
}

void AboutDialog::drawDialog() {
	g_gui.theme()->setDrawArea(Common::Rect(_x, _y, _x+_w, _y+_h));
	Dialog::drawDialog();

	// Draw text
	// TODO: Add a "fade" effect for the top/bottom text lines
	// TODO: Maybe prerender all of the text into another surface,
	//       and then simply compose that over the screen surface
	//       in the right way. Should be even faster...
	const int firstLine = _scrollPos / _lineHeight;
	const int lastLine = MIN((_scrollPos + _h) / _lineHeight + 1, (uint32)_lines.size());
	int y = _y + _yOff - (_scrollPos % _lineHeight);

	for (int line = firstLine; line < lastLine; line++) {
		const char *str = _lines[line].c_str();
		Theme::TextAlign align = Theme::kTextAlignCenter;
		Theme::State state = Theme::kStateEnabled;
		while (str[0] == '\\') {
			switch (str[1]) {
			case 'C':
				align = Theme::kTextAlignCenter;
				break;
			case 'L':
				align = Theme::kTextAlignLeft;
				break;
			case 'R':
				align = Theme::kTextAlignRight;
				break;
			case 'c':
				switch (str[2]) {
				case '0':
					state = Theme::kStateEnabled;
					break;
				case '1':
					state = Theme::kStateHighlight;
					break;
				case '2':
					state = Theme::kStateDisabled;
					break;
				case '3':
					warning("Need state for color 3");
					// color = g_gui._shadowcolor;
					break;
				case '4':
					warning("Need state for color 4");
					// color = g_gui._bgcolor;
					break;
				default:
					error("Unknown color type '%c'", str[2]);
				}
				str++;
				break;
			default:
				error("Unknown scroller opcode '%c'\n", str[1]);
				break;
			}
			str += 2;
		}
		// Trim leading whitespaces if center mode is on
		if (align == Theme::kTextAlignCenter)
			while (*str && *str == ' ')
				str++;

		g_gui.theme()->drawText(Common::Rect(_x + _xOff, y, _x + _w - _xOff, y + g_gui.theme()->getFontHeight()), str, state, align, false, 0, false);
		y += _lineHeight;
	}
	g_gui.theme()->resetDrawArea();
}

void AboutDialog::handleTickle() {
	const uint32 t = getMillis();
	int scrollOffset = ((int)t - (int)_scrollTime) / kScrollMillisPerPixel;
	if (scrollOffset > 0) {
		int modifiers = g_system->getEventManager()->getModifierState();

		// Scroll faster when shift is pressed
		if (modifiers & Common::KBD_SHIFT)
			scrollOffset *= 4;
		// Reverse scrolling when alt is pressed
		if (modifiers & Common::KBD_ALT)
			scrollOffset *= -1;
		_scrollPos += scrollOffset;
		_scrollTime = t;

		if (_scrollPos < 0) {
			_scrollPos = 0;
		} else if ((uint32)_scrollPos > _lines.size() * _lineHeight) {
			_scrollPos = 0;
			_scrollTime += kScrollStartDelay;
		}
		drawDialog();
	}
}

void AboutDialog::handleMouseUp(int x, int y, int button, int clickCount) {
	// Close upon any mouse click
	close();
}

void AboutDialog::handleKeyDown(uint16 ascii, int keycode, int modifiers) {
	if (ascii)
		_willClose = true;
}

void AboutDialog::handleKeyUp(uint16 ascii, int keycode, int modifiers) {
	if (ascii && _willClose)
		close();
}

void AboutDialog::reflowLayout() {
	Dialog::reflowLayout();
	const int screenW = g_system->getOverlayWidth();
	const int screenH = g_system->getOverlayHeight();

	_xOff = g_gui.evaluator()->getVar("aboutXOff");
	_yOff = g_gui.evaluator()->getVar("aboutYOff");
	int outerBorder = g_gui.evaluator()->getVar("aboutOuterBorder");

	_w = screenW - 2 * outerBorder;
	_h = screenH - 2 * outerBorder;

	_lineHeight = g_gui.getFontHeight() + 3;

	// Heuristic to compute 'optimal' dialog width
	int maxW = _w - 2*_xOff;
	_w = 0;
	for (int i = 0; i < ARRAYSIZE(credits); i++) {
		int tmp = g_gui.getStringWidth(credits[i] + 5);
		if ( _w < tmp && tmp <= maxW) {
			_w = tmp;
		}
	}
	_w += 2*_xOff;

	_lineHeight = g_gui.getFontHeight() + 3;

	_x = (screenW - _w) / 2;
	_y = (screenH - _h) / 2;
}

} // End of namespace GUI
