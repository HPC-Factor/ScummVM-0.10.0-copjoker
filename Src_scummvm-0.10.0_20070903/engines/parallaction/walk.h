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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/engines/parallaction/walk.h $
 * $Id: walk.h 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

#ifndef PARALLACTION_WALK_H
#define PARALLACTION_WALK_H

#include "parallaction/defs.h"

namespace Parallaction {

struct Animation;
struct Job;

struct WalkNode {
	int16	_x;
	int16	_y;

public:
	WalkNode();
	WalkNode(int16 x, int16 y);
	WalkNode(const WalkNode& w);

	void getPoint(Common::Point &p) const;
};

typedef ManagedList<WalkNode*> WalkNodeList;


void 		 jobWalk(void*, Job *j);


class PathBuilder {

	Animation 		*_anim;

	WalkNodeList 	*_list;
	Common::List<WalkNode*> 	_subPath;

	void correctPathPoint(Common::Point &to);
	uint32 buildSubPath(const Common::Point& pos, const Common::Point& stop);
	uint16 walkFunc1(int16 x, int16 y, WalkNode *Node);

public:
	PathBuilder(Animation *anim);
	WalkNodeList* buildPath(uint16 x, uint16 y);

};


}

#endif
