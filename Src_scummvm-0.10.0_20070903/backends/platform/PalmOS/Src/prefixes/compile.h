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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/backends/platform/PalmOS/Src/prefixes/compile.h $
 * $Id: compile.h 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

#ifndef __COMPILE_H__
#define __COMPILE_H__

#define DISABLE_SCUMM
#define DISABLE_SCUMM_7_8
#define DISABLE_HE

#define DISABLE_AGOS
#define DISABLE_SKY
#define DISABLE_SWORD1
#define DISABLE_SWORD2
#define DISABLE_QUEEN
#define DISABLE_SAGA
#define DISABLE_KYRA
#define DISABLE_AWE
#define DISABLE_GOB 
#define DISABLE_LURE
#define DISABLE_CINE
#define DISABLE_AGI
#define DISABLE_TOUCHE
#define DISABLE_PARALLACTION
#define DISABLE_CRUISE

// ScummVM
#define DISABLE_HQ_SCALERS
#define DISABLE_FANCY_THEMES
//#define CT_NO_TRANSPARENCY
//#define REDUCE_MEMORY_USAGE

#include "compile_base.h"

//#define DISABLE_ADLIB
//#define DISABLE_LIGHTSPEED

#ifdef COMPILE_ZODIAC
#	undef	DISABLE_FANCY_THEMES
#	define	USE_ZLIB
// set an external ZLIB since save/load implementation
// doesn't support built-in zodiac version which is 1.1.4
// (seen inflateInit2 which err on "MAX_WBITS + 32")
#	define	USE_ZLIB_EXTERNAL
#	define	DISABLE_SONY
#endif

#ifdef COMPILE_OS5
#	define	DISABLE_TAPWAVE
#	define	USE_ZLIB
#endif

#endif
