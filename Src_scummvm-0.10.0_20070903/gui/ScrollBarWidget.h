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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/gui/ScrollBarWidget.h $
 * $Id: ScrollBarWidget.h 27024 2007-05-30 21:56:52Z fingolfin $
 */

#ifndef SCROLLBARWIDGET_H
#define SCROLLBARWIDGET_H

#include "gui/widget.h"

namespace GUI {

enum {
	kNormalScrollBarWidth	= 9,
	kBigScrollBarWidth		= 15
};


enum {
	kSetPositionCmd		= 'SETP'
};


class ScrollBarWidget : public Widget, public CommandSender {
protected:
	typedef enum {
		kNoPart,
		kUpArrowPart,
		kDownArrowPart,
		kSliderPart,
		kPageUpPart,
		kPageDownPart
	} Part;

	Part	_part;
	int		_sliderHeight;
	int		_sliderPos;

	Part	_draggingPart;
	int		_sliderDeltaMouseDownPos;

public:
	int		_numEntries;
	int		_entriesPerPage;
	int		_currentPos;

public:
	ScrollBarWidget(GuiObject *boss, int x, int y, int w, int h);

	void handleMouseDown(int x, int y, int button, int clickCount);
	void handleMouseUp(int x, int y, int button, int clickCount);
	void handleMouseWheel(int x, int y, int direction);
	void handleMouseMoved(int x, int y, int button);
	void handleMouseEntered(int button)	{ setFlags(WIDGET_HILITED); }
	void handleMouseLeft(int button)	{ clearFlags(WIDGET_HILITED); _part = kNoPart; draw(); }
	void handleTickle();

	// FIXME - this should be private, but then we also have to add accessors
	// for _numEntries, _entriesPerPage and _currentPos. This again leads to the question:
	// should these accessors force a redraw?
	void recalc();

protected:
	void drawWidget(bool hilite);
	void checkBounds(int old_pos);
};

} // End of namespace GUI

#endif
