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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/backends/fs/abstract-fs.h $
 * $Id: abstract-fs.h 27024 2007-05-30 21:56:52Z fingolfin $
 */

#ifndef BACKENDS_ABSTRACT_FS_H
#define BACKENDS_ABSTRACT_FS_H

#include "common/array.h"
#include "common/str.h"

#include "common/fs.h"

class AbstractFilesystemNode;

typedef Common::Array<AbstractFilesystemNode *>	AbstractFSList;

/**
 * Abstract file system node. Private subclasses implement the actual
 * functionality.
 *
 * Most of the methods correspond directly to methods in class FilesystemNode,
 * so if they are not documented here, look there for more information about
 * the semantics.
 */
class AbstractFilesystemNode {
protected:
	friend class FilesystemNode;
	typedef Common::String String;
	typedef FilesystemNode::ListMode ListMode;

	/**
	 * The parent node of this directory.
	 * The parent of the root is the root itself.
	 */
	virtual AbstractFilesystemNode *parent() const = 0;

	/**
	 * The child node with the given name. If no child with this name
	 * exists, returns 0. When called on a non-directory node, it should
	 * handle this gracefully by returning 0.
	 *
	 * @note Handling calls on non-dir nodes gracefully makes it possible to
	 * switch to a lazy type detection scheme in the future.
	 */
	virtual AbstractFilesystemNode *child(const String &name) const = 0;


	/**
	 * Returns a special node representing the FS root. The starting point for
	 * any file system browsing.
	 * On Unix, this will be simply the node for / (the root directory).
	 * On Windows, it will be a special node which "contains" all drives (C:, D:, E:).
	 */
	static AbstractFilesystemNode *getRoot();

	/**
	 * Returns a node representing the "current directory". If your system does
	 * not support this concept, you can either try to emulate it or
	 * simply return some "sensible" default directory node, e.g. the same
	 * value as getRoot() returns.
	 */
	static AbstractFilesystemNode *getCurrentDirectory();


	/**
	 * Construct a node based on a path; the path is in the same format as it
	 * would be for calls to fopen().
	 *
	 * Furthermore getNodeForPath(oldNode.path()) should create a new node
	 * identical to oldNode. Hence, we can use the "path" value for persistent
	 * storage e.g. in the config file.
	 *
	 * @todo: This is of course a place where non-portable code easily will sneak
	 *        in, because the format of the path used here is not well-defined.
	 *        So we really should reconsider this API and try to come up with
	 *        something which is more portable but still flexible enough for our
	 *        purposes.
	 */
	static AbstractFilesystemNode *getNodeForPath(const String &path);


public:
	virtual ~AbstractFilesystemNode() {}

	virtual String name() const = 0;
	
	// By default, we use the actual file name as 'display name'.
	virtual String displayName() const { return name(); }

	virtual bool isValid() const = 0;

	virtual bool isDirectory() const = 0;
	
	/**
	 * Return the 'path' of the current node, usable in fopen(). See also
	 * the static getNodeForPath() method.
	 */
	virtual String path() const = 0;
	virtual bool listDir(AbstractFSList &list, ListMode mode) const = 0;


	/* TODO:
	bool exists();

	bool isDirectory();
	bool isFile();

	bool isReadable();
	bool isWriteable();
	*/
};


#endif
