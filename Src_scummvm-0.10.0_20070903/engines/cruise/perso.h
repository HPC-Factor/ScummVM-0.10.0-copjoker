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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/engines/cruise/perso.h $
 * $Id: perso.h 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

#ifndef CRUISE_PERSO_H
#define CRUISE_PERSO_H

namespace Cruise {

enum {
	NUM_NODES = 20,
	NUM_PERSONS = 10
};

struct persoStruct {
	int16 inc_droite;	// 2
	int16 inc_droite0;	// 2
	int16 inc_chemin;	// 2
	int16 coordinates[400][2];	// 1600
	int16 solution[NUM_NODES + 3][2];	//((20+3)*2*2)
	int16 inc_jo1;		// 2
	int16 inc_jo2;		// 2
	int16 dir_perso;	// 2
	int16 inc_jo0;		// 2
};

extern persoStruct *persoTable[NUM_PERSONS];
extern int16 computedVar14;

void freePerso(int persoIdx);
void freeAllPerso(void);
void affiche_chemin(int16 persoIdx, int16 * returnVar);

} // End of namespace Cruise

#endif
