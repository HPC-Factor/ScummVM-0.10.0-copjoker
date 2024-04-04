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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/backends/plugins/dc/dc-provider.cpp $
 * $Id: dc-provider.cpp 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

#if defined(DYNAMIC_MODULES) && defined(__DC__)

#include "backends/plugins/dc/dc-provider.h"
#include "backends/plugins/dynamic-plugin.h"
#include "common/fs.h"

#include "dcloader.h"
#define PLUGIN_DIRECTORY	"/"
#define PLUGIN_PREFIX		""
#define PLUGIN_SUFFIX		".PLG"


class DCPlugin : public DynamicPlugin {
protected:
	void *_dlHandle;
	Common::String _filename;

	virtual VoidFunc findSymbol(const char *symbol) {
		void *func = dlsym(_dlHandle, symbol);
		if (!func)
			warning("Failed loading symbol '%s' from plugin '%s' (%s)", symbol, _filename.c_str(), dlerror());
	
		// FIXME HACK: This is a HACK to circumvent a clash between the ISO C++
		// standard and POSIX: ISO C++ disallows casting between function pointers
		// and data pointers, but dlsym always returns a void pointer. For details,
		// see e.g. <http://www.trilithium.com/johan/2004/12/problem-with-dlsym/>.
		assert(sizeof(VoidFunc) == sizeof(func));
		VoidFunc tmp;
		memcpy(&tmp, &func, sizeof(VoidFunc));
		return tmp;
	}

public:
	DCPlugin(const Common::String &filename)
		: _dlHandle(0), _filename(filename) {}

	bool loadPlugin() {
		assert(!_dlHandle);
		_dlHandle = dlopen(_filename.c_str(), RTLD_LAZY);
	
		if (!_dlHandle) {
			warning("Failed loading plugin '%s' (%s)", _filename.c_str(), dlerror());
			return false;
		}
	
		bool ret = DynamicPlugin::loadPlugin();
		
		if (ret)
			dlforgetsyms(_dlHandle);

		return ret;
	}
	void unloadPlugin() {
		if (_dlHandle) {
			if (dlclose(_dlHandle) != 0)
				warning("Failed unloading plugin '%s' (%s)", _filename.c_str(), dlerror());
			_dlHandle = 0;
		}
	}
};


DCPluginProvider::DCPluginProvider() {
}

DCPluginProvider::~DCPluginProvider() {
}

PluginList DCPluginProvider::getPlugins() {
	PluginList pl;
	
	
	// Load dynamic plugins
	// TODO... this is right now just a nasty hack.
	// This should search one or multiple directories for all plugins it can
	// find (to this end, we maybe should use a special prefix/suffix; e.g.
	// instead of libscumm.so, use scumm.engine or scumm.plugin etc.).
	//
	// The list of directories to search could be e.g.:
	// User specified (via config file), ".", "./plugins", "$(prefix)/lib".
	//
	// We also need to add code which ensures what we are looking at is
	// a) a ScummVM engine and b) matches the version of the executable.
	// Hence one more symbol should be exported by plugins which returns
	// the "ABI" version the plugin was built for, and we can compare that
	// to the ABI version of the executable.

	// Load all plugins.
	// Scan for all plugins in this directory
	FilesystemNode dir(PLUGIN_DIRECTORY);
	FSList files;
	if (!dir.listDir(files, FilesystemNode::kListFilesOnly)) {
		error("Couldn't open plugin directory '%s'", PLUGIN_DIRECTORY);
	}

	for (FSList::const_iterator i = files.begin(); i != files.end(); ++i) {
		Common::String name(i->name());
		if (name.hasPrefix(PLUGIN_PREFIX) && name.hasSuffix(PLUGIN_SUFFIX)) {
			pl.push_back(new DCPlugin(i->path()));
		}
	}
	
	
	return pl;
}


#endif // defined(DYNAMIC_MODULES) && defined(__DC__)
