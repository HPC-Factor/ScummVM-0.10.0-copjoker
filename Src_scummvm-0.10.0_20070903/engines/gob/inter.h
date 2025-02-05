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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/engines/gob/inter.h $
 * $Id: inter.h 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

#ifndef GOB_INTER_H
#define GOB_INTER_H

#include "gob/goblin.h"

namespace Gob {

// This is to help devices with small memory (PDA, smartphones, ...)
// to save abit of memory used by opcode names in the Scumm engine.
#ifndef REDUCE_MEMORY_USAGE
# define _OPCODE(ver, x)  { &ver::x, #x }
#else
# define _OPCODE(ver, x)  { &ver::x, "" }
#endif

class Inter {
public:
	uint8 _terminate;

	int16 *_breakFromLevel;
	int16 *_nestLevel;

	uint32 _soundEndTimeKey;
	int16 _soundStopVal;

	void initControlVars(char full);
	int16 load16();
	char evalExpr(int16 *pRes);
	bool evalBoolResult();
	void renewTimeInVars();
	void storeMouse();
	void storeKey(int16 key);

	void funcBlock(int16 retFlag);
	void callSub(int16 retFlag);

	virtual int16 loadSound(int16 slot) = 0;
	virtual void animPalette() = 0;

	Inter(GobEngine *vm);
	virtual ~Inter() {}

protected:
	struct OpFuncParams {
		byte cmdCount;
		byte counter;
		int16 retFlag;
	};
	struct OpGobParams {
		int16 extraData;
		int32 *retVarPtr;
		Goblin::Gob_Object *objDesc;
	};

	bool _break;

	int16 _animPalLowIndex[8];
	int16 _animPalHighIndex[8];
	int16 _animPalDir[8];

	char _pasteBuf[300];
	byte _pasteSizeBuf[300];
	int16 _pastePos;

	// The busy-wait detection in o1_keyFunc breaks fast scrolling in Ween
	bool _noBusyWait;

	GobEngine *_vm;

	virtual void setupOpcodes() = 0;
	virtual void executeDrawOpcode(byte i) = 0;
	virtual bool executeFuncOpcode(byte i, byte j, OpFuncParams &params) = 0;
	virtual void executeGoblinOpcode(int i, OpGobParams &params) = 0;
	virtual const char *getOpcodeDrawDesc(byte i) = 0;
	virtual const char *getOpcodeFuncDesc(byte i, byte j) = 0;
	virtual const char *getOpcodeGoblinDesc(int i) = 0;

	virtual void checkSwitchTable(byte **ppExec) = 0;

	void o_drawNOP() {}
	bool o_funcNOP(OpFuncParams &params) { return false; }
	void o_gobNOP(OpGobParams &params) {}
};

class Inter_v1 : public Inter {
public:
	Inter_v1(GobEngine *vm);
	virtual ~Inter_v1() {}

	virtual int16 loadSound(int16 slot);
	virtual void animPalette();

protected:
	typedef void (Inter_v1::*OpcodeDrawProcV1)();
	typedef bool (Inter_v1::*OpcodeFuncProcV1)(OpFuncParams &);
	typedef void (Inter_v1::*OpcodeGoblinProcV1)(OpGobParams &);
	struct OpcodeDrawEntryV1 {
		OpcodeDrawProcV1 proc;
		const char *desc;
	};
	struct OpcodeFuncEntryV1 {
		OpcodeFuncProcV1 proc;
		const char *desc;
	};
	struct OpcodeGoblinEntryV1 {
		OpcodeGoblinProcV1 proc;
		const char *desc;
	};
	const OpcodeDrawEntryV1 *_opcodesDrawV1;
	const OpcodeFuncEntryV1 *_opcodesFuncV1;
	const OpcodeGoblinEntryV1 *_opcodesGoblinV1;
	static const int _goblinFuncLookUp[][2];

