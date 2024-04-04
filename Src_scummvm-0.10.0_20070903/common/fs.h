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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/common/fs.h $
 * $Id: fs.h 27024 2007-05-30 21:56:52Z fingolfin $
 */

#ifndef COMMON_FS_H
#define COMMON_FS_H

#include "common/array.h"
#include "common/str.h"

//namespace Common {

class FilesystemNode;
class AbstractFilesystemNode;


/**
 * List of multiple file system nodes. E.g. the contents of a given directory.
 * This is subclass instead of just a typedef so that we can use forward
 * declarations of it in other places.
 */
class FSList : public Common::Array<FilesystemNode> {};


/**
 * FilesystemNode provides an abstraction for file pathes, allowing for portable
 * file system browsing. To this ends, multiple or single roots have to be supported
 * (compare Unix with a single root, Windows with multiple roots C:, D:, ...).
 *
 * To this end, we abstract away from paths; implementations can be based on
 * paths (and it's left to them whether / or \ or : is the path separator :-);
 * but it is also possible to use inodes or vrefs (MacOS 9) or anything else.
 *
 * NOTE: Backends still have to provide a way to extract a path from a FSIntern
 *
 * You may ask now: "isn't this cheating? Why do we go through all this when we use
 * a path in the end anyway?!?".
 * Well, for once as long as we don't provide our own file open/read/write API, we
 * still have to use fopen(). Since all our targets already support fopen(), it should
 * be possible to get a fopen() compatible string for any file system node.
 *
 * Secondly, with this abstraction layer, we still avoid a lot of complications based on
 * differences in FS roots, different path separators, or even systems with no real
 * paths (MacOS 9 doesn't even have the notion of a "current directory").
 * And if we ever want to support devices with no FS in the classical sense (Palm...),
 * we can build upon this.
 */
class FilesystemNode {
private:
	AbstractFilesystemNode *_realNode;
	int *_refCount;

	FilesystemNode(AbstractFilesystemNode *realNode);

public:
	/**
	 * Flag to tell listDir() which kind of files to list.
	 */
	enum ListMode {
		kListFilesOnly = 1,
		kListDirectoriesOnly = 2,
		kListAll = 3
	};

	/**
	 * Create a new invalid FilesystemNode. In other words, isValid() for that
	 * node returns false, and if you try to get it's path, an assert is
	 * triggered.
	 */
	FilesystemNode();

	/**
	 * Create a new FilesystemNode referring to the specified path. This is
	 * the counterpart to the path() method.
	 *
	 * If path is empty or equals ".", then a node representing the "current
	 * directory" will be created. If that is not possible (since e.g. the
	 * operating system doesn't support the concept), some other directory is
	 * used (usually the root directory).
	 */
	FilesystemNode(const Common::String &path);

	/**
	 * Copy constructor.
	 */
	FilesystemNode(const FilesystemNode &node);
	
	/**
	 * Destructor.
	 */
	virtual ~FilesystemNode();

	/**
	 * Copy operator.
	 */
	FilesystemNode &operator  =(const FilesystemNode &node);

	/**
	 * Checks if the FilesystemNode is valid for any usage
	 */
	bool isValid() const;

	/**
	 * Get the parent node of this node. If this node has no parent node,
	 * then it returns a duplicate of this node.
	 */
	FilesystemNode getParent() const;

	/**
	 * Fetch a child node of this node, with the given name. Only valid for
	 * directory nodes (an assertion is triggered otherwise). If no no child
	 * node with the given name exists, an invalid node is returned.
	 */
	FilesystemNode getChild(const Common::String &name) const;

	/**
	 * Return a list of child nodes of this directory node. If called on a node
	 * that does not represent a directory, false is returned.
	 * @return true if succesful, false otherwise (e.g. when the directory does not exist).
	 * @todo Rename this to listChildren or getChildren.
	 */
	virtual bool listDir(FSList &fslist, ListMode mode = kListDirectoriesOnly) const;

	/**
	 * Is this node pointing to a directory?
	 * @todo Currently we assume that a valid node that is not a directory
	 * automatically is a file (ignoring things like symlinks). That might
	 * actually be OK... but we could still add an isFile method. Or even replace
	 * isValid and isDirectory by a getType() method that can return values like
	 * kDirNodeType, kFileNodeType, kInvalidNodeType.
	 */
	virtual bool isDirectory() const;

	/**
	 * Return a human readable string for this node, usable for display (e.g.
	 * in the GUI code). Do *not* rely on it being usable for anything else,
	 * like constructing paths!
	 * @return the display name
	 */
	virtual Common::String displayName() const;

	/**
	 * Return a string representation of the name of the file. This is can be
	 * used e.g. by detection code that relies on matching the name of a given
	 * file. But it is *not* suitable for use with fopen / File::open, nor
	 * should it be archived.
	 *
	 * @return the file name
	 */
	virtual Common::String name() const;

	/**
	 * Return a string representation of the file which can be passed to fopen(),
	 * and is suitable for archiving (i.e. writing to the config file).
	 * This will usually be a 'path' (hence the name of the method), but can
	 * be anything that fulfills the above criterions.
	 *
	 * @note Do not assume that this string contains (back)slashes or any
	 *       other kind of 'path separators'.
	 *
	 * @return the 'path' represented by this filesystem node
	 */
	virtual Common::String path() const;

	/**
	 * Compare the name of this node to the name of another. Directories
	 * go before normal files.
	 */
	bool operator< (const FilesystemNode& node) const;

protected:
	void decRefCount();
};

//} // End of namespace Common

#endif
