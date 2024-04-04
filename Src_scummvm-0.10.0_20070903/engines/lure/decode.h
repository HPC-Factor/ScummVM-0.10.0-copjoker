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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/engines/lure/decode.h $
 * $Id: decode.h 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

#ifndef LURE_DECODE_H
#define LURE_DECODE_H

#include "common/stdafx.h"
#include "lure/luredefs.h"
#include "lure/memory.h"

namespace Lure {

class PictureDecoder {
private:
	byte *dataIn;
	uint32 BP;
	uint32 dataPos, dataPos2;
	uint32 outputOffset;
	byte AL, AH;
	byte CH, CL;

	void writeByte(MemoryBlock *dest, byte v);
	void writeBytes(MemoryBlock *dest, byte v, uint16 numBytes);
	byte DSSI(bool incr = true);
	byte ESBX(bool incr = true);
	void decrCtr();
	bool shlCarry();
	void swap(uint16 &v1, uint16 &v2);
public:
	MemoryBlock *decode(MemoryBlock *src, uint32 maxOutputSize = SCREEN_SIZE);
};

class AnimationDecoder {
public:
	static void rcl(uint16 &value, bool &carry);
	static uint32 decode_data(MemoryBlock *src, MemoryBlock *dest, uint32 srcPos);
	static void decode_data_2(byte *&pSrc, uint16 &currData, uint16 &bitCtr, 
					   uint16 &dx, bool &carry);
};

} // End of namespace Lure

#endif
