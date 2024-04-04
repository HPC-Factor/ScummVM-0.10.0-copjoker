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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/engines/saga/stream.h $
 * $Id: stream.h 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

#ifndef SAGA_STREAM_H
#define SAGA_STREAM_H

#include "common/stream.h"

namespace Saga {

using Common::MemoryReadStream;

class MemoryReadStreamEndian : public Common::MemoryReadStream {
private:
public:
	bool _bigEndian;
	MemoryReadStreamEndian(const byte *buf, uint32 len, bool bigEndian = false) : MemoryReadStream(buf, len), _bigEndian(bigEndian) {}

	uint16 readUint16() {
		return (_bigEndian) ? readUint16BE(): readUint16LE();
	}

	uint32 readUint32() {
		return (_bigEndian) ? readUint32BE(): readUint32LE();
	}

	inline int16 readSint16() {
		return (int16)readUint16();
	}


	inline int32 readSint32() {
		return (int32)readUint32();
	}
};

} // End of namespace Saga
#endif
