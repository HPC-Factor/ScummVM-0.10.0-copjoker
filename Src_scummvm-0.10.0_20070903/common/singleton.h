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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/common/singleton.h $
 * $Id: singleton.h 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

#ifndef COMMON_SINGLETON_H
#define COMMON_SINGLETON_H

#include "common/noncopyable.h"

namespace Common {

/**
 * Generic template base class for implementing the singleton design pattern.
 */
template <class T>
class Singleton : NonCopyable {
private:
	Singleton<T>(const Singleton<T>&);
	Singleton<T>& operator= (const Singleton<T>&);

	static T* _singleton;

	/**
	 * The default object factory used by the template class Singleton.
	 * By specialising this template function, one can make a singleton use a
	 * custom object factory. For example, to support encapsulation, your
	 * singleton class might be pure virtual (or "abstract" in Java terminology),
	 * and you specialise makeInstance to return an instance of a subclass.
	 */
	//template <class T>
#if defined (_WIN32_WCE) || defined (_MSC_VER) || defined (__WINS__)
//FIXME evc4 and msvc7 doesn't like it as private member
public:
#endif
	static T* makeInstance() {
		return new T();
	}


public:
	static T& instance() {
		// TODO: We aren't thread safe. For now we ignore it since the
		// only thing using this singleton template is the config manager,
		// and that is first instantiated long before any threads.
		// TODO: We don't leak, but the destruction order is nevertheless
		// semi-random. If we use multiple singletons, the destruction
		// order might become an issue. There are various approaches
		// to solve that problem, but for now this is sufficient
		if (!_singleton)
			_singleton = T::makeInstance();
		return *_singleton;
	}
protected:
	Singleton<T>()		{ }
#ifdef __SYMBIAN32__
	virtual ~Singleton()	{ }
#else
	virtual ~Singleton<T>()	{ }
#endif

	typedef T	SingletonBaseType;
};

#define DECLARE_SINGLETON(T) template<> T* Common::Singleton<T>::_singleton=0

}	// End of namespace Common

#endif
