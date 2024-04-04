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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/engines/sword1/credits.h $
 * $Id: credits.h 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

#ifndef SWORD1_CREDITS_H
#define SWORD1_CREDITS_H

#include "common/util.h"

namespace Audio {
	class Mixer;
}
class OSystem;

namespace Sword1 {

class ArcFile {
public:
	ArcFile(void);
	~ArcFile(void);
	bool open(const char *name);
	uint8 *fetchFile(uint32 fileId, uint32 *size = NULL);
	uint8 *decompressFile(uint32 fileId);
	void enterPath(uint32 id);
	void backToRoot(void) { _bufPos = _buf; }
private:
	uint8 *_bufPos;
	uint8 *_buf;
};

class CreditsPlayer {
public:
	CreditsPlayer(OSystem *pSystem, Audio::Mixer *pMixer);
	void play(void);
private:
	void generateFonts(ArcFile *arcFile);
	void renderLine(uint8 *screenBuf, uint8 *line, uint16 yBufPos, uint8 flags);
	void fadePalette(uint8 *srcPal, bool fadeup, uint16 len);
	void delay(int msecs);
	uint16 getWidth(uint8 *font, uint8 *line);
	uint8 getPalIdx(uint8 r, uint8 g, uint8 b);
	uint8 _palette[256 * 4];
	uint32 _palLen;
	uint8 _numChars;

	OSystem *_system;
	Audio::Mixer *_mixer;

	uint8 *_smlFont, *_bigFont;
};

} // end of namespace Sword1

#endif // BS1CREDITS_H
