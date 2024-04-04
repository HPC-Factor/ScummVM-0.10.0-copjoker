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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/base/plugins.cpp $
 * $Id: plugins.cpp 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

#include "base/plugins.h"
#include "common/util.h"


#ifndef DYNAMIC_MODULES
class StaticPlugin : public Plugin {
	PluginRegistrator *_plugin;
public:
	StaticPlugin(PluginRegistrator *plugin)
		: _plugin(plugin) {
		assert(_plugin);
	}
	
	~StaticPlugin() {
		delete _plugin;
	}

	virtual bool loadPlugin()		{ return true; }
	virtual void unloadPlugin()		{}

	const char *getName() const { return _plugin->_name; }
	const char *getCopyright() const { return _plugin->_copyright; }

	PluginError createInstance(OSystem *syst, Engine **engine) const {
		assert(_plugin->_ef);
		return (*_plugin->_ef)(syst, engine);
	}

	GameList getSupportedGames() const { return _plugin->_games; }

	GameDescriptor findGame(const char *gameid) const {
		assert(_plugin->_qf);
		return (*_plugin->_qf)(gameid);
	}

	GameList detectGames(const FSList &fslist) const {
		assert(_plugin->_df);
		return (*_plugin->_df)(fslist);
	}
};

class StaticPluginProvider : public PluginProvider {
public:
	StaticPluginProvider() {
	}
	
	~StaticPluginProvider() {
	}

	virtual PluginList getPlugins() {
		PluginList pl;
	
		#define LINK_PLUGIN(ID) \
			extern PluginRegistrator *g_##ID##_PluginReg; \
			extern void g_##ID##_PluginReg_alloc(); \
			g_##ID##_PluginReg_alloc(); \
			plugin = g_##ID##_PluginReg; \
			pl.push_back(new StaticPlugin(plugin));
	
		// "Loader" for the static plugins.
		// Iterate over all registered (static) plugins and load them.
		PluginRegistrator *plugin;
	
		#ifndef DISABLE_SCUMM
		LINK_PLUGIN(SCUMM)
		#endif
		#ifndef DISABLE_SKY
		LINK_PLUGIN(SKY)
		#endif
		#ifndef DISABLE_SWORD1
		LINK_PLUGIN(SWORD1)
		#endif
		#ifndef DISABLE_SWORD2
		LINK_PLUGIN(SWORD2)
		#endif
		#ifndef DISABLE_AGOS
		LINK_PLUGIN(AGOS)
		#endif
		#ifndef DISABLE_QUEEN
		LINK_PLUGIN(QUEEN)
		#endif
		#ifndef DISABLE_SAGA
		LINK_PLUGIN(SAGA)
		#endif
		#ifndef DISABLE_KYRA
		LINK_PLUGIN(KYRA)
		#endif
		#ifndef DISABLE_GOB
		LINK_PLUGIN(GOB)
		#endif
		#ifndef DISABLE_LURE
		LINK_PLUGIN(LURE)
		#endif
		#ifndef DISABLE_CINE
		LINK_PLUGIN(CINE)
		#endif
		#ifndef DISABLE_AGI
		LINK_PLUGIN(AGI)
		#endif
		#ifndef DISABLE_TOUCHE
		LINK_PLUGIN(TOUCHE)
		#endif
		#ifndef DISABLE_PARALLACTION
		LINK_PLUGIN(PARALLACTION)
		#endif
		#ifndef DISABLE_CRUISE
		LINK_PLUGIN(CRUISE)
		#endif

		return pl;
	}
};


#endif

#pragma mark -

DECLARE_SINGLETON(PluginManager);

PluginManager::PluginManager() {
#ifndef DYNAMIC_MODULES
	// Add the static plugin provider if we do not build with dynamic
	// plugins.
	addPluginProvider(new StaticPluginProvider());
#endif
}

PluginManager::~PluginManager() {
	// Explicitly unload all loaded plugins
	unloadPlugins();
	
	// Delete the plugin providers
	for (ProviderList::iterator pp = _providers.begin();
	                            pp != _providers.end();
	                            ++pp) {
		delete *pp;
	}
}

void PluginManager::addPluginProvider(PluginProvider *pp) {
	_providers.push_back(pp);
}

void PluginManager::loadPlugins() {
	for (ProviderList::iterator pp = _providers.begin();
	                            pp != _providers.end();
	                            ++pp) {
		PluginList pl((**pp).getPlugins());
		for (PluginList::iterator plugin = pl.begin(); plugin != pl.end(); ++plugin) {
			tryLoadPlugin(*plugin);
		}
	}

}

void PluginManager::unloadPlugins() {
	unloadPluginsExcept(NULL);
}

void PluginManager::unloadPluginsExcept(const Plugin *plugin) {
	Plugin *found = NULL;
	for (PluginList::iterator p = _plugins.begin(); p != _plugins.end(); ++p) {
		if (*p == plugin) {
			found = *p;
		} else {
			(**p).unloadPlugin();
			delete *p;
		}
	}
	_plugins.clear();
	if (found != NULL) {
		_plugins.push_back(found);
	}
}

bool PluginManager::tryLoadPlugin(Plugin *plugin) {
	assert(plugin);
	// Try to load the plugin
	if (plugin->loadPlugin()) {
		// If successful, add it to the list of known plugins and return.
		_plugins.push_back(plugin);
		
		// TODO/FIXME: We should perform some additional checks here:
		// * Check for some kind of "API version" (possibly derived from the
		//   SVN tree revision?)
		// * If two plugins provide the same engine, we should only load one.
		//   To detect this situation, we could just compare the plugin name.
		//   To handle it, simply prefer modules loaded earlier to those coming.
		//   Or vice versa... to be determined... :-)
		
		return true;
	} else {
		// Failed to load the plugin
		delete plugin;
		return false;
	}
}

GameList PluginManager::detectGames(const FSList &fslist) const {
	GameList candidates;

	// Iterate over all known games and for each check if it might be
	// the game in the presented directory.
	PluginList::const_iterator iter;
	for (iter = _plugins.begin(); iter != _plugins.end(); ++iter) {
		candidates.push_back((*iter)->detectGames(fslist));
	}

	return candidates;
}
