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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/gui/browser.h $
 * $Id: browser.h 27024 2007-05-30 21:56:52Z fingolfin $
 */

#ifndef BROWSER_DIALOG_H
#define BROWSER_DIALOG_H

#include "gui/dialog.h"
#include "common/str.h"
#include "common/fs.h"

#ifdef MACOSX
#include <Carbon/Carbon.h>
#endif

namespace GUI {

class ListWidget;
class StaticTextWidget;

class BrowserDialog : public Dialog {
	typedef Common::String String;
	typedef Common::StringList StringList;
public:
	BrowserDialog(const char *title, bool dirBrowser);

#ifdef MACOSX
	~BrowserDialog();
	virtual int runModal();
#else
	virtual void open();
	virtual void handleCommand(CommandSender *sender, uint32 cmd, uint32 data);
#endif

	const FilesystemNode	&getResult() { return _choice; }

protected:
#ifdef MACOSX
	CFStringRef		_titleRef;
#else
	ListWidget		*_fileList;
	StaticTextWidget	*_currentPath;
	FilesystemNode	_node;
	FSList			_nodeContent;
#endif
	FilesystemNode	_choice;
	bool			_isDirBrowser;

#ifndef MACOSX
	void updateListing();
#endif
};

} // End of namespace GUI

#endif
