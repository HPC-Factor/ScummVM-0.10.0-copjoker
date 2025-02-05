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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/common/config-manager.h $
 * $Id: config-manager.h 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

#ifndef COMMON_CONFIG_MANAGER_H
#define COMMON_CONFIG_MANAGER_H

#include "common/array.h"
//#include "common/config-file.h"
#include "common/hashmap.h"
#include "common/singleton.h"
#include "common/str.h"
#include "common/hash-str.h"

namespace Common {

class WriteStream;


/**
 * The (singleton) configuration manager, used to query & set configuration
 * values using string keys.
 *
 * @todo Implement the callback based notification system (outlined below)
 *       which sends out notifications to interested parties whenever the value
 *       of some specific (or any) configuration key changes.
 */
class ConfigManager : public Singleton<ConfigManager> {

public:

	class Domain : public StringMap {
	private:
		StringMap _keyValueComments;
		String _domainComment;

	public:
		const String &get(const String &key) const;

		void setDomainComment(const String &comment);
		const String &getDomainComment() const;

		void setKVComment(const String &key, const String &comment);
		const String &getKVComment(const String &key) const;
		bool hasKVComment(const String &key) const;
	};

	typedef HashMap<String, Domain, IgnoreCase_Hash, IgnoreCase_EqualTo> DomainMap;

#if !(defined(PALMOS_ARM) || defined(PALMOS_DEBUG) || defined(__GP32__))
	/** The name of the application domain (normally 'scummvm'). */
	static const String kApplicationDomain;

	/** The transient (pseudo) domain. */
	static const String kTransientDomain;
#else
	static const char *kApplicationDomain;
	static const char *kTransientDomain;
	
	const String _emptyString;
#endif

	void				loadDefaultConfigFile();
	void				loadConfigFile(const String &filename);

	/**
	 * Retrieve the config domain with the given name.
	 * @param domName	the name of the domain to retrieve
	 * @return pointer to the domain, or 0 if the domain doesn't exist.
	 */
	Domain *			getDomain(const String &domName);
	const Domain *		getDomain(const String &domName) const;


	//
	// Generic access methods: No domain specified, use the values from the
	// various domains in the order of their priority.
	//
	
	bool				hasKey(const String &key) const;
	const String &		get(const String &key) const;
	void				set(const String &key, const String &value);

#if 1
	//
	// Domain specific access methods: Acces *one specific* domain and modify it.
	// TODO: I'd like to get rid of most of those if possible, or at least reduce
	// their usage, by using getDomain as often as possible. For example in the
	// options dialog code...
	//

	bool				hasKey(const String &key, const String &domName) const;
	const String &		get(const String &key, const String &domName) const;
	void				set(const String &key, const String &value, const String &domName);

	void				removeKey(const String &key, const String &domName);
#endif

	//
	// Some additional convenience accessors.
	//
	int					getInt(const String &key, const String &domName = String::emptyString) const;
	bool				getBool(const String &key, const String &domName = String::emptyString) const;
	void				setInt(const String &key, int value, const String &domName = String::emptyString);
	void				setBool(const String &key, bool value, const String &domName = String::emptyString);


	void				registerDefault(const String &key, const String &value);
	void				registerDefault(const String &key, const char *value);
	void				registerDefault(const String &key, int value);
	void				registerDefault(const String &key, bool value);

	void				flushToDisk();

	void				setActiveDomain(const String &domName);
	Domain *			getActiveDomain() { return _activeDomain; }
	const Domain *		getActiveDomain() const { return _activeDomain; }
	const String &		getActiveDomainName() const { return _activeDomainName; }

	void				addGameDomain(const String &domName);
	void				removeGameDomain(const String &domName);
	void				renameGameDomain(const String &oldName, const String &newName);
	bool				hasGameDomain(const String &domName) const;
	const DomainMap &	getGameDomains() const { return _gameDomains; }
	
/*
	TODO: Callback/change notification system
	typedef void (*ConfigCallback)(const ConstString &key, void *refCon);

	void   registerCallback(ConfigCallback cfgc, void *refCon, const ConstString &key = String::emptyString)
	void unregisterCallback(ConfigCallback cfgc, const ConstString &key = String::emptyString)
*/

private:
	friend class Singleton<SingletonBaseType>;
	ConfigManager();

	void			loadFile(const String &filename);
	void			writeDomain(WriteStream &stream, const String &name, const Domain &domain);

	Domain			_transientDomain;
	DomainMap		_gameDomains;
	Domain			_appDomain;
	Domain			_defaultsDomain;

	StringList		_domainSaveOrder;

	String			_activeDomainName;
	Domain *		_activeDomain;

	String			_filename;
};

}	// End of namespace Common

/** Shortcut for accessing the configuration manager. */
#define ConfMan		Common::ConfigManager::instance()

#endif
