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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/engines/cine/prc.h $
 * $Id: prc.h 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

#ifndef CINE_PRC_H
#define CINE_PRC_H

namespace Cine {

struct prcLinkedListStruct {
	struct prcLinkedListStruct *next;
	int16 stack[SCRIPT_STACK_SIZE];
	int16 localVars[50];
	uint16 compareResult;
	uint16 scriptPosition;
	byte *scriptPtr;
	int16 scriptIdx;
};

extern prcLinkedListStruct globalScriptsHead;
extern prcLinkedListStruct objScriptList;

void resetglobalScriptsHead(void);
void freePrcLinkedList(void);
void loadPrc(const char *pPrcName);

} // End of namespace Cine

#endif
