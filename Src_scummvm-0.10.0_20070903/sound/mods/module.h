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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/sound/mods/module.h $
 * $Id: module.h 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

#ifndef SOUND_MODS_MODULE_H
#define SOUND_MODS_MODULE_H

#include "common/stream.h"

namespace Modules {

#include "common/pack-start.h"	// START STRUCT PACKING

struct note_t {
	byte sample;
	byte note;
	uint16 period;
	uint16 effect;
};

#include "common/pack-end.h"	// END STRUCT PACKING

typedef note_t pattern_t[64][4];

struct sample_t {
	byte name[23];
	uint16 len;
	byte finetune;
	byte vol;
	uint16 repeat;
	uint16 replen;
	int8 *data;
};

class Module {
public:
	byte songname[21];

	static const int NUM_SAMPLES = 31;
	sample_t sample[NUM_SAMPLES];

	byte songlen;
	byte undef;
	byte songpos[128];
	byte sig[4];
	pattern_t *pattern;

	Module();
	~Module();

	bool load(Common::ReadStream &stream);
	static byte periodToNote(int16 period, byte finetune = 0);
	static int16 noteToPeriod(byte note, byte finetune = 0);

private:
	static const int16 periods[16][60];
};

} // End of namespace Modules

#endif
