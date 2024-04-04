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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/backends/platform/PalmOS/Src/native/oscalls.cpp $
 * $Id: oscalls.cpp 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

#include <PenInputMgr.h>
#include "pace.h"

GlobalsType global;


PACE_CLASS_WRAPPER(Err)
	__68k_StatShow(void) {
	PACE_PIN_EXEC_NP(pinStatShow, Err)
}

PACE_CLASS_WRAPPER(Err)
	__68k_StatHide(void) {
	PACE_PIN_EXEC_NP(pinStatHide, Err)
}

PACE_CLASS_WRAPPER(Err)
	__68k_PINSetInputAreaState(UInt16 state) {
	PACE_PARAMS_INIT()
	PACE_PARAMS_ADD16(state)
	PACE_PARAMS_END()
	PACE_PIN_EXEC(pinPINSetInputAreaState, Err)
}

PACE_CLASS_WRAPPER(Err)
	__68k_SysSetOrientation(UInt16 orientation) {
	PACE_PARAMS_INIT()
	PACE_PARAMS_ADD16(orientation)
	PACE_PARAMS_END()
	PACE_PIN_EXEC(pinSysSetOrientation, Err)
}

PACE_CLASS_WRAPPER(UInt16)
	__68k_SysGetOrientation(void) {
	PACE_PIN_EXEC_NP(pinSysGetOrientation, UInt16)
}

