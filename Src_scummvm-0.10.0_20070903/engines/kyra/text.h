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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/engines/kyra/text.h $
 * $Id: text.h 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

#ifndef KYRA_TEXT_H
#define KYRA_TEXT_H

namespace Kyra {
class Screen;
class KyraEngine;
class FontId;

class TextDisplayer {
	struct TalkCoords {
		uint16 y, x, w;
	};

	enum {
		TALK_SUBSTRING_LEN = 80,
		TALK_SUBSTRING_NUM = 3
	};
public:
	TextDisplayer(KyraEngine *vm, Screen *screen);
	~TextDisplayer() {}

	void setTalkCoords(uint16 y);
	int getCenterStringX(const char *str, int x1, int x2);
	int getCharLength(const char *str, int len);
	int dropCRIntoString(char *str, int offs);
	char *preprocessString(const char *str);
	int buildMessageSubstrings(const char *str);
	int getWidestLineWidth(int linesCount);
	void calcWidestLineBounds(int &x1, int &x2, int w, int cx);
	void restoreTalkTextMessageBkgd(int srcPage, int dstPage);
	void printTalkTextMessage(const char *text, int x, int y, uint8 color, int srcPage, int dstPage);
	void printIntroTextMessage(const char *text, int x, int y, uint8 col1, uint8 col2, uint8 col3,
			int dstPage, Screen::FontId font=Screen::FID_8_FNT);	
	void printText(const char *str, int x, int y, uint8 c0, uint8 c1, uint8 c2, Screen::FontId font=Screen::FID_8_FNT);
	void printCharacterText(const char *text, int8 charNum, int charX);

	uint16 _talkMessageY;
	uint16 _talkMessageH;
	bool printed() const { return _talkMessagePrinted; }
private:
	Screen *_screen;
	KyraEngine *_vm;

	char _talkBuffer[300];
	char _talkSubstrings[TALK_SUBSTRING_LEN * TALK_SUBSTRING_NUM];
	TalkCoords _talkCoords;
	bool _talkMessagePrinted;
};
} // end of namespace Kyra

#endif

