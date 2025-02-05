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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */
 
 #ifndef _BLITTERS_H_
 #define _BLITTERS_H_

namespace DS {

void asmDrawStripToScreen(int height, int width, byte const* text, byte const* src, byte* dst, 
	int vsPitch, int vmScreenWidth, int textSurfacePitch);
void asmCopy8Col(byte* dst, int dstPitch, const byte* src, int height);
void Rescale_320x256xPAL8_To_256x256x1555(u16* dest, const u8* src, const u16* palette, int destStride, int srcStride);
void Rescale_320x256x1555_To_256x256x1555(u16* dest, const u16* src, int destStride, int srcStride);

}
	
#endif
