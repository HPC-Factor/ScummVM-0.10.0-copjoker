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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/engines/sky/grid.h $
 * $Id: grid.h 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

#ifndef SKY_GRID_H
#define SKY_GRID_H

#include "common/stdafx.h"
#include "common/scummsys.h"
#include "skydefs.h"

namespace Sky {

struct Compact;
class Disk;
class SkyCompact;

class Grid {
public:
	Grid(Disk *pDisk, SkyCompact *skyCompact);
	~Grid(void);

	// grid.asm routines
	void loadGrids(void);
	void removeObjectFromWalk(Compact *cpt);
	void objectToWalk(Compact *cpt);

	// function.asm
	// note that this routine does the same as objectToWalk, it just doesn't get
	// its x, y, width parameters from cpt.
	void plotGrid(uint32 x, uint32 y, uint32 width, Compact *cpt);
	// same here, it's basically the same as removeObjectFromWalk
	void removeGrid(uint32 x, uint32 y, uint32 width, Compact *cpt);
	uint8 *giveGrid(uint32 pScreen);

private:
	void objectToWalk(uint8 gridIdx, uint32 bitNum, uint32 width);
	void removeObjectFromWalk(uint8 gridIdx, uint32 bitNum, uint32 width);
	bool getGridValues(Compact *cpt, uint8 *resGrid, uint32 *resBitNum, uint32 *resWidth);
	bool getGridValues(uint32 x, uint32 y, uint32 width, Compact *cpt, uint8 *resGrid, uint32 *resBitNum, uint32 *resWidth);

	static int8 _gridConvertTable[];
	uint8 *_gameGrids[TOT_NO_GRIDS];
	Disk *_skyDisk;
	SkyCompact *_skyCompact;
};

} // End of namespace Sky

#endif //SKYGRID_H
