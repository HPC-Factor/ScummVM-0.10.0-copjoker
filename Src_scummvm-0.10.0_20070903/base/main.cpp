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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/base/main.cpp $
 * $Id: main.cpp 27380 2007-06-12 21:21:21Z wjpalenstijn $
 *
 */

/*! \mainpage %ScummVM Source Reference
 *
 * These pages contains a cross referenced documentation for the %ScummVM source code,
 * generated with Doxygen (http://www.doxygen.org) directly from the source.
 * Currently not much is actually properly documented, but at least you can get an overview
 * of almost all the classes, methods and variables, and how they interact.
 */

#include "common/stdafx.h"
#include "engines/engine.h"
#include "base/commandLine.h"
#include "base/plugins.h"
#include "base/version.h"

#include "common/config-manager.h"
#include "common/file.h"
#include "common/fs.h"
#include "common/system.h"
#include "gui/newgui.h"
#include "gui/message.h"

#if defined(_WIN32_WCE)
#include "backends/platform/wince/CELauncherDialog.h"
#elif defined(__DC__)
#include "backends/platform/dc/DCLauncherDialog.h"
#else
#include "gui/launcher.h"
#endif

#ifdef PALMOS_68K
#include "args.h"
#endif


static bool launcherDialog(OSystem &system) {

	system.beginGFXTransaction();
		// Set the user specified graphics mode (if any).
		system.setGraphicsMode(ConfMan.get("gfx_mode").c_str());

		system.initSize(320, 200);
	system.endGFXTransaction();

	// Set initial window caption
	system.setWindowCaption(gScummVMFullVersion);

	// Clear the main screen
	system.clearScreen();

#if defined(_WIN32_WCE)
	CELauncherDialog dlg;
#elif defined(__DC__)
	DCLauncherDialog dlg;
#else
	GUI::LauncherDialog dlg;
#endif
	return (dlg.runModal() != -1);
}

static const Plugin *detectMain() {
	const Plugin *plugin = 0;

	// Make sure the gameid is set in the config manager, and that it is lowercase.
	Common::String gameid(ConfMan.getActiveDomainName());
	assert(!gameid.empty());
	if (ConfMan.hasKey("gameid"))
		gameid = ConfMan.get("gameid");
	gameid.toLowercase();
	ConfMan.set("gameid", gameid);

	// Query the plugins and find one that will handle the specified gameid
	printf("Looking for %s\n", gameid.c_str());
	GameDescriptor game = Base::findGame(gameid, &plugin);

	if (plugin == 0) {
		printf("Failed game detection\n");
		warning("%s is an invalid gameid. Use the --list-games option to list supported gameid", gameid.c_str());
		return 0;
	}

	// FIXME: Do we really need this one? 
	printf("Trying to start game '%s'\n", game.description().c_str());

	return plugin;
}

static int runGame(const Plugin *plugin, OSystem &system, const Common::String &edebuglevels) {
	Common::String gameDataPath(ConfMan.get("path"));
	if (gameDataPath.empty()) {
	} else if (gameDataPath.lastChar() != '/'
#if defined(__MORPHOS__) || defined(__amigaos4__)
					&& gameDataPath.lastChar() != ':'
#endif
					&& gameDataPath.lastChar() != '\\') {
		gameDataPath += '/';
		ConfMan.set("path", gameDataPath, Common::ConfigManager::kTransientDomain);
	}

	// We add it here, so MD5-based detection will be able to
	// read mixed case files
	Common::String path;
	if (ConfMan.hasKey("path")) {
		path = ConfMan.get("path");
		FilesystemNode dir(path);
		if (!dir.isDirectory()) {
			warning("Game directory does not exist (%s)", path.c_str());
			return 0;
		}
	} else {
		path = ".";
		warning("No path was provided. Assuming the data files are in the current directory");
	}
	Common::File::addDefaultDirectory(path);

	// Create the game engine
	Engine *engine = 0;
	PluginError err = plugin->createInstance(&system, &engine);
	if (!engine || err != kNoError) {
		// TODO: Show an error dialog or so?
		// TODO: Also take 'err' into consideration...
		//GUI::MessageDialog alert("ScummVM could not find any game in the specified directory!");
		//alert.runModal();
		const char *errMsg = 0;
		switch (err) {
		case kInvalidPathError:
			errMsg = "Invalid game path";
			break;
		case kNoGameDataFoundError:
			errMsg = "Unable to locate game data";
			break;
		default:
			errMsg = "Unknown error";
		}

		warning("%s failed to instantiate engine: %s (target '%s', path '%s')",
			plugin->getName(),
			errMsg,
			ConfMan.getActiveDomainName().c_str(),
			path.c_str()
			);
		return 0;
	}

	// Set the window caption to the game name
	Common::String caption(ConfMan.get("description"));

	Common::String desc = Base::findGame(ConfMan.get("gameid")).description();
	if (caption.empty() && !desc.empty())
		caption = desc;
	if (caption.empty())
		caption = ConfMan.getActiveDomainName();	// Use the domain (=target) name
	if (!caption.empty())	{
		system.setWindowCaption(caption.c_str());
	}

	if (ConfMan.hasKey("path"))
		Common::File::addDefaultDirectory(ConfMan.get("path"));
	else
		Common::File::addDefaultDirectory(".");

	// Add extrapath (if any) to the directory search list
	if (ConfMan.hasKey("extrapath"))
		Common::File::addDefaultDirectoryRecursive(ConfMan.get("extrapath"));

	if (ConfMan.hasKey("extrapath", Common::ConfigManager::kApplicationDomain))
		Common::File::addDefaultDirectoryRecursive(ConfMan.get("extrapath", Common::ConfigManager::kApplicationDomain));

#ifdef DATA_PATH
	// Add the global DATA_PATH to the directory search list
	Common::File::addDefaultDirectoryRecursive(DATA_PATH);
#endif

	// On creation the engine should've set up all debug levels so we can use
	// the command line arugments here
	Common::enableSpecialDebugLevelList(edebuglevels);

	int result;

	// Init the engine (this might change the screen parameters
	result = engine->init();

	// Run the game engine if the initialization was successful.
	if (result == 0) {
		result = engine->go();
	}

	// We clear all debug levels again even though the engine should do it
	Common::clearAllSpecialDebugLevels();

	// Free up memory
	delete engine;

	// Reset the file/directory mappings
	Common::File::resetDefaultDirectories();

	return result;
}


