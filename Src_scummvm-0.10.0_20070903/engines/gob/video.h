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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/engines/gob/video.h $
 * $Id: video.h 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

#ifndef GOB_VIDEO_H
#define GOB_VIDEO_H

#include "gob/gob.h"

namespace Gob {

// Some Surfaces are simultaneous in Draw::spritesArray and discrete
// variables, so it's a references counting class that cleans
// up its own mess
class SurfaceDesc : public ReferenceCounter<SurfaceDesc> {
public:
	int16 _vidMode;

	int16 getWidth() { return _width; }
	int16 getHeight() { return _height; }
	byte *getVidMem() { return _vidMem; }
	bool hasOwnVidMem() { return _ownVidMem; }

	void setVidMem(byte *vidMem);
	void resize(int16 width, int16 height);
	void swap(SurfaceDesc &surf);
	void swap(SurfaceDesc *surf) { assert(surf); swap(*surf); }

	SurfaceDesc(int16 vidMode, int16 width, int16 height, byte *vidMem = 0);
	~SurfaceDesc() { if (_ownVidMem) delete[] _vidMem; }

private:
	int16 _width;
	int16 _height;
	byte *_vidMem;
	bool _ownVidMem;
};

class Video {
public:
	struct FontDesc {
		byte *dataPtr;
		int8 itemWidth;
		int8 itemHeight;
		int8 startItem;
		int8 endItem;
		int8 itemSize;
		int8 bitWidth;
		byte *extraData;
		FontDesc() : dataPtr(0), itemWidth(0), itemHeight(0), startItem(0),
			               endItem(0), itemSize(0), bitWidth(0) {}
	};

#define GDR_VERSION	4

#define PRIMARY_SURFACE		0x80
#define RETURN_PRIMARY		0x01
#define DISABLE_SPR_ALLOC	0x20
#define SCUMMVM_CURSOR		0x100

#include "common/pack-start.h"	// START STRUCT PACKING

	struct Color {
		byte red;
		byte green;
		byte blue;
	};

#include "common/pack-end.h"	// END STRUCT PACKING

	struct PalDesc {
		Color *vgaPal;
		int16 *unused1;
		int16 *unused2;
		PalDesc() : vgaPal(0), unused1(0), unused2(0) {}
	};

	bool _doRangeClamp;
	int16 _surfWidth;
	int16 _surfHeight;
	int16 _scrollOffsetX;
	int16 _scrollOffsetY;
	int16 _splitHeight1;
	int16 _splitHeight2;
	int16 _splitStart;

	void freeDriver();
	void initPrimary(int16 mode);
	SurfaceDesc *initSurfDesc(int16 vidMode, int16 width,
			int16 height, int16 flags);
	void retrace(bool mouse = true);
	void waitRetrace(bool mouse = true);
	void sparseRetrace(int max);

	void putPixel(int16 x, int16 y, int16 color, SurfaceDesc *dest);
	void fillRect(SurfaceDesc *dest, int16 left, int16 top,
			int16 right, int16 bottom, int16 color);
	void drawLine(SurfaceDesc *dest, int16 x0, int16 y0, int16 x1, int16 y1,
				  int16 color);
	void drawCircle(SurfaceDesc *dest, int16 x0, int16 y0,
			int16 radius, int16 color);
	void clearSurf(SurfaceDesc *dest);
	void drawSprite(SurfaceDesc *source, SurfaceDesc *dest,
			int16 left, int16 top, int16 right, int16 bottom,
			int16 x, int16 y, int16 transp);
	void drawLetter(int16 item, int16 x, int16 y, FontDesc *fontDesc,
			int16 color1, int16 color2, int16 transp, SurfaceDesc *dest);
	void drawPackedSprite(byte *sprBuf, int16 width, int16 height,
			int16 x, int16 y, int16 transp, SurfaceDesc *dest);
	void drawPackedSprite(const char *path, SurfaceDesc *dest,
			int width = 320);

	void setPalColor(byte *pal, byte red, byte green, byte blue) {
		pal[0] = red << 2;
		pal[1] = green << 2;
		pal[2] = blue << 2;
		pal[3] = 0;
	}
	void setPalColor(byte *pal, Color &color) {
		setPalColor(pal, color.red, color.green, color.blue);
	}
	void setPalElem(int16 index, char red, char green, char blue,
			int16 unused, int16 vidMode);
	void setPalette(PalDesc *palDesc);
	void setFullPalette(PalDesc *palDesc);
	void setPalette(Color *palette);

	virtual char spriteUncompressor(byte *sprBuf, int16 srcWidth,
			int16 srcHeight, int16 x, int16 y, int16 transp,
			SurfaceDesc *destDesc) = 0;

	Video(class GobEngine *vm);
	virtual ~Video() {}

protected:
	class VideoDriver *_videoDriver;

	int _curSparse;
	uint32 _lastSparse;

	GobEngine *_vm;

	char initDriver(int16 vidMode);
};

class Video_v1 : public Video {
public:
	virtual char spriteUncompressor(byte *sprBuf, int16 srcWidth, int16 srcHeight,
			int16 x, int16 y, int16 transp, SurfaceDesc *destDesc);

	Video_v1(GobEngine *vm);
	virtual ~Video_v1() {}
};

class Video_v2 : public Video_v1 {
public:
	virtual char spriteUncompressor(byte *sprBuf, int16 srcWidth, int16 srcHeight,
			int16 x, int16 y, int16 transp, SurfaceDesc *destDesc);

	Video_v2(GobEngine *vm);
	virtual ~Video_v2() {}
};

class VideoDriver {
public:
	VideoDriver() {}
	virtual ~VideoDriver() {}
	virtual void drawSprite(SurfaceDesc *source, SurfaceDesc *dest, int16 left, int16 top, int16 right, int16 bottom, int16 x, int16 y, int16 transp) = 0;
	virtual void fillRect(SurfaceDesc *dest, int16 left, int16 top, int16 right, int16 bottom, byte color) = 0;
	virtual void putPixel(int16 x, int16 y, byte color, SurfaceDesc *dest) = 0;
	virtual void drawLetter(unsigned char item, int16 x, int16 y, Video::FontDesc *fontDesc, byte color1, byte color2, byte transp, SurfaceDesc *dest) = 0;
	virtual void drawLine(SurfaceDesc *dest, int16 x0, int16 y0, int16 x1, int16 y1, byte color) = 0;
	virtual void drawPackedSprite(byte *sprBuf, int16 width, int16 height, int16 x, int16 y, byte transp, SurfaceDesc *dest) = 0;
};

} // End of namespace Gob

#endif // GOB_VIDEO_H
