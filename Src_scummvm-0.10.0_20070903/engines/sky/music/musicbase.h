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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/engines/sky/music/musicbase.h $
 * $Id: musicbase.h 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

#ifndef SKY_MUSIC_MUSICBASE_H
#define SKY_MUSIC_MUSICBASE_H

#include "common/stdafx.h"
#include "common/scummsys.h"
#include "common/mutex.h"

namespace Sky {

class Disk;

#define FILES_PER_SECTION 4

typedef struct {
	uint8 musicToProcess;
} Actions;

class ChannelBase {
public:
	virtual ~ChannelBase() {}
	virtual uint8 process(uint16 aktTime) = 0;
	virtual void updateVolume(uint16 pVolume) = 0;
	virtual bool isActive(void) = 0;
private:
};

class MusicBase {
public:
	MusicBase(Disk *pDisk);
	virtual ~MusicBase(void);
	void loadSection(uint8 pSection);
	void startMusic(uint16 param);
	void stopMusic();
	bool musicIsPlaying(void);
	uint8 giveVolume(void);
	uint8 giveCurrentMusic(void);
	virtual void setVolume(uint16 param) = 0;

protected:

	Disk *_skyDisk;
	uint8 *_musicData;

	uint16 _musicDataLoc;
	uint16 _driverFileBase;

	uint16 _musicVolume, _numberOfChannels;
	uint8 _currentMusic, _currentSection;
	uint8 _musicTempo0; // can be changed by music stream
	uint8 _musicTempo1; // given once per music
	uint32 _tempo;      // calculated from musicTempo0 and musicTempo1
	uint32 _aktTime;
	Actions _onNextPoll;
	ChannelBase *_channels[10];
	Common::Mutex _mutex;

	virtual void setupPointers(void) = 0;
	virtual void setupChannels(uint8 *channelData) = 0;
	virtual void startDriver(void) = 0;

	void updateTempo(void);
	void loadNewMusic(void);
	void pollMusic(void);

	void stopMusicInternal(void);
};

} // End of namespace Sky

#endif //MUSICBASE_H
