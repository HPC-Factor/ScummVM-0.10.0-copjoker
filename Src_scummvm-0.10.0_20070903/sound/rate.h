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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/sound/rate.h $
 * $Id: rate.h 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

#ifndef SOUND_RATE_H
#define SOUND_RATE_H

#include "common/scummsys.h"
#include "engines/engine.h"

class AudioStream;


namespace Audio {

typedef int16 st_sample_t;
typedef uint16 st_volume_t;
typedef uint32 st_size_t;
typedef uint32 st_rate_t;

/* Minimum and maximum values a sample can hold. */
#define ST_SAMPLE_MAX 0x7fffL
#define ST_SAMPLE_MIN (-ST_SAMPLE_MAX - 1L)

#define ST_EOF (-1)
#define ST_SUCCESS (0)

static inline void clampedAdd(int16& a, int b) {
	register int val;
#ifdef OUTPUT_UNSIGNED_AUDIO
	val = (a ^ 0x8000) + b;
#else
	val = a + b;
#endif

	if (val > ST_SAMPLE_MAX)
		val = ST_SAMPLE_MAX;
	else if (val < ST_SAMPLE_MIN)
		val = ST_SAMPLE_MIN;

#ifdef OUTPUT_UNSIGNED_AUDIO
	a = ((int16)val) ^ 0x8000;
#else
	a = val;
#endif
}

// Q&D hack to get this SOX stuff to work
#define st_report warning
#define st_warn warning
#define st_fail error


class RateConverter {
public:
	RateConverter() {}
	virtual ~RateConverter() {}
	virtual int flow(AudioStream &input, st_sample_t *obuf, st_size_t osamp, st_volume_t vol_l, st_volume_t vol_r) = 0;
	virtual int drain(st_sample_t *obuf, st_size_t osamp, st_volume_t vol) = 0;
};

RateConverter *makeRateConverter(st_rate_t inrate, st_rate_t outrate, bool stereo, bool reverseStereo = false);

} // End of namespace Audio

#endif
