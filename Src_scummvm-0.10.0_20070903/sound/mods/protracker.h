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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/sound/mods/protracker.h $
 * $Id: protracker.h 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

#ifndef SOUND_MODS_PROTRACKER_H
#define SOUND_MODS_PROTRACKER_H

#include "common/stdafx.h"
#include "common/stream.h"

namespace Audio {

class AudioStream;

/*
 * Factory function for ProTracker streams. Reads all data from the
 * given ReadStream and creates an AudioStream from this. No reference
 * to the 'stream' object is kept, so you can safely delete it after 
 * invoking this factory.
 * 
 * @param stream	the ReadStream from which to read the ProTracker data
 * @param rate		TODO
 * @param stereo	TODO
 * @return	a new AudioStream, or NULL, if an error occured
 */
AudioStream *makeProtrackerStream(Common::ReadStream *stream, int rate = 44100, bool stereo = true);

} // End of namespace Audio

#endif
