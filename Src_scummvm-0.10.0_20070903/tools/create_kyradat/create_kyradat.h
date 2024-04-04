/* ScummVM Tools
 * Copyright (C) 2007 The ScummVM project
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
 * $URL$
 * $Id$
 *
 */

#ifndef CREATE_KYRADAT_H
#define CREATE_KYRADAT_H

#include "util.h"
#include "pak.h"

struct Language {
	int lang;
	const char *ext;
};

enum kExtractID {
	kForestSeq = 1,
	kKallakWritingSeq,
	kKyrandiaLogoSeq,
	kKallakMalcolmSeq,
	kMalcolmTreeSeq,
	kWestwoodLogoSeq,

	kDemo1Seq,
	kDemo2Seq,
	kDemo3Seq,
	kDemo4Seq,

	kAmuleteAnimSeq,

	kOutroReunionSeq,

	kIntroCPSStrings,
	kIntroCOLStrings,
	kIntroWSAStrings,
	kIntroStrings,

	kOutroHomeString,

	kRoomFilenames,
	kRoomList,

	kCharacterImageFilenames,
	
	kItemNames,
	kTakenStrings,
	kPlacedStrings,
	kDroppedStrings,
	kNoDropStrings,

	kPutDownString,
	kWaitAmuletString,
	kBlackJewelString,
	kPoisonGoneString,
	kHealingTipString,
	kWispJewelStrings,
	kMagicJewelStrings,

	kThePoisonStrings,
	kFluteStrings,

	kFlaskFullString,
	kFullFlaskString,

	kVeryCleverString,
	kNewGameString,
	
	kDefaultShapes,
	kHealing1Shapes,
	kHealing2Shapes,
	kPoisonDeathShapes,
	kFluteShapes,
	kWinter1Shapes,
	kWinter2Shapes,
	kWinter3Shapes,
	kDrinkShapes,
	kWispShapes,
	kMagicAnimShapes,
	kBranStoneShapes,

	kPaletteList1,
	kPaletteList2,
	kPaletteList3,
	kPaletteList4,
	kPaletteList5,
	kPaletteList6,
	kPaletteList7,
	kPaletteList8,
	kPaletteList9,
	kPaletteList10,
	kPaletteList11,
	kPaletteList12,
	kPaletteList13,
	kPaletteList14,
	kPaletteList15,
	kPaletteList16,
	kPaletteList17,
	kPaletteList18,
	kPaletteList19,
	kPaletteList20,
	kPaletteList21,
	kPaletteList22,
	kPaletteList23,
	kPaletteList24,
	kPaletteList25,
	kPaletteList26,
	kPaletteList27,
	kPaletteList28,
	kPaletteList29,
	kPaletteList30,
	kPaletteList31,
	kPaletteList32,
	kPaletteList33,

	kGUIStrings,
	kConfigStrings,
	
	kKyra1TownsSFXTable,
	kCreditsStrings,
	kSjisVTable,
	kMenuSKB,
	
	kMaxResIDs
};

struct ExtractEntry {
	int id;
	uint32 startOff;
	uint32 endOff;
};

struct ExtractFilename {
	int id;
	int type;
	const char *filename;
};

enum kSpecial {
	kTalkieVersion = 0,
	kDemoVersion = 1,
	kFMTownsVersionE = 2,
	kFMTownsVersionJ = 3,
	kAmigaVersion = 4
};

struct SpecialExtension {
	int special;
	const char *ext;
};

enum kGame {
	kKyra1 = 0,
	kKyra2,
	kKyra3
};

struct Game {
	int game;
	int lang;
	int special;

	const char *md5;
	const ExtractEntry *entries;
};

#define GAME_DUMMY_ENTRY { -1, -1, -1, 0, 0 }

struct GameNeed {
	int game;
	int special;

	const int *entries;
};

enum kExtractType {
	kTypeLanguageList = 0,
	kTypeStringList,
	kTypeRoomList,
	kTypeShapeList,
	kTypeRawData
};

struct ExtractType {
	int type;
	bool (*extract)(PAKFile &out, const Game *g, const byte *data, const uint32 size, const char *filename, int fmtPatch);
	void (*createFilename)(char *dstFilename, const int lang, const int special, const char *filename);
};

#endif
