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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/gui/PopUpWidget.h $
 * $Id: PopUpWidget.h 27024 2007-05-30 21:56:52Z fingolfin $
 */

#ifndef POPUPWIDGET_H
#define POPUPWIDGET_H

#include "gui/widget.h"
#include "common/str.h"
#include "common/array.h"

namespace GUI {

enum {
	kPopUpItemSelectedCmd	= 'POPs'
};

/**
 * Popup or dropdown widget which, when clicked, "pop up" a list of items and
 * lets the user pick on of them.
 *
 * Implementation wise, when the user selects an item, then a kPopUpItemSelectedCmd
 * is broadcast, with data being equal to the tag value of the selected entry.
 */
class PopUpWidget : public Widget, public CommandSender {
	friend class PopUpDialog;
	typedef Common::String String;

	struct Entry {
		String	name;
		uint32	tag;
	};
	typedef Common::Array<Entry> EntryList;
protected:
	EntryList		_entries;
	int				_selectedItem;

	String			_label;
	uint			_labelWidth;

	int				_leftPadding;
	int				_rightPadding;
	int				_labelSpacing;

public:
	PopUpWidget(GuiObject *boss, const String &name, const String &label, uint labelWidth = 0);

	void changeLabelWidth(uint newWidth) { _labelWidth = newWidth; }

	void handleMouseDown(int x, int y, int button, int clickCount);

	void appendEntry(const String &entry, uint32 tag = (uint32)-1);
	void clearEntries();

	/** Select the entry at the given index. */
	void setSelected(int item);

	/** Select the first entry matching the given tag. */
	void setSelectedTag(uint32 tag);

	int getSelected() const						{ return _selectedItem; }
	uint32 getSelectedTag() const				{ return (_selectedItem >= 0) ? _entries[_selectedItem].tag : (uint32)-1; }
	const String& getSelectedString() const		{ return (_selectedItem >= 0) ? _entries[_selectedItem].name : String::emptyString; }

	void handleMouseEntered(int button)	{ setFlags(WIDGET_HILITED); draw(); }
	void handleMouseLeft(int button)	{ clearFlags(WIDGET_HILITED); draw(); }

	virtual void reflowLayout();
protected:
	void drawWidget(bool hilite);
};

} // End of namespace GUI

#endif
