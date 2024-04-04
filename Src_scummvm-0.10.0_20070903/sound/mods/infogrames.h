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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/sound/mods/infogrames.h $
 * $Id: infogrames.h 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

#ifndef SOUND_MODS_INFOGRAMES_H
#define SOUND_MODS_INFOGRAMES_H

#include "sound/mods/paula.h"
#include "common/stream.h"
#include "common/file.h"

namespace Audio {

/** A player for the Infogrames/RobHubbard2 format */
class Infogrames : public Paula {
public:
	class Instruments {
	public:
		Instruments();
		template<typename T> Instruments(T ins) {
			init();
			bool result = load(ins);
			assert(result);
		}
		~Instruments();

		bool load(Common::SeekableReadStream &ins);
		bool load(const char *ins) {
			Common::File f;

			if (f.open(ins))
				return load(f);
			return false;
		}
		void unload(void);

		uint8 getCount(void) const { return _count; }

	protected:
		struct Sample {
			int8 *data;
			int8 *dataRepeat;
			uint32 length;
			uint32 lengthRepeat;
		} _samples[32];

		uint8 _count;
		int8 *_sampleData;

		void init();

		friend class Infogrames;
	};

	Infogrames(Instruments &ins, bool stereo = false, int rate = 44100,
			int interruptFreq = 0);
	~Infogrames();

	Instruments *getInstruments(void) const { return _instruments; }
	bool getRepeating(void) const { return _repCount != 0; }
	void setRepeating (int32 repCount) { _repCount = repCount; }
	virtual void startPlay(void) { _playing = true;}
	virtual void stopPlay(void)
	{
		_mutex.lock();
		_playing = false;
		_mutex.unlock();
	}
	virtual void pausePlay(bool pause) { _playing = !pause; }

	bool load(Common::SeekableReadStream &dum);
	bool load(const char *dum) {
		Common::File f;

		if (f.open(dum))
			return load(f);
		return false;
	}
	void unload(void);
	void restart(void) { if (_data) { stopPlay(); init(); startPlay(); } }

protected:
	Instruments *_instruments;

	static const uint8 tickCount[];
	static const uint16 periods[];
	byte *_data;
	int32 _repCount;

	byte *_subSong;
	byte *_cmdBlocks;
	byte *_volSlideBlocks;
	byte *_periodSlideBlocks;
	uint8 _speedCounter;
	uint8 _speed;

	uint16 _volume;
	int16 _period;
	uint8 _sample;

	struct Slide {
		byte *data;
		int8 amount;
		uint8 dataOffset;
		int16 finetuneNeg;
		int16 finetunePos;
		uint8 curDelay1;
		uint8 curDelay2;
		uint8 flags; // 0: Apply finetune modifier, 2: Don't slide, 7: Continuous
	};
	struct Channel {
		byte *cmdBlockIndices;
		byte *cmdBlocks;
		byte *cmds;
		uint8 ticks;
		uint8 tickCount;
		Slide volSlide;
		Slide periodSlide;
		int16 period;
		int8 periodMod;
		uint8 flags; // 0: Need init, 5: Loop cmdBlocks, 6: Ignore channel
	} _chn[4];

	void init(void);
	void reset(void);
	void getNextSample(Channel &chn);
	int16 tune(Slide &slide, int16 start) const;
	virtual void interrupt(void);
};

} // End of namespace Audio

#endif
