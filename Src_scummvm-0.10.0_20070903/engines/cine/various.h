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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/engines/cine/various.h $
 * $Id: various.h 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

#ifndef CINE_VARIOUS_H
#define CINE_VARIOUS_H

#include "common/stdafx.h"
#include "common/file.h"

#include "cine/cine.h"

namespace Cine {

void initLanguage(Common::Language lang);

int16 makeMenuChoice(const commandeType commandList[], uint16 height, uint16 X, uint16 Y, uint16 width);
int16 makeMenuChoice2(const commandeType commandList[], uint16 height, uint16 X, uint16 Y, uint16 width);
void makeCommandLine(void);
void makeActionMenu(void);

extern bool disableSystemMenu;
extern bool inMenu; 

struct unk1Struct {
	byte *ptr;
	uint16 len;
};

#define NUM_MAX_MESSAGE 255

extern unk1Struct messageTable[NUM_MAX_MESSAGE];

struct SeqListElement {
	struct SeqListElement *next;
	int16 var4;
	int16 var6;
	int16 var8;
	int16 varA;
	int16 varC;
	int16 varE;
	int16 var10;
	int16 var12;
	int16 var14;
	int16 var16;
	int16 var18;
	int16 var1A;
	int16 var1C;
	int16 var1E;
};

extern SeqListElement seqList;

extern uint32 var6;
extern uint32 var8;

extern uint16 var2;
extern uint16 var3;
extern uint16 var4;
extern uint16 var5;

void mainLoopSub1(void);
void setTextWindow(uint16 param1, uint16 param2, uint16 param3, uint16 param4);

extern uint16 errorVar;
extern byte menuVar;

extern uint16 allowPlayerInput;

extern uint16 checkForPendingDataLoadSwitch;

extern uint16 fadeRequired;
extern uint16 isDrawCommandEnabled;
extern uint16 waitForPlayerClick;
extern uint16 menuCommandLen;
extern uint16 var17;
extern uint16 var18;
extern uint16 var19;
extern uint16 var20;
extern byte var21;

extern int16 playerCommand;

extern char commandBuffer[80];

extern char currentPrcName[20];
extern char currentRelName[20];
extern char currentObjectName[20];
extern char currentMsgName[20];
extern char newPrcName[20];
extern char newRelName[20];
extern char newObjectName[20];
extern char newMsgName[20];

extern char currentBgName[8][15];
extern char currentCtName[15];
extern char currentPartName[15];

void stopSample(void);
void mainLoopSub3(void);
uint16 executePlayerInput(void);

void drawOverlays(void);

extern uint16 mouseUpdateStatus;
extern uint16 dummyU16;

void getMouseData(uint16 param, uint16 *pButton, uint16 *pX, uint16 *pY);
int getKeyData(void);

uint16 processKeyboard(uint16 param);

void mainLoopSub6(void);

void checkForPendingDataLoad(void);

extern uint16 exitEngine;

void hideMouse(void);

void removeExtention(char *dest, const char *source);

struct selectedObjStruct {
	int16 idx;
	int16 param;
};

extern uint16 defaultMenuBoxColor;
extern uint16 defaultMenuBoxColor2;

#define NUM_MAX_ZONE 16
extern uint16 zoneData[NUM_MAX_ZONE];

void addMessage(byte param1, int16 param2, int16 param3, int16 param4, int16 param5);

extern int16 additionalBgVScroll;

void addSeqListElement(int16 param0, int16 param1, int16 param2, int16 param3, int16 param4, int16 param5, int16 param6, int16 param7, int16 param8);
void resetSeqList();
void processSeqList(void);

bool makeTextEntryMenu(const char *caption, char *string, int strLen, int y);

} // End of namespace Cine

#endif