	virtual void setupOpcodes();
	virtual void executeDrawOpcode(byte i);
	virtual bool executeFuncOpcode(byte i, byte j, OpFuncParams &params);
	virtual void executeGoblinOpcode(int i, OpGobParams &params);
	virtual const char *getOpcodeDrawDesc(byte i);
	virtual const char *getOpcodeFuncDesc(byte i, byte j);
	virtual const char *getOpcodeGoblinDesc(int i);

	virtual void checkSwitchTable(byte **ppExec);

	void o1_loadMult();
	void o1_playMult();
	void o1_freeMultKeys();
	void o1_initCursor();
	void o1_initCursorAnim();
	void o1_clearCursorAnim();
	void o1_setRenderFlags();
	void o1_loadAnim();
	void o1_freeAnim();
	void o1_updateAnim();
	void o1_initMult();
	void o1_freeMult();
	void o1_animate();
	void o1_loadMultObject();
	void o1_getAnimLayerInfo();
	void o1_getObjAnimSize();
	void o1_loadStatic();
	void o1_freeStatic();
	void o1_renderStatic();
	void o1_loadCurLayer();
	void o1_playCDTrack();
	void o1_getCDTrackPos();
	void o1_stopCD();
	void o1_loadFontToSprite();
	void o1_freeFontToSprite();
	bool o1_callSub(OpFuncParams &params);
	bool o1_printTotText(OpFuncParams &params);
	bool o1_loadCursor(OpFuncParams &params);
	bool o1_switch(OpFuncParams &params);
	bool o1_repeatUntil(OpFuncParams &params);
	bool o1_whileDo(OpFuncParams &params);
	bool o1_if(OpFuncParams &params);
	bool o1_evaluateStore(OpFuncParams &params);
	bool o1_loadSpriteToPos(OpFuncParams &params);
	bool o1_printText(OpFuncParams &params);
	bool o1_loadTot(OpFuncParams &params);
	bool o1_palLoad(OpFuncParams &params);
	bool o1_keyFunc(OpFuncParams &params);
	bool o1_capturePush(OpFuncParams &params);
	bool o1_capturePop(OpFuncParams &params);
	bool o1_animPalInit(OpFuncParams &params);
	bool o1_drawOperations(OpFuncParams &params);
	bool o1_setcmdCount(OpFuncParams &params);
	bool o1_return(OpFuncParams &params);
	bool o1_renewTimeInVars(OpFuncParams &params);
	bool o1_speakerOn(OpFuncParams &params);
	bool o1_speakerOff(OpFuncParams &params);
	bool o1_putPixel(OpFuncParams &params);
	bool o1_goblinFunc(OpFuncParams &params);
	bool o1_createSprite(OpFuncParams &params);
	bool o1_freeSprite(OpFuncParams &params);
	bool o1_returnTo(OpFuncParams &params);
	bool o1_loadSpriteContent(OpFuncParams &params);
	bool o1_copySprite(OpFuncParams &params);
	bool o1_fillRect(OpFuncParams &params);
	bool o1_drawLine(OpFuncParams &params);
	bool o1_strToLong(OpFuncParams &params);
	bool o1_invalidate(OpFuncParams &params);
	bool o1_setBackDelta(OpFuncParams &params);
	bool o1_playSound(OpFuncParams &params);
	bool o1_stopSound(OpFuncParams &params);
	bool o1_loadSound(OpFuncParams &params);
	bool o1_freeSoundSlot(OpFuncParams &params);
	bool o1_waitEndPlay(OpFuncParams &params);
	bool o1_playComposition(OpFuncParams &params);
	bool o1_getFreeMem(OpFuncParams &params);
	bool o1_checkData(OpFuncParams &params);
	bool o1_prepareStr(OpFuncParams &params);
	bool o1_insertStr(OpFuncParams &params);
	bool o1_cutStr(OpFuncParams &params);
	bool o1_strstr(OpFuncParams &params);
	bool o1_istrlen(OpFuncParams &params);
	bool o1_setMousePos(OpFuncParams &params);
	bool o1_setFrameRate(OpFuncParams &params);
	bool o1_animatePalette(OpFuncParams &params);
	bool o1_animateCursor(OpFuncParams &params);
	bool o1_blitCursor(OpFuncParams &params);
	bool o1_loadFont(OpFuncParams &params);
	bool o1_freeFont(OpFuncParams &params);
	bool o1_readData(OpFuncParams &params);
	bool o1_writeData(OpFuncParams &params);
	bool o1_manageDataFile(OpFuncParams &params);
	void o1_setState(OpGobParams &params);
	void o1_setCurFrame(OpGobParams &params);
	void o1_setNextState(OpGobParams &params);
	void o1_setMultState(OpGobParams &params);
	void o1_setOrder(OpGobParams &params);
	void o1_setActionStartState(OpGobParams &params);
	void o1_setCurLookDir(OpGobParams &params);
	void o1_setType(OpGobParams &params);
	void o1_setNoTick(OpGobParams &params);
	void o1_setPickable(OpGobParams &params);
	void o1_setXPos(OpGobParams &params);
	void o1_setYPos(OpGobParams &params);
	void o1_setDoAnim(OpGobParams &params);
	void o1_setRelaxTime(OpGobParams &params);
	void o1_setMaxTick(OpGobParams &params);
	void o1_getState(OpGobParams &params);
	void o1_getCurFrame(OpGobParams &params);
	void o1_getNextState(OpGobParams &params);
	void o1_getMultState(OpGobParams &params);
	void o1_getOrder(OpGobParams &params);
	void o1_getActionStartState(OpGobParams &params);
	void o1_getCurLookDir(OpGobParams &params);
	void o1_getType(OpGobParams &params);
	void o1_getNoTick(OpGobParams &params);
	void o1_getPickable(OpGobParams &params);
	void o1_getObjMaxFrame(OpGobParams &params);
	void o1_getXPos(OpGobParams &params);
	void o1_getYPos(OpGobParams &params);
	void o1_getDoAnim(OpGobParams &params);
	void o1_getRelaxTime(OpGobParams &params);
	void o1_getMaxTick(OpGobParams &params);
	void o1_manipulateMap(OpGobParams &params);
	void o1_getItem(OpGobParams &params);
	void o1_manipulateMapIndirect(OpGobParams &params);
	void o1_getItemIndirect(OpGobParams &params);
	void o1_setPassMap(OpGobParams &params);
	void o1_setGoblinPosH(OpGobParams &params);
	void o1_getGoblinPosXH(OpGobParams &params);
	void o1_getGoblinPosYH(OpGobParams &params);
	void o1_setGoblinMultState(OpGobParams &params);
	void o1_setGoblinUnk14(OpGobParams &params);
	void o1_setItemIdInPocket(OpGobParams &params);
	void o1_setItemIndInPocket(OpGobParams &params);
	void o1_getItemIdInPocket(OpGobParams &params);
	void o1_getItemIndInPocket(OpGobParams &params);
	void o1_setGoblinPos(OpGobParams &params);
	void o1_setGoblinState(OpGobParams &params);
	void o1_setGoblinStateRedraw(OpGobParams &params);
	void o1_decRelaxTime(OpGobParams &params);
	void o1_getGoblinPosX(OpGobParams &params);
	void o1_getGoblinPosY(OpGobParams &params);
	void o1_clearPathExistence(OpGobParams &params);
	void o1_setGoblinVisible(OpGobParams &params);
	void o1_setGoblinInvisible(OpGobParams &params);
	void o1_getObjectIntersect(OpGobParams &params);
	void o1_getGoblinIntersect(OpGobParams &params);
	void o1_setItemPos(OpGobParams &params);
	void o1_loadObjects(OpGobParams &params);
	void o1_freeObjects(OpGobParams &params);
	void o1_animateObjects(OpGobParams &params);
	void o1_drawObjects(OpGobParams &params);
	void o1_loadMap(OpGobParams &params);
	void o1_moveGoblin(OpGobParams &params);
	void o1_switchGoblin(OpGobParams &params);
	void o1_loadGoblin(OpGobParams &params);
	void o1_writeTreatItem(OpGobParams &params);
	void o1_moveGoblin0(OpGobParams &params);
	void o1_setGoblinTarget(OpGobParams &params);
	void o1_setGoblinObjectsPos(OpGobParams &params);
	void o1_initGoblin(OpGobParams &params);

