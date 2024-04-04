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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/gui/launcher.h $
 * $Id: launcher.h 27024 2007-05-30 21:56:52Z fingolfin $
 */

#ifndef LAUNCHER_DIALOG_H
#define LAUNCHER_DIALOG_H

#include "gui/dialog.h"
#include "common/str.h"

namespace GUI {

class BrowserDialog;
class ListWidget;
class GraphicsWidget;


Common::String addGameToConf(const GameDescriptor &result);


class LauncherDialog : public Dialog {
	typedef Common::String String;
	typedef Common::StringList StringList;
public:
	LauncherDialog();
	~LauncherDialog();

	virtual void handleCommand(CommandSender *sender, uint32 cmd, uint32 data);

	virtual void handleKeyDown(uint16 ascii, int keycode, int modifiers);
	virtual void handleKeyUp(uint16 ascii, int keycode, int modifiers);

protected:
	ListWidget		*_list;
	ButtonWidget	*_addButton;
	Widget			*_startButton;
	Widget			*_editButton;
	Widget			*_removeButton;
#ifndef DISABLE_FANCY_THEMES
	GraphicsWidget		*_logo;
#endif
	StringList		_domains;
	BrowserDialog	*_browser;

	virtual void reflowLayout();

	void updateListing();
	void updateButtons();

	void open();
	void close();
	virtual void addGame();
	void removeGame(int item);
	void editGame(int item);

	void selectGame(const String &name);
};

} // End of namespace GUI

#endif
