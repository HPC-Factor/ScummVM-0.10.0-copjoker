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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/gui/KeysDialog.h $
 * $Id: KeysDialog.h 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

#ifndef KEYSDIALOG_H
#define KEYSDIALOG_H

#include "gui/newgui.h"
#include "gui/dialog.h"
#include "gui/ListWidget.h"
#include "common/str.h"

namespace GUI {

class KeysDialog : public GUI::Dialog {
public:
	KeysDialog(const Common::String &title = "Choose an action to map");

	virtual void handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data);
	virtual void handleKeyUp(uint16 ascii, int keycode, int modifiers);
	virtual void handleKeyDown(uint16 ascii, int keycode, int modifiers);

protected:

	GUI::ListWidget		 *_actionsList;
	GUI::StaticTextWidget *_actionTitle;
	GUI::StaticTextWidget *_keyMapping;
	int				 _actionSelected;
};

} // namespace GUI

#endif