	void manipulateMap(int16 xPos, int16 yPos, int16 item);
};

class Inter_v2 : public Inter_v1 {
public:
	Inter_v2(GobEngine *vm);
	virtual ~Inter_v2() {}

	virtual int16 loadSound(int16 search);
	virtual void animPalette();

protected:
	typedef void (Inter_v2::*OpcodeDrawProcV2)();
	typedef bool (Inter_v2::*OpcodeFuncProcV2)(OpFuncParams &);
	typedef void (Inter_v2::*OpcodeGoblinProcV2)(OpGobParams &);
	struct OpcodeDrawEntryV2 {
		OpcodeDrawProcV2 proc;
		const char *desc;
	};
	struct OpcodeFuncEntryV2 {
		OpcodeFuncProcV2 proc;
		const char *desc;
	};
	struct OpcodeGoblinEntryV2 {
		OpcodeGoblinProcV2 proc;
		const char *desc;
	};
	const OpcodeDrawEntryV2 *_opcodesDrawV2;
	const OpcodeFuncEntryV2 *_opcodesFuncV2;
	const OpcodeGoblinEntryV2 *_opcodesGoblinV2;
	static const int _goblinFuncLookUp[][2];

	virtual void setupOpcodes();
	virtual void executeDrawOpcode(byte i);
	virtual bool executeFuncOpcode(byte i, byte j, OpFuncParams &params);
	virtual void executeGoblinOpcode(int i, OpGobParams &params);
	virtual const char *getOpcodeDrawDesc(byte i);
	virtual const char *getOpcodeFuncDesc(byte i, byte j);
	virtual const char *getOpcodeGoblinDesc(int i);

