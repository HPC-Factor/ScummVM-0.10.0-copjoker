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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/engines/cine/gfx.h $
 * $Id: gfx.h 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

#ifndef CINE_GFX_H
#define CINE_GFX_H

namespace Cine {

void gfxDrawSprite(byte *src4, uint16 sw, uint16 sh, byte *dst4, int16 sx, int16 sy);

extern byte *page1Raw;
extern byte *page2Raw;
extern byte *page3Raw;

extern uint16 c_palette[256];

void gfxInit();
void setMouseCursor(int cursor);
void convertGfx(byte *source, byte *dest, const uint16 width, const uint16 height);
void convertGfx2(byte *source, byte *dest, const uint16 width, const uint16 height);
void gfxCopyPage(byte *source, byte *dest);

void transformPaletteRange(byte startColor, byte numColor, int8 r, int8 g, int8 b);
void gfxFlipPage(void);

void gfxSpriteFunc1(byte *ptr, byte *msk, uint16 width, uint16 height, byte *page, int16 x, int16 y);
void gfxFillSprite(byte *src4, uint16 sw, uint16 sh, byte *dst4, int16 sx, int16 sy, uint8 fillColor = 0);

void gfxSpriteFunc2(byte *spritePtr, byte *spriteMskPtr, int16 width, int16 height, byte *maskPtr,
    int16 maskWidth, int16 maskHeight, byte *bufferSprPtr, byte *bufferMskPtr, int16 xs, int16 ys, int16 xm, int16 ym, byte maskIdx);

void gfxDrawLine(int16 x1, int16 y1, int16 x2, int16 y2, byte color, byte *page);
void gfxDrawPlainBox(int16 x1, int16 y1, int16 x2, int16 y2, byte color);

void gfxResetPage(byte *pagePtr);

int16 gfxGetBit(int16 x, int16 y, byte *ptr, int16 width);

void gfxResetRawPage(byte *pageRaw);
void gfxConvertSpriteToRaw(byte *dest, byte *source, uint16 width, uint16 height);
void gfxCopyRawPage(byte *source, byte * dest);
void gfxFlipRawPage(byte *frontBuffer);
void drawSpriteRaw(byte *spritePtr, byte *maskPtr, int16 width, int16 height, byte *page, int16 x, int16 y);
void gfxDrawPlainBoxRaw(int16 x1, int16 y1, int16 x2, int16 y2, byte color, byte *page);
void drawSpriteRaw2(byte *spritePtr, byte transColor, int16 width, int16 height, byte *page, int16 x, int16 y);

void fadeToBlack(void);

void gfxFuncGen1(byte *param1, byte *param2, byte *param3, byte *param4, int16 param5);
void ptrGfxFunc13(void);
void gfxFuncGen2(void);

void blitScreen(byte *frontBuffer, byte *backbuffer);
void blitRawScreen(byte *frontBuffer);
void flip(void);

} // End of namespace Cine

#endif
