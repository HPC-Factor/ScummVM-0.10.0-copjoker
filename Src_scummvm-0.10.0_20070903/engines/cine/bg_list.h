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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/engines/cine/bg_list.h $
 * $Id: bg_list.h 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

#ifndef CINE_BGLIST_H
#define CINE_BGLIST_H

#include "common/stdafx.h"
#include "common/scummsys.h"
#include "common/savefile.h"

namespace Cine {

struct BGIncrustList {
	struct BGIncrustList *next;
	byte *unkPtr;
	int16 objIdx;
	int16 param;
	int16 x;
	int16 y;
	int16 frame;
	int16 part;
};

extern BGIncrustList *bgIncrustList;
extern uint32 var8;

void addToBGList(int16 objIdx, bool addList = true);
void addSpriteFilledToBGList(int16 idx, bool addList = true);

void createBgIncrustListElement(int16 objIdx, int16 param);
void freeBgIncrustList(void);
void resetBgIncrustList(void);
void loadBgIncrustFromSave(Common::InSaveFile *fHandle);
void reincrustAllBg(void);

} // End of namespace Cine

#endif
