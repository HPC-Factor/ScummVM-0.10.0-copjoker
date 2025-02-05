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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/engines/saga/gfx.cpp $
 * $Id: gfx.cpp 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

// Misc. graphics routines

#include "saga/saga.h"
#include "saga/gfx.h"
#include "saga/interface.h"
#include "saga/sagaresnames.h"
#include "saga/rscfile.h"
#include "saga/scene.h"
#include "saga/stream.h"

#include "common/system.h"
#include "graphics/cursorman.h"

namespace Saga {

Gfx::Gfx(SagaEngine *vm, OSystem *system, int width, int height) : _vm(vm), _system(system) {
	_system->beginGFXTransaction();
		_vm->initCommonGFX(width > 320);
		_system->initSize(width, height);
	_system->endGFXTransaction();

	debug(5, "Init screen %dx%d", width, height);
	// Convert surface data to R surface data
	_backBuffer.create(width, height, 1);

	// Set module data
	_init = 1;

	// Start with the cursor shown. It will be hidden before the intro, if
	// there is an intro. (With boot params, there may not be.)
	setCursor(kCursorNormal);
	showCursor(true);
}

Gfx::~Gfx() {
	_backBuffer.free();
}

void Surface::drawPalette() {
	int x;
	int y;
	int color = 0;
	Rect palRect;

	for (y = 0; y < 16; y++) {
		palRect.top = (y * 8) + 4;
		palRect.bottom = palRect.top + 8;

		for (x = 0; x < 16; x++) {
			palRect.left = (x * 8) + 4;
			palRect.right = palRect.left + 8;

			drawRect(palRect, color);
			color++;
		}
	}
}

// * Copies a rectangle from a raw 8 bit pixel buffer to the specified surface.
// - The surface must match the logical dimensions of the buffer exactly.
void Surface::blit(const Common::Rect &destRect, const byte *sourceBuffer) {
	const byte *readPointer;
	byte *writePointer;
	int row;
	ClipData clipData;

	clipData.sourceRect.left = 0;
	clipData.sourceRect.top = 0;
	clipData.sourceRect.right = destRect.width();
	clipData.sourceRect.bottom = destRect.height();

	clipData.destPoint.x = destRect.left;
	clipData.destPoint.y = destRect.top;
	clipData.destRect.left = 0;
	clipData.destRect.right = w;
	clipData.destRect.top = 0;
	clipData.destRect.bottom = h;

	if (!clipData.calcClip()) {
		return;
	}

	// Transfer buffer data to surface
	readPointer = (sourceBuffer + clipData.drawSource.x) +
						(clipData.sourceRect.right * clipData.drawSource.y);

	writePointer = ((byte *)pixels + clipData.drawDest.x) + (pitch * clipData.drawDest.y);

	for (row = 0; row < clipData.drawHeight; row++) {
		memcpy(writePointer, readPointer, clipData.drawWidth);

		writePointer += pitch;
		readPointer += clipData.sourceRect.right;
	}
}

void Surface::drawPolyLine(const Point *points, int count, int color) {
	int i;
	if (count >= 3) {
		for (i = 1; i < count; i++) {
			drawLine(points[i].x, points[i].y, points[i - 1].x, points[i - 1].y, color);
		}

		drawLine(points[count - 1].x, points[count - 1].y, points->x, points->y, color);
	}
}

/**
* Dissolve one image with another.
* If flags if set to 1, do zero masking.
*/
void Surface::transitionDissolve(const byte *sourceBuffer, const Common::Rect &sourceRect, int flags, double percent) {
#define XOR_MASK 0xB400;
	int pixelcount = w * h;
	int seqlimit = (int)(65535 * percent);
	int seq = 1;
	int i, x1, y1;
	byte color;

	for (i = 0; i < seqlimit; i++) {
		if (seq & 1) {
			seq = (seq >> 1) ^ XOR_MASK;
		} else {
			seq = seq >> 1;
		}

		if (seq == 1) {
			return;
		}

		if (seq >= pixelcount) {
			continue;
		} else {
			x1 = seq % w;
			y1 = seq / w;

			if (sourceRect.contains(x1, y1)) {
				color = sourceBuffer[(x1-sourceRect.left) + sourceRect.width()*(y1-sourceRect.top)];
				if (flags == 0 || color)
					((byte*)pixels)[seq] = color;
			}
		}
	}
}

void Gfx::initPalette() {
	if (_vm->getGameType() != GType_IHNM)
		return;

	ResourceContext *resourceContext = _vm->_resource->getContext(GAME_RESOURCEFILE);
	if (resourceContext == NULL) {
		error("Resource::loadGlobalResources() resource context not found");
	}

	byte *resourcePointer;
	size_t resourceLength;

	_vm->_resource->loadResource(resourceContext, RID_IHNM_DEFAULT_PALETTE,
								 resourcePointer, resourceLength);

	MemoryReadStream metaS(resourcePointer, resourceLength);

	for (int i = 0; i < 256; i++) {
		_globalPalette[i].red = metaS.readByte();
		_globalPalette[i].green = metaS.readByte();
		_globalPalette[i].blue = metaS.readByte();
	}

	free(resourcePointer);

	setPalette(_globalPalette, true);
}

void Gfx::setPalette(const PalEntry *pal, bool full) {
	int i;
	byte *ppal;
	int from, numcolors;

	if (_vm->getGameType() != GType_IHNM || full) {
		from = 0;
		numcolors = PAL_ENTRIES;
	} else {
		from = 0;
		numcolors = 248;
	}

	for (i = 0, ppal = &_currentPal[from * 4]; i < numcolors; i++, ppal += 4) {
		ppal[0] = _globalPalette[i].red = pal[i].red;
		ppal[1] = _globalPalette[i].green = pal[i].green;
		ppal[2] = _globalPalette[i].blue = pal[i].blue;
		ppal[3] = 0;
	}

	// Color 0 should always be black in IHNM
	if (_vm->getGameType() == GType_IHNM)
		memset(&_currentPal[0 * 4], 0, 4);

	// Make 256th color black. See bug #1256368
	if ((_vm->getPlatform() == Common::kPlatformMacintosh) && !_vm->_scene->isInIntro())
		memset(&_currentPal[255 * 4], 0, 4);

	_system->setPalette(_currentPal, 0, PAL_ENTRIES);
}

void Gfx::setPaletteColor(int n, int r, int g, int b) {
	bool update = false;

	// This function may get called a lot. To avoid forcing full-screen
	// updates, only update the palette if the color actually changes.

	if (_currentPal[4 * n + 0] != r) {
		_currentPal[4 * n + 0] = _globalPalette[n].red = r;
		update = true;
	}
	if (_currentPal[4 * n + 1] != g) {
		_currentPal[4 * n + 1] = _globalPalette[n].green = g;
		update = true;
	}
	if (_currentPal[4 * n + 2] != b) {
		_currentPal[4 * n + 2] = _globalPalette[n].blue = b;
		update = true;
	}
	if (_currentPal[4 * n + 3] != 0) {
		_currentPal[4 * n + 3] = 0;
		update = true;
	}

	if (update)
		_system->setPalette(_currentPal, n, 1);
}

void Gfx::getCurrentPal(PalEntry *src_pal) {
	int i;
	byte *ppal;

	for (i = 0, ppal = _currentPal; i < PAL_ENTRIES; i++, ppal += 4) {
		src_pal[i].red = ppal[0];
		src_pal[i].green = ppal[1];
		src_pal[i].blue = ppal[2];
	}
}

void Gfx::palToBlack(PalEntry *srcPal, double percent) {
	int i;
	//int fade_max = 255;
	int new_entry;
	byte *ppal;
	PalEntry *palE;
	int from, numcolors;

	double fpercent;

	if (_vm->getGameType() != GType_IHNM) {
		from = 0;
		numcolors = PAL_ENTRIES;
	} else {
		from = 0;
		numcolors = 248;
	}

	if (percent > 1.0) {
		percent = 1.0;
	}

	// Exponential fade
	fpercent = percent * percent;

	fpercent = 1.0 - fpercent;

	// Use the correct percentage change per frame for each palette entry
	for (i = 0, ppal = _currentPal; i < PAL_ENTRIES; i++, ppal += 4) {
		if (i < from || i >= from + numcolors)
			palE = &_globalPalette[i];
		else
			palE = &srcPal[i];

		new_entry = (int)(palE->red * fpercent);

		if (new_entry < 0) {
			ppal[0] = 0;
		} else {
			ppal[0] = (byte) new_entry;
		}

		new_entry = (int)(palE->green * fpercent);

		if (new_entry < 0) {
			ppal[1] = 0;
		} else {
			ppal[1] = (byte) new_entry;
		}

		new_entry = (int)(palE->blue * fpercent);

		if (new_entry < 0) {
			ppal[2] = 0;
		} else {
			ppal[2] = (byte) new_entry;
		}
		ppal[3] = 0;
	}

	// Color 0 should always be black in IHNM
	if (_vm->getGameType() == GType_IHNM)
		memset(&_currentPal[0 * 4], 0, 4);

	// Make 256th color black. See bug #1256368
	if ((_vm->getPlatform() == Common::kPlatformMacintosh) && !_vm->_scene->isInIntro())
		memset(&_currentPal[255 * 4], 0, 4);

	_system->setPalette(_currentPal, 0, PAL_ENTRIES);
}

void Gfx::blackToPal(PalEntry *srcPal, double percent) {
	int new_entry;
	double fpercent;
	byte *ppal;
	int i;
	PalEntry *palE;
	int from, numcolors;

	if (_vm->getGameType() != GType_IHNM) {
		from = 0;
		numcolors = PAL_ENTRIES;
	} else {
		from = 0;
		numcolors = 248;
	}

	if (percent > 1.0) {
		percent = 1.0;
	}

	// Exponential fade
	fpercent = percent * percent;

	fpercent = 1.0 - fpercent;

	// Use the correct percentage change per frame for each palette entry
	for (i = 0, ppal = _currentPal; i < PAL_ENTRIES; i++, ppal += 4) {
		if (i < from || i >= from + numcolors)
			palE = &_globalPalette[i];
		else
			palE = &srcPal[i];

		new_entry = (int)(palE->red - palE->red * fpercent);

		if (new_entry < 0) {
			ppal[0] = 0;
		} else {
			ppal[0] = (byte)new_entry;
		}

		new_entry = (int)(palE->green - palE->green * fpercent);

		if (new_entry < 0) {
			ppal[1] = 0;
		} else {
			ppal[1] = (byte) new_entry;
		}

		new_entry = (int)(palE->blue - palE->blue * fpercent);

		if (new_entry < 0) {
			ppal[2] = 0;
		} else {
			ppal[2] = (byte) new_entry;
		}
		ppal[3] = 0;
	}

	// Color 0 should always be black in IHNM
	if (_vm->getGameType() == GType_IHNM)
		memset(&_currentPal[0 * 4], 0, 4);

	// Make 256th color black. See bug #1256368
	if ((_vm->getPlatform() == Common::kPlatformMacintosh) && !_vm->_scene->isInIntro())
		memset(&_currentPal[255 * 4], 0, 4);

	_system->setPalette(_currentPal, 0, PAL_ENTRIES);
}

void Gfx::showCursor(bool state) {
	CursorMan.showMouse(state);
}

void Gfx::setCursor(CursorType cursorType) {
	if (_vm->getGameType() == GType_ITE) {
		// Set up the mouse cursor
		const byte A = kITEColorLightGrey;
		const byte B = kITEColorWhite;

		const byte cursor_img[CURSOR_W * CURSOR_H] = {
			0, 0, 0, A, 0, 0, 0,
			0, 0, 0, A, 0, 0, 0,
			0, 0, 0, A, 0, 0, 0,
			A, A, A, B, A, A, A,
			0, 0, 0, A, 0, 0, 0,
			0, 0, 0, A, 0, 0, 0,
			0, 0, 0, A, 0, 0, 0,
		};

		CursorMan.replaceCursor(cursor_img, CURSOR_W, CURSOR_H, 3, 3, 0);
	} else {
		uint32 resourceId;

		switch (cursorType) {
		case kCursorBusy:
			resourceId = RID_IHNM_HOURGLASS_CURSOR;
			break;
		default:
			resourceId = (uint32)-1;
			break;
		}

		byte *resource;
		size_t resourceLength;
		byte *image;
		size_t imageLength;
		int width, height;

		if (resourceId != (uint32)-1) {
			ResourceContext *context = _vm->_resource->getContext(GAME_RESOURCEFILE);

			_vm->_resource->loadResource(context, resourceId, resource, resourceLength);

			_vm->decodeBGImage(resource, resourceLength, &image, &imageLength, &width, &height);
		} else {
			resource = NULL;
			width = height = 31;
			image = (byte *)calloc(width, height);

			for (int i = 0; i < 14; i++) {
				image[15 * 31 + i] = 1;
				image[15 * 31 + 30 - i] = 1;
				image[i * 31 + 15] = 1;
				image[(30 - i) * 31 + 15] = 1;
			}
		}

		// Note: Hard-coded hotspot
		CursorMan.replaceCursor(image, width, height, 15, 15, 0);

		free(image);
		free(resource);
	}
}

bool hitTestPoly(const Point *points, unsigned int npoints, const Point& test_point) {
	int yflag0;
	int yflag1;
	bool inside_flag = false;
	unsigned int pt;

	const Point *vtx0 = &points[npoints - 1];
	const Point *vtx1 = &points[0];

	yflag0 = (vtx0->y >= test_point.y);
	for (pt = 0; pt < npoints; pt++, vtx1++) {
		yflag1 = (vtx1->y >= test_point.y);
		if (yflag0 != yflag1) {
			if (((vtx1->y - test_point.y) * (vtx0->x - vtx1->x) >=
				(vtx1->x - test_point.x) * (vtx0->y - vtx1->y)) == yflag1) {
				inside_flag = !inside_flag;
			}
		}
		yflag0 = yflag1;
		vtx0 = vtx1;
	}

	return inside_flag;
}

} // End of namespace Saga
