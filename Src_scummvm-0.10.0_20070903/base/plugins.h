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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/base/plugins.h $
 * $Id: plugins.h 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

#ifndef BASE_PLUGINS_H
#define BASE_PLUGINS_H

#include "common/stdafx.h"
#include "common/array.h"
#include "common/list.h"
#include "common/singleton.h"
#include "common/util.h"
#include "base/game.h"

class Engine;
class FSList;
class OSystem;

/**
 * Error codes which mayb be reported by plugins under various circumstances.
 * @todo Turn this into a global 'ErrorCode' enum used by all of ScummVM ?
 */
enum PluginError {
	kNoError = 0,	// No error occured
	kInvalidPathError,
	kNoGameDataFoundError,
	kUnsupportedGameidError,
	
	kUnknownError		// Catch-all error, used if no other error code matches
};


/**
 * Abstract base class for the plugin system.
 * Subclasses for this can be used to wrap both static and dynamic
 * plugins.
 */
class Plugin {
public:
	virtual ~Plugin() {}

	virtual bool loadPlugin() = 0;
	virtual void unloadPlugin() = 0;

	virtual const char *getName() const = 0;
	virtual const char *getCopyright() const = 0;
//	virtual int getVersion() const	{ return 0; }	// TODO!

	virtual GameList getSupportedGames() const = 0;
	virtual GameDescriptor findGame(const char *gameid) const = 0;
	virtual GameList detectGames(const FSList &fslist) const = 0;

	virtual PluginError createInstance(OSystem *syst, Engine **engine) const = 0;
};


/**
 * REGISTER_PLUGIN is a convenience macro meant to ease writing
 * the plugin interface for our modules. In particular, using it
 * makes it possible to compile the very same code in a module
 * both as a static and a dynamic plugin.
 *
 * Each plugin has to define the following functions:
 * - GameList Engine_##ID##_gameIDList()
 *   -> returns a list of gameid/desc pairs. Only used to implement '--list-games'.
 * - GameDescriptor Engine_##ID##_findGameID(const char *gameid)
 *   -> asks the Engine for a GameDescriptor matching the gameid. If that is not
 *      possible, the engine MUST set the gameid of the returned value to 0.
 *      Note: This MUST succeed for every gameID on the list returned by
 *      gameIDList(), but MAY also work for additional gameids (e.g. to support
 *      obsolete targets).
 * - DetectedGameList Engine_##ID##_detectGames(const FSList &fslist)
 *   -> scans through the given file list (usually the contents of a directory),
 *      and attempts to detects games present in that location.
 * - PluginError Engine_##ID##_create(OSystem *syst, Engine **engine)
 *   -> factory function, create an instance of the Engine class.
 *
 * @todo	add some means to query the plugin API version etc.
 */

#ifndef DYNAMIC_MODULES
#define REGISTER_PLUGIN(ID,name,copyright) \
	PluginRegistrator *g_##ID##_PluginReg; \
	void g_##ID##_PluginReg_alloc() { \
		g_##ID##_PluginReg = new PluginRegistrator(name, copyright, \
			Engine_##ID##_gameIDList(), \
			Engine_##ID##_findGameID, \
			Engine_##ID##_create, \
			Engine_##ID##_detectGames \
			);\
	} \
	void dummyFuncToAllowTrailingSemicolon()
#else
#define REGISTER_PLUGIN(ID,name,copyright) \
	extern "C" { \
		PLUGIN_EXPORT const char *PLUGIN_name() { return name; } \
		PLUGIN_EXPORT const char *PLUGIN_copyright() { return copyright; } \
		PLUGIN_EXPORT GameList PLUGIN_gameIDList() { return Engine_##ID##_gameIDList(); } \
		PLUGIN_EXPORT GameDescriptor PLUGIN_findGameID(const char *gameid) { return Engine_##ID##_findGameID(gameid); } \
		PLUGIN_EXPORT PluginError PLUGIN_createEngine(OSystem *syst, Engine **engine) { return Engine_##ID##_create(syst, engine); } \
		PLUGIN_EXPORT GameList PLUGIN_detectGames(const FSList &fslist) { return Engine_##ID##_detectGames(fslist); } \
	} \
	void dummyFuncToAllowTrailingSemicolon()
#endif

#ifndef DYNAMIC_MODULES
/**
 * The PluginRegistrator class is used by the static version of REGISTER_PLUGIN
 * to allow static 'plugins' to register with the PluginManager.
 */
class PluginRegistrator {
	friend class StaticPlugin;
public:
	typedef GameDescriptor (*GameIDQueryFunc)(const char *gameid);
	typedef PluginError (*EngineFactory)(OSystem *syst, Engine **engine);
	typedef GameList (*DetectFunc)(const FSList &fslist);

protected:
	const char *_name;
	const char *_copyright;
	GameIDQueryFunc _qf;
	EngineFactory _ef;
	DetectFunc _df;
	GameList _games;

public:
	PluginRegistrator(const char *name, const char *copyright, GameList games, GameIDQueryFunc qf, EngineFactory ef, DetectFunc df)
		: _name(name), _copyright(copyright), _qf(qf), _ef(ef), _df(df), _games(games) {}
};
#endif


/** List of plugins. */
typedef Common::Array<Plugin *> PluginList;


class PluginProvider {
public:
	virtual ~PluginProvider() {}
	
	/**
	 * Return a list of Plugin objects. The caller is responsible for actually
	 * loading/unloading them (by invoking the appropriate methods).
	 * Furthermore, the caller is responsible for deleting these objects
	 * eventually.
	 */
	virtual PluginList getPlugins() = 0;
};

//class PluginManager;

/**
 * Instances of this class manage all plugins, including loading them,
 * making wrapper objects of class Plugin available, and unloading them.
 *
 * @todo Add support for dynamic plugins (this may need additional API, e.g. for a plugin path)
 */
class PluginManager : public Common::Singleton<PluginManager> {
	typedef Common::List<PluginProvider *> ProviderList;
private:
	PluginList _plugins;
	ProviderList _providers;

	bool tryLoadPlugin(Plugin *plugin);

	friend class Common::Singleton<SingletonBaseType>;
	PluginManager();

public:
	~PluginManager();
	
	void addPluginProvider(PluginProvider *pp);

	void loadPlugins();
	void unloadPlugins();
	void unloadPluginsExcept(const Plugin *plugin);

	const PluginList &getPlugins()	{ return _plugins; }

	GameList detectGames(const FSList &fslist) const;
};

#endif
