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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/engines/lure/screen.h $
 * $Id: screen.h 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

#ifndef LURE_SCREEN_H
#define LURE_SCREEN_H

#include "common/stdafx.h"
#include "engines/engine.h"
#include "lure/luredefs.h"
#include "lure/palette.h"
#include "lure/disk.h"
#include "lure/memory.h"
#include "lure/surface.h"

namespace Lure {

class Screen {
private:
	OSystem &_system;
	Disk &_disk;
	Surface *_screen;
	Palette *_palette;
public:
	Screen(OSystem &system);
	~Screen();
	static Screen &getReference();

	void setPaletteEmpty();
	void setPalette(Palette *p);
	Palette &getPalette() { return *_palette; }
	void paletteFadeIn(Palette *p);
	void paletteFadeOut();
	void resetPalette();
	void empty();
	void update();
	void updateArea(uint16 x, uint16 y, uint16 w, uint16 h);

	Surface &screen() { return *_screen; }
	uint8 *screen_raw() { return _screen->data().data(); }
	uint8 *pixel_raw(uint16 x, uint16 y) { return screen_raw() + (y * FULL_SCREEN_WIDTH) + x; }
};

} // End of namespace Lure

#endif
