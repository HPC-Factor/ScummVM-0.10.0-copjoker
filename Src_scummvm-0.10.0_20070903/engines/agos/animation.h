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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/engines/agos/animation.h $
 * $Id: animation.h 27372 2007-06-12 06:47:40Z Kirben $
 *
 */

#ifndef AGOS_ANIMATION_H
#define AGOS_ANIMATION_H

#include "common/file.h"
#include "common/stream.h"

#include "graphics/dxa_player.h"
#include "sound/mixer.h"

namespace AGOS {

class AGOSEngine;

class MoviePlayer : public Graphics::DXAPlayer {
	AGOSEngine *_vm;

	Audio::Mixer *_mixer;

	Audio::SoundHandle _bgSound;
	Audio::AudioStream *_bgSoundStream;

	Audio::SoundHandle _omniTVSound;
	Common::SeekableReadStream *_omniTVFile;

	bool _omniTV;
	bool _leftButtonDown;
	bool _rightButtonDown;
	uint32 _ticks;
	
	char baseName[40];
	static const char *_sequenceList[90];
	uint8 _sequenceNum;
public:
	MoviePlayer(AGOSEngine *vm, Audio::Mixer *mixer);

	bool load(const char *filename);
	void play();
	void nextFrame();
protected:
	virtual void setPalette(byte *pal);
private:
	void playOmniTV();

	void handleNextFrame();
	bool processFrame();
	void startSound();
};

} // End of namespace AGOS

#endif
