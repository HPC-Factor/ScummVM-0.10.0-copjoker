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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/engines/scumm/smush/saud_channel.cpp $
 * $Id: saud_channel.cpp 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

#include "common/stdafx.h"
#include "common/endian.h"

#include "scumm/util.h"
#include "scumm/smush/channel.h"
#include "scumm/smush/chunk.h"

namespace Scumm {

SaudChannel::SaudChannel(int32 track) : SmushChannel(track),
	_nbframes(0),
	_markReached(false),
	_index(0),
	_keepSize(false) {
}

bool SaudChannel::isTerminated() const {
	return (_markReached && _dataSize == 0 && _sbuffer == 0);
}

bool SaudChannel::handleSubTags(int32 &offset) {
	if (_tbufferSize - offset >= 8) {
		Chunk::type type = READ_BE_UINT32(_tbuffer + offset);
		uint32 size = READ_BE_UINT32(_tbuffer + offset + 4);
		uint32 available_size = _tbufferSize - offset;

		switch (type) {
		case MKID_BE('STRK'):
			_inData = false;
			if (available_size >= (size + 8)) {
				MemoryChunk c((byte *)_tbuffer + offset);
				if (c.size() != 14 && c.size() != 10) {
					error("STRK has an invalid size : %d", c.size());
				}
			} else
				return false;
			break;
		case MKID_BE('SMRK'):
			_inData = false;
			if (available_size >= (size + 8)) {
				MemoryChunk c((byte *)_tbuffer + offset);
				_markReached = true;
			} else
				return false;
			break;
		case MKID_BE('SHDR'):
			_inData = false;
			if (available_size >= (size + 8)) {
				MemoryChunk c((byte *)_tbuffer + offset);
				if (c.size() != 4)
					error("SHDR has an invalid size : %d", c.size());
			} else
				return false;
			break;
		case MKID_BE('SDAT'):
			_inData = true;
			_dataSize = size;
			offset += 8;
			return false;
		default:
			error("unknown Chunk in SAUD track : %s ", tag2str(type));
		}
		offset += size + 8;
		return true;
	}
	return false;
}

bool SaudChannel::setParameters(int32 nb, int32 flags, int32 volume, int32 pan, int32 index) {
	_nbframes = nb;
	_flags = flags; // bit 7 == IS_VOICE, bit 6 == IS_BACKGROUND_MUSIC, other ??
	_volume = volume;
	_pan = pan;
	_index = index;
	if (index != 0) {
		_dataSize = -2;
		_keepSize = true;
		_inData = true;
	}
	return true;
}

bool SaudChannel::checkParameters(int32 index, int32 nb, int32 flags, int32 volume, int32 pan) {
	if (++_index != index)
		error("invalid index in SaudChannel::checkParameters()");
	if (_nbframes != nb)
		error("invalid duration in SaudChannel::checkParameters()");
	if (_flags != flags)
		error("invalid flags in SaudChannel::checkParameters()");
	if (_volume != volume || _pan != pan) {
		_volume = volume;
		_pan = pan;
	}
	return true;
}

bool SaudChannel::appendData(Chunk &b, int32 size) {
	if (_dataSize == -1) {
		assert(size > 8);
		Chunk::type saud_type = b.readUint32BE();
		/*uint32 saud_size =*/ b.readUint32BE();
		if (saud_type != MKID_BE('SAUD'))
			error("Invalid Chunk for SaudChannel : %X", saud_type);
		size -= 8;
		_dataSize = -2;
	}
	if (_tbuffer) {
		byte *old = _tbuffer;
		_tbuffer = new byte[_tbufferSize + size];
		if (!_tbuffer)
			error("saud_channel failed to allocate memory");
		memcpy(_tbuffer, old, _tbufferSize);
		delete []old;
		b.read(_tbuffer + _tbufferSize, size);
		_tbufferSize += size;
	} else {
		_tbufferSize = size;
		_tbuffer = new byte[_tbufferSize];
		if (!_tbuffer)
			error("saud_channel failed to allocate memory");
		b.read(_tbuffer, _tbufferSize);
	}

	if (_keepSize) {
		_sbufferSize = _tbufferSize;
		_sbuffer = _tbuffer;
		_tbufferSize = 0;
		_tbuffer = 0;
	} else {
		processBuffer();
	}

	return true;
}

byte *SaudChannel::getSoundData() {
	byte *tmp = _sbuffer;

	if (!_keepSize) {
		assert(_dataSize > 0);
		_dataSize -= _sbufferSize;
	}

	_sbuffer = 0;
	_sbufferSize = 0;
	
	return tmp;
}

} // End of namespace Scumm