	virtual void checkSwitchTable(byte **ppExec);

	void o2_playMult();
	void o2_setRenderFlags();
	void o2_multSub();
	void o2_initMult();
	void o2_loadMultObject();
	void o2_renderStatic();
	void o2_loadCurLayer();
	void o2_playCDTrack();
	void o2_waitCDTrackEnd();
	void o2_stopCD();
	void o2_readLIC();
	void o2_freeLIC();
	void o2_getCDTrackPos();
	void o2_loadFontToSprite();
	void o2_totSub();
	void o2_switchTotSub();
	void o2_copyVars();
	void o2_pasteVars();
	void o2_loadMapObjects();
	void o2_freeGoblins();
	void o2_moveGoblin();
	void o2_writeGoblinPos();
	void o2_stopGoblin();
	void o2_setGoblinState();
	void o2_placeGoblin();
	void o2_initScreen();
	void o2_scroll();
	void o2_setScrollOffset();
	void o2_playImd();
	void o2_getImdInfo();
	void o2_openItk();
	void o2_closeItk();
	void o2_setImdFrontSurf();
	void o2_resetImdFrontSurf();
	bool o2_evaluateStore(OpFuncParams &params);
	bool o2_printText(OpFuncParams &params);
	bool o2_animPalInit(OpFuncParams &params);
	bool o2_goblinFunc(OpFuncParams &params);
	bool o2_createSprite(OpFuncParams &params);
	bool o2_stopSound(OpFuncParams &params);
	bool o2_loadSound(OpFuncParams &params);
	bool o2_getFreeMem(OpFuncParams &params);
	bool o2_checkData(OpFuncParams &params);
	bool o2_readData(OpFuncParams &params);
	bool o2_writeData(OpFuncParams &params);
	void o2_loadInfogramesIns(OpGobParams &params);
	void o2_playInfogrames(OpGobParams &params);
	void o2_startInfogrames(OpGobParams &params);
	void o2_stopInfogrames(OpGobParams &params);
	void o2_handleGoblins(OpGobParams &params);
};

class Inter_Bargon : public Inter_v2 {
public:
	Inter_Bargon(GobEngine *vm);
	virtual ~Inter_Bargon() {}

protected:
	typedef void (Inter_Bargon::*OpcodeDrawProcBargon)();
	typedef bool (Inter_Bargon::*OpcodeFuncProcBargon)(OpFuncParams &);
	typedef void (Inter_Bargon::*OpcodeGoblinProcBargon)(OpGobParams &);
	struct OpcodeDrawEntryBargon {
		OpcodeDrawProcBargon proc;
		const char *desc;
	};
	struct OpcodeFuncEntryBargon {
		OpcodeFuncProcBargon proc;
		const char *desc;
	};
	struct OpcodeGoblinEntryBargon {
		OpcodeGoblinProcBargon proc;
		const char *desc;
	};
	const OpcodeDrawEntryBargon *_opcodesDrawBargon;
	const OpcodeFuncEntryBargon *_opcodesFuncBargon;
	const OpcodeGoblinEntryBargon *_opcodesGoblinBargon;
	static const int _goblinFuncLookUp[][2];

