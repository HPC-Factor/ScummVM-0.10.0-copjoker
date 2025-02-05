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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/engines/scumm/imuse/instrument.h $
 * $Id: instrument.h 27024 2007-05-30 21:56:52Z fingolfin $
 */

#ifndef SCUMM_IMUSE_INSTRUMENT_H
#define SCUMM_IMUSE_INSTRUMENT_H

#include "common/stdafx.h"
#include "common/scummsys.h"

class MidiChannel;

namespace Scumm {

class Serializer;
class Instrument;

class InstrumentInternal {
public:
	virtual ~InstrumentInternal() {}
	virtual void saveOrLoad(Serializer *s) = 0;
	virtual void send(MidiChannel *mc) = 0;
	virtual void copy_to(Instrument *dest) = 0;
	virtual bool is_valid() = 0;
	virtual operator int() { return 255; }
};

class Instrument {
private:
	byte _type;
	InstrumentInternal *_instrument;

public:
	enum {
		itNone = 0,
		itProgram = 1,
		itAdlib = 2,
		itRoland = 3
	};

	Instrument() : _type (0), _instrument (0) { }
	~Instrument() { delete _instrument; }
	static void nativeMT32(bool native);
	static const byte _gmRhythmMap[35];

	void clear();
	void copy_to(Instrument *dest) {
		if (_instrument)
			_instrument->copy_to(dest);
		else
			dest->clear();
	}

	void program(byte program, bool mt32);
	void adlib(const byte *instrument);
	void roland(const byte *instrument);

	byte getType() { return _type; }
	bool isValid() { return (_instrument ? _instrument->is_valid() : false); }
	void saveOrLoad(Serializer *s);
	void send(MidiChannel *mc) {
		if (_instrument)
			_instrument->send(mc);
	}
};

} // End of namespace Scumm

#endif