extern "C" int scummvm_main(int argc, char *argv[]) {
	Common::String specialDebug;
	Common::String command;

	// Verify that the backend has been initialized (i.e. g_system has been set).
	assert(g_system);
	OSystem &system = *g_system;
	
	// Register config manager defaults
	Base::registerDefaults();

	// Parse the command line
	Common::StringMap settings;
	command = Base::parseCommandLine(settings, argc, argv);
#ifdef PALMOS_68K
	ArgsFree(argv);
#endif

	// Load the config file (possibly overriden via command line):
	if (settings.contains("config")) {
		ConfMan.loadConfigFile(settings["config"]);
		settings.erase("config");
	} else {
		ConfMan.loadDefaultConfigFile();
	}

	// Update the config file
	ConfMan.set("versioninfo", gScummVMVersion, Common::ConfigManager::kApplicationDomain);


	// Load and setup the debuglevel and the debug flags. We do this at the
	// soonest possible moment to ensure debug output starts early on, if 
	// requested.
	if (settings.contains("debuglevel")) {
		gDebugLevel = (int)strtol(settings["debuglevel"].c_str(), 0, 10);
		printf("Debuglevel (from command line): %d\n", gDebugLevel);
		settings.erase("debuglevel");	// This option should not be passed to ConfMan.
	} else if (ConfMan.hasKey("debuglevel"))
		gDebugLevel = ConfMan.getInt("debuglevel");

	if (settings.contains("debugflags")) {
		specialDebug = settings["debugflags"];
		settings.erase("debugflags");
	}

	// Load the plugins.
	PluginManager::instance().loadPlugins();
	
	// Process the remaining command line settings. Must be done after the
	// config file and the plugins have been loaded.
	if (!Base::processSettings(command, settings))
		return 0;

	// Init the backend. Must take place after all config data (including
	// the command line params) was read.
	system.initBackend();

	// Unless a game was specified, show the launcher dialog
	if (0 == ConfMan.getActiveDomain()) {
		launcherDialog(system);

		// Discard any command line options. Those that affect the graphics
		// mode etc. already have should have been handled by the backend at
		// this point. And the others (like bootparam etc.) should not
		// blindly be passed to the first game launched from the launcher.
		ConfMan.getDomain(Common::ConfigManager::kTransientDomain)->clear();
	}

	// FIXME: We're now looping the launcher. This, of course, doesn't
	// work as well as it should. In theory everything should be destroyed
	// cleanly, so this is now enabled to encourage people to fix bits :)
	while (0 != ConfMan.getActiveDomain()) {
		// Verify the given game name is a valid supported game
		const Plugin *plugin = detectMain();
		if (plugin) {
			// Unload all plugins not needed for this game,
			// to save memory
			PluginManager::instance().unloadPluginsExcept(plugin);

			int result = runGame(plugin, system, specialDebug);
			if (result == 0)
				break;

			// Discard any command line options. It's unlikely that the user
			// wanted to apply them to *all* games ever launched.
			ConfMan.getDomain(Common::ConfigManager::kTransientDomain)->clear();
			
			// Clear the active config domain
			ConfMan.setActiveDomain("");

			// PluginManager::instance().unloadPlugins();
			PluginManager::instance().loadPlugins();
		}

		launcherDialog(system);
	}
	return 0;
}