	virtual void setupOpcodes();
	virtual void executeDrawOpcode(byte i);
	virtual bool executeFuncOpcode(byte i, byte j, OpFuncParams &params);
	virtual void executeGoblinOpcode(int i, OpGobParams &params);
	virtual const char *getOpcodeDrawDesc(byte i);
	virtual const char *getOpcodeFuncDesc(byte i, byte j);
	virtual const char *getOpcodeGoblinDesc(int i);

	void oBargon_intro0(OpGobParams &params);
	void oBargon_intro1(OpGobParams &params);
	void oBargon_intro2(OpGobParams &params);
	void oBargon_intro3(OpGobParams &params);
	void oBargon_intro4(OpGobParams &params);
	void oBargon_intro5(OpGobParams &params);
	void oBargon_intro6(OpGobParams &params);
	void oBargon_intro7(OpGobParams &params);
	void oBargon_intro8(OpGobParams &params);
	void oBargon_intro9(OpGobParams &params);
};

class Inter_v3 : public Inter_v2 {
public:
	Inter_v3(GobEngine *vm);
	virtual ~Inter_v3() {}

protected:
	typedef void (Inter_v3::*OpcodeDrawProcV3)();
	typedef bool (Inter_v3::*OpcodeFuncProcV3)(OpFuncParams &);
	typedef void (Inter_v3::*OpcodeGoblinProcV3)(OpGobParams &);
	struct OpcodeDrawEntryV3 {
		OpcodeDrawProcV3 proc;
		const char *desc;
	};
	struct OpcodeFuncEntryV3 {
		OpcodeFuncProcV3 proc;
		const char *desc;
	};
	struct OpcodeGoblinEntryV3 {
		OpcodeGoblinProcV3 proc;
		const char *desc;
	};
	const OpcodeDrawEntryV3 *_opcodesDrawV3;
	const OpcodeFuncEntryV3 *_opcodesFuncV3;
	const OpcodeGoblinEntryV3 *_opcodesGoblinV3;
	static const int _goblinFuncLookUp[][2];

	virtual void setupOpcodes();
	virtual void executeDrawOpcode(byte i);
	virtual bool executeFuncOpcode(byte i, byte j, OpFuncParams &params);
	virtual void executeGoblinOpcode(int i, OpGobParams &params);
	virtual const char *getOpcodeDrawDesc(byte i);
	virtual const char *getOpcodeFuncDesc(byte i, byte j);
	virtual const char *getOpcodeGoblinDesc(int i);

	bool o3_getTotTextItemPart(OpFuncParams &params);
	bool o3_copySprite(OpFuncParams &params);
	bool o3_checkData(OpFuncParams &params);
	bool o3_readData(OpFuncParams &params);
	bool o3_writeData(OpFuncParams &params);
};

} // End of namespace Gob

#endif // GOB_INTER_H
