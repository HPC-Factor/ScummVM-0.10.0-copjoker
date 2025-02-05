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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/common/str.h $
 * $Id: str.h 27051 2007-06-02 12:42:40Z fingolfin $
 */

#ifndef COMMON_STRING_H
#define COMMON_STRING_H

#include "common/scummsys.h"
#include "common/array.h"

namespace Common {

/**
 * Simple string class for ScummVM. Provides automatic storage managment,
 * and overloads several operators in a 'natural' fashion, mimicking
 * the std::string class. Even provides simple iterators.
 * 
 * This class tries to avoid allocating lots of small blocks on the heap,
 * since that is inefficient on several platforms supported by ScummVM.
 * Instead, small strings are stored 'inside' the string object (i.e. on
 * the stack, for stack allocated objects), and only for strings exceeding
 * a certain length do we allocate a buffer on the heap.
 */
class String {
protected:
	/**
	 * The size of the internal storage. Increasing this means less heap
	 * allocations are needed, at the cost of more stack memory usage,
	 * and of course lots of wasted memory. Empirically, 90% or more of
	 * all String instances are less than 32 chars long. If a platform
	 * is very short on stack space, it would be possible to lower this.
	 * A value of 24 still seems acceptable, though considerably worse,
	 * while 16 seems to be the lowest you want to go... Anything lower
	 * than 8 makes no sense, since that's the size of member _extern
	 * (on 32 bit machines; 12 bytes on systems with 64bit pointers).
	 */
	static const uint32 _builtinCapacity = 32;

	/**
	 * Length of the string. Stored to avoid having to call strlen
	 * a lot. Yes, we limit ourselves to strings shorter than 4GB --
	 * on purpose :-).
	 */
	uint32 		_len;
	
	/**
	 * Pointer to the actual string storage. Either points to _storage,
	 * or to a block allocated on the heap via malloc.
	 */
	char		*_str;
	
	
	union {
		/**
		 * Internal string storage.
		 */
		char _storage[_builtinCapacity];
		/**
		 * External string storage data -- the refcounter, and the
		 * capacity of the string _str points to.
		 */
		struct {
			mutable int *_refCount;
			uint32 		_capacity;
		} _extern;
	};
	
	inline bool isStorageIntern() const {
		return _str == _storage;
	}
	
public:
#if !(defined(PALMOS_ARM) || defined(PALMOS_DEBUG) || defined(__GP32__))
	static const String emptyString;
#else
	static const char *emptyString;
#endif

	String() : _len(0), _str(_storage) { _storage[0] = 0; }
	String(const char *str, uint32 len = 0);
	String(const String &str);
	String(char c);
	virtual ~String();

	String &operator  =(const char *str);
	String &operator  =(const String &str);
	String &operator  =(char c);
	String &operator +=(const char *str);
	String &operator +=(const String &str);
	String &operator +=(char c);

	bool operator ==(const String &x) const;
	bool operator ==(const char *x) const;
	bool operator !=(const String &x) const;
	bool operator !=(const char *x) const;

	bool operator <(const String &x) const;
	bool operator <=(const String &x) const;
	bool operator >(const String &x) const;
	bool operator >=(const String &x) const;

	bool equals(const String &x) const;
	bool equalsIgnoreCase(const String &x) const;
	int compareTo(const String &x) const;	// strcmp clone
	int compareToIgnoreCase(const String &x) const;	// stricmp clone

	bool equals(const char *x) const;
	bool equalsIgnoreCase(const char *x) const;
	int compareTo(const char *x) const;	// strcmp clone
	int compareToIgnoreCase(const char *x) const;	// stricmp clone

	bool hasSuffix(const char *x) const;
	bool hasPrefix(const char *x) const;
	
	bool contains(const char *x) const;

	inline const char *c_str() const		{ return _str; }
	inline uint size() const				{ return _len; }

	inline bool empty() const	{ return (_len == 0); }
	char lastChar() const	{ return (_len > 0) ? _str[_len-1] : 0; }

	char operator [](int idx) const {
		assert(_str && idx >= 0 && idx < (int)_len);
		return _str[idx];
	}

	char &operator [](int idx) {
		assert(_str && idx >= 0 && idx < (int)_len);
		return _str[idx];
	}

	void deleteLastChar();
	void deleteChar(uint32 p);
	void clear();
	void insertChar(char c, uint32 p);

	void toLowercase();
	void toUppercase();

	uint hash() const;

public:
	typedef char *        iterator;
	typedef const char *  const_iterator;

	iterator		begin() {
		return _str;
	}

	iterator		end() {
		return begin() + size();
	}

	const_iterator	begin() const {
		return _str;
	}

	const_iterator	end() const {
		return begin() + size();
	}

protected:
	void ensureCapacity(uint32 new_len, bool keep_old);
	void incRefCount() const;
	void decRefCount(int *oldRefCount);
};

// Append two strings to form a new (temp) string
String operator +(const String &x, const String &y);

String operator +(const char *x, const String &y);
String operator +(const String &x, const char *y);

String operator +(const String &x, char y);
String operator +(char x, const String &y);

// Some useful additional comparision operators for Strings
bool operator == (const char *x, const String &y);
bool operator != (const char *x, const String &y);

// Utility functions to remove leading and trailing whitespaces
extern char *ltrim(char *t);
extern char *rtrim(char *t);
extern char *trim(char *t);

class StringList : public Array<String> {
public:
	void push_back(const char *str) {
		ensureCapacity(_size + 1);
		_data[_size++] = str;
	}

	void push_back(const String &str) {
		ensureCapacity(_size + 1);
		_data[_size++] = str;
	}
};

}	// End of namespace Common

#endif
