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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/base/game.cpp $
 * $Id: game.cpp 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

#include "base/game.h"
#include "base/plugins.h"

void GameDescriptor::updateDesc(const char *extra) {
	// TODO: The format used here (LANG/PLATFORM/EXTRA) is not set in stone.
	// We may want to change the order (PLATFORM/EXTRA/LANG, anybody?), or
	// the seperator (instead of '/' use ', ' or ' ').
	const bool hasCustomLanguage = (language() != Common::UNK_LANG);
	const bool hasCustomPlatform = (platform() != Common::kPlatformUnknown);
	const bool hasExtraDesc = (extra && extra[0]);

	// Adapt the description string if custom platform/language is set.
	if (hasCustomLanguage || hasCustomPlatform || hasExtraDesc) {
		Common::String descr = description();

		descr += " (";
		if (hasExtraDesc)
			descr += extra;
		if (hasCustomPlatform) {
			if (hasExtraDesc)
				descr += "/";
			descr += Common::getPlatformDescription(platform());
		}
		if (hasCustomLanguage) {
			if (hasExtraDesc || hasCustomPlatform)
				descr += "/";
			descr += Common::getLanguageDescription(language());
		}
		descr += ")";
		setVal("description", descr);
	}
}

namespace Base {

// TODO: Find a better place for this function.
GameDescriptor findGame(const Common::String &gameName, const Plugin **plugin) {
	// Find the GameDescriptor for this target
	const PluginList &plugins = PluginManager::instance().getPlugins();
	GameDescriptor result;

	if (plugin)
		*plugin = 0;

	PluginList::const_iterator iter = plugins.begin();
	for (iter = plugins.begin(); iter != plugins.end(); ++iter) {
		result = (*iter)->findGame(gameName.c_str());
		if (!result.gameid().empty()) {
			if (plugin)
				*plugin = *iter;
			break;
		}
	}
	return result;
}

} // End of namespace Base
