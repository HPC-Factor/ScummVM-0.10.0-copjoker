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

 * This program is distributed file the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/engines/scumm/thumbnail.cpp $
 * $Id: thumbnail.cpp 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

#include "common/stdafx.h"
#include "common/system.h"
#include "common/savefile.h"
#include "graphics/scaler.h"
#include "scumm/scumm.h"

namespace Scumm {

#define THMB_VERSION 1

struct ThumbnailHeader {
	uint32 type;
	uint32 size;
	byte version;
	uint16 width, height;
	byte bpp;
};

#define ThumbnailHeaderSize (4+4+1+2+2+1)

inline void colorToRGB(uint16 color, uint8 &r, uint8 &g, uint8 &b) {
	r = (((color >> 11) & 0x1F) << 3);
	g = (((color >> 5) & 0x3F) << 2);
	b = ((color&0x1F) << 3);
}

Graphics::Surface *ScummEngine::loadThumbnail(Common::InSaveFile *file) {
	ThumbnailHeader header;

	header.type = file->readUint32BE();
	// We also accept the bad 'BMHT' header here, for the sake of compatibility
	// with some older savegames which were written incorrectly due to a bug in
	// ScummVM which wrote the thumb header type incorrectly on LE systems.
	if (header.type != MKID_BE('THMB') && header.type != MKID_BE('BMHT'))
		return 0;

	header.size = file->readUint32BE();
	header.version = file->readByte();

	if (header.version > THMB_VERSION) {
		file->skip(header.size - 9);
		warning("Loading a newer thumbnail version");
		return 0;
	}

	header.width = file->readUint16BE();
	header.height = file->readUint16BE();
	header.bpp = file->readByte();

	// TODO: support other bpp values than 2
	if (header.bpp != 2) {
		file->skip(header.size - 14);
		return 0;
	}

	Graphics::Surface *thumb = new Graphics::Surface();
	thumb->create(header.width, header.height, sizeof(uint16));

	uint16* pixels = (uint16 *)thumb->pixels;

	for (int y = 0; y < thumb->h; ++y) {
		for (int x = 0; x < thumb->w; ++x) {
			uint8 r, g, b;
			colorToRGB(file->readUint16BE(), r, g, b);

			// converting to current OSystem Color
			*pixels++ = _system->RGBToColor(r, g, b);
		}
	}

	return thumb;
}

void ScummEngine::saveThumbnail(Common::OutSaveFile *file) {
	Graphics::Surface thumb;

#if !defined(PALMOS_68K) || !defined(__DS__)
	if (!createThumbnailFromScreen(&thumb))
#endif
		thumb.create(kThumbnailWidth, kThumbnailHeight2, sizeof(uint16));

	ThumbnailHeader header;
	header.type = MKID_BE('THMB');
	header.size = ThumbnailHeaderSize + thumb.w*thumb.h*thumb.bytesPerPixel;
	header.version = THMB_VERSION;
	header.width = thumb.w;
	header.height = thumb.h;
	header.bpp = thumb.bytesPerPixel;

	file->writeUint32BE(header.type);
	file->writeUint32BE(header.size);
	file->writeByte(header.version);
	file->writeUint16BE(header.width);
	file->writeUint16BE(header.height);
	file->writeByte(header.bpp);

	// TODO: for later this shouldn't be casted to uint16...
	uint16* pixels = (uint16 *)thumb.pixels;
	for (uint16 p = 0; p < thumb.w*thumb.h; ++p, ++pixels)
		file->writeUint16BE(*pixels);

	thumb.free();
}

} // end of namespace Scumm
