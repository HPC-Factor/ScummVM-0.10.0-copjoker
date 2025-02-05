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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/engines/saga/sndres.h $
 * $Id: sndres.h 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

// Sound resource class header

#ifndef SAGA_SNDRES_H
#define SAGA_SNDRES_H

#include "saga/itedata.h"
#include "saga/sound.h"

namespace Saga {

class SndRes {
public:

	SndRes(SagaEngine *vm);
	~SndRes();

	int loadSound(uint32 resourceId);
	void playSound(uint32 resourceId, int volume, bool loop);
	void playVoice(uint32 resourceId);
	int getVoiceLength(uint32 resourceId);
	void setVoiceBank(int serial);

	FxTable *_fxTable;
	int _fxTableLen;

	int16 *_fxTableIDs;
	int _fxTableIDsLen;

 private:
	bool load(ResourceContext *context, uint32 resourceId, SoundBuffer &buffer, bool onlyHeader);
	bool loadVocSound(byte *soundResource, size_t soundResourceLength, SoundBuffer &buffer);
	bool loadWavSound(byte *soundResource, size_t soundResourceLength, SoundBuffer &buffer);

	ResourceContext *_sfxContext;
	ResourceContext *_voiceContext;

	int _voiceSerial; // voice bank number

	SagaEngine *_vm;
};

} // End of namespace Saga

#endif
