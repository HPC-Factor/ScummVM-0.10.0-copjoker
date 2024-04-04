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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/common/list.h $
 * $Id: list.h 27024 2007-05-30 21:56:52Z fingolfin $
 */

#ifndef COMMON_LIST_H
#define COMMON_LIST_H

#include "common/scummsys.h"

namespace Common {

/**
 * Simple double linked list, modeled after the list template of the standard
 * C++ library. 
 */
template <class T>
class List {
protected:
#if defined (_WIN32_WCE) || defined (_MSC_VER)
//FIXME evc4 and msvc7 doesn't like it as protected member
public:
#endif
	struct NodeBase {
		NodeBase *_prev;
		NodeBase *_next;
	};
	
	template <class T2>
	struct Node : public NodeBase {
		T2 _data;
		
		Node(const T2 &x) : _data(x) {}
	};

	template <class T2>
	class Iterator {
		friend class List<T>;
		NodeBase *_node;

#if !defined (__WINSCW__)
		explicit Iterator(NodeBase *node) : _node(node) {}
#else
		Iterator(NodeBase *node) : _node(node) {}
#endif

	public:
		Iterator() : _node(0) {}

		// Prefix inc
		Iterator<T2> &operator++() {
			if (_node)
				_node = _node->_next;
			return *this;
		}
		// Postfix inc
		Iterator<T2> operator++(int) {
			Iterator tmp(_node);
			++(*this);
			return tmp;
		}
		// Prefix dec
		Iterator<T2> &operator--() {
			if (_node)
				_node = _node->_prev;
			return *this;
		}
		// Postfix dec
		Iterator<T2> operator--(int) {
			Iterator tmp(_node);
			--(*this);
			return tmp;
		}
		T2& operator*() const {
			assert(_node);
#if (__GNUC__ == 2) && (__GNUC_MINOR__ >= 95)
			return static_cast<List<T>::Node<T2> *>(_node)->_data;
#else
			return static_cast<Node<T2>*>(_node)->_data;
#endif
		}
		T2* operator->() const {
			return &(operator*());
		}
		
		bool operator==(const Iterator<T2>& x) const {
			return _node == x._node;
		}
		
		bool operator!=(const Iterator<T2>& x) const {
			return _node != x._node;
		}
	};

	NodeBase *_anchor;

public:
	typedef Iterator<T>			iterator;
	typedef Iterator<const T>	const_iterator;

	typedef T value_type;

public:
	List() {
		_anchor = new NodeBase;
		_anchor->_prev = _anchor;
		_anchor->_next = _anchor;
	}
	List(const List<T>& list) {
		_anchor = new NodeBase;
		_anchor->_prev = _anchor;
		_anchor->_next = _anchor;

		insert(begin(), list.begin(), list.end());
	}

	~List() {
		clear();
		delete _anchor;
	}

	void push_front(const T& element) {
		insert(begin(), element);
	}

	void push_back(const T& element) {
		insert(end(), element);
	}

	void insert(iterator pos, const T& element) {
		NodeBase *newNode = new Node<T>(element);
		
		newNode->_next = pos._node;
		newNode->_prev = pos._node->_prev;
		newNode->_prev->_next = newNode;
		newNode->_next->_prev = newNode;
	}

    template <typename iterator2>
	void insert(iterator pos, iterator2 first, iterator2 last) {
		for (; first != last; ++first)
			insert(pos, *first);
	}

	iterator erase(iterator pos) {
		assert(pos != end());

		NodeBase *next = pos._node->_next;
		NodeBase *prev = pos._node->_prev;
		Node<T> *node = static_cast<Node<T> *>(pos._node);
		prev->_next = next;
		next->_prev = prev;
		delete node;
		return iterator(next);
	}

	iterator reverse_erase(iterator pos) {
		assert(pos != end());

		NodeBase *next = pos._node->_next;
		NodeBase *prev = pos._node->_prev;
		Node<T> *node = static_cast<Node<T> *>(pos._node);
		prev->_next = next;
		next->_prev = prev;
		delete node;
		return iterator(prev);
	}

	iterator erase(iterator first, iterator last) {
		while (first != last)
			erase(first++);

		return last;
	}

	void remove(const T &val) {
		iterator i = begin();
		while (i != end())
			if (val == i.operator*())
				i = erase(i);
			else
				++i;
	}


	List<T>& operator  =(const List<T>& list) {
		if (this != &list) {
			iterator i;
			const_iterator j;

			for (i = begin(), j = list.begin();  (i != end()) && (j != list.end()) ; ++i, ++j) {
				static_cast<Node<T> *>(i._node)->_data = static_cast<Node<T> *>(j._node)->_data;
			}

			if (i == end())
				insert(i, j, list.end());
			else
				erase(i, end());
		}
		
		return *this;
	}

	uint size() const {
		int n = 0;
		for (const_iterator i = begin(); i != end(); ++i)
			++n;
		return n;
	}

	void clear() {
		erase(begin(), end());
	}
	
	bool empty() const { 
		return (_anchor == _anchor->_next);
	}


	iterator		begin() {
		return iterator(_anchor->_next);
	}

	iterator		reverse_begin() {
		return iterator(_anchor->_prev);
	}

	iterator		end() {
		return iterator(_anchor);
	}

	const_iterator	begin() const {
		return const_iterator(_anchor->_next);
	}

	const_iterator	reverse_begin() const {
		return const_iterator(_anchor->_prev);
	}

	const_iterator	end() const {
		return const_iterator(_anchor);
	}
};

} // End of namespace Common

#endif
