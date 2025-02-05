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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/backends/intern.h $
 * $Id: intern.h 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

#ifndef BACKENDS_INTERN_H
#define BACKENDS_INTERN_H

#include "common/system.h"

#ifdef _WIN32_WCE
#define SAMPLES_PER_SEC 22050
#define SAMPLES_PER_SEC_OLD 11025
#define SAMPLES_PER_SEC_NEW 22050

#elif defined(__SYMBIAN32__)
 #ifdef SAMPLES_PER_SEC_8000 // the GreanSymbianMMP format cannot handle values for defines :(
  #define SAMPLES_PER_SEC 8000
#else
  #define SAMPLES_PER_SEC 16000
 #endif

#elif defined(__PLAYSTATION2__)
#define SAMPLES_PER_SEC 48000 // the SPU can't handle anything else

#elif defined(__PSP__)
#define	SAMPLES_PER_SEC	44100

#elif defined(__GP2X__)
#define	SAMPLES_PER_SEC	11025

#elif defined(PALMOS_MODE)
#	ifdef PALMOS_ARM
#		ifdef COMPILE_ZODIAC
#			define SAMPLES_PER_SEC 44100
#		else
#			define SAMPLES_PER_SEC 22050
#		endif
#	else
#		define SAMPLES_PER_SEC 8000
#	endif

#else
//#define SAMPLES_PER_SEC 11025
#define SAMPLES_PER_SEC 22050
//#define SAMPLES_PER_SEC 44100
#endif

#endif
