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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/backends/platform/wince/CELauncherDialog.cpp $
 * $Id: CELauncherDialog.cpp 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

#include "common/stdafx.h"

#include "wince-sdl.h"

#include "CELauncherDialog.h"

#include "engines/engine.h"

#include "gui/newgui.h"
#include "gui/widget.h"
#include "gui/browser.h"
#include "gui/message.h"

#include "common/config-manager.h"

using namespace GUI;
using namespace Common;

class CEAboutDialog : public Dialog {
public:
	CEAboutDialog::CEAboutDialog()
	: Dialog(10, 60, 300, 77) {
		char tempo[100];

		addButton(this,(_w - kButtonWidth) / 2, 45, "OK", kCloseCmd, '\r');	// Close dialog - FIXME

		Common::String videoDriver("Using SDL driver ");
		SDL_VideoDriverName(tempo, sizeof(tempo));
		videoDriver += tempo;
		new StaticTextWidget(this, 0, 10, _w, kLineHeight, videoDriver, kTextAlignCenter);
		Common::String displayInfos("Display ");
		sprintf(tempo, "%dx%d (real %dx%d)", GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), OSystem_WINCE3::getScreenWidth(), OSystem_WINCE3::getScreenHeight());
		displayInfos += tempo;
		new StaticTextWidget(this, 0, 20, _w, kLineHeight, displayInfos, kTextAlignCenter);
	}
};

CELauncherDialog::CELauncherDialog() : GUI::LauncherDialog() {
}

void CELauncherDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	LauncherDialog::handleCommand(sender, cmd, data);
	if (cmd == 'ABOU') {
		CEAboutDialog about;
		about.runModal();
	}
}

void CELauncherDialog::automaticScanDirectory(const FilesystemNode &node) {
	// First check if we have a recognized game in the current directory
	FSList files;
	node.listDir(files, FilesystemNode::kListFilesOnly);
	// detect
	GameList candidates(PluginManager::instance().detectGames(files));
	// insert
	if (candidates.size() >= 1) {
		GameDescriptor result = candidates[0];
		result["path"] = node.path();
		addGameToConf(result);
	}
	// Then recurse on the subdirectories
	FSList dirs;
	node.listDir(dirs, FilesystemNode::kListDirectoriesOnly);
	for (FSList::const_iterator currentDir = dirs.begin(); currentDir != dirs.end(); ++currentDir)
		automaticScanDirectory(*currentDir);

}

/* FIXME: We do this here, replicating code of the launcher, because findfirst/next
 * returns some illegal paths atm.
 */
void CELauncherDialog::addGame() {
	MessageDialog alert("Do you want to perform an automatic scan ?", "Yes", "No");
	if (alert.runModal() == kMessageOK && _browser->runModal() > 0) {
		// Clear existing domains
		ConfigManager::DomainMap &domains = (ConfigManager::DomainMap&)ConfMan.getGameDomains();
		domains.clear();
		ConfMan.flushToDisk();
		automaticScanDirectory(_browser->getResult());
		ConfMan.flushToDisk();
		updateListing();
		draw();
	}
	else
		GUILauncherDialog::addGame();
}
