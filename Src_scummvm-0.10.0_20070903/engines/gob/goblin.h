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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/engines/gob/goblin.h $
 * $Id: goblin.h 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

#ifndef GOB_GOBLIN_H
#define GOB_GOBLIN_H

#include "gob/util.h"
#include "gob/sound.h"
#include "gob/mult.h"

namespace Gob {

#define TYPE_USUAL		0
#define TYPE_AMORPHOUS	1
#define TYPE_MOBILE		3

class Goblin {

public:

#include "common/pack-start.h"	// START STRUCT PACKING

	struct Gob_State {
		int16 animation;
		int16 layer;
		int16 unk0;
		int16 unk1;
		int16 sndItem;  // high/low byte - sound sample index
		int16 freq;     // high/low byte * 100 - frequency
		int16 repCount;	// high/low byte - repeat count
		int16 sndFrame;
	};

	typedef Gob_State *Gob_PState;

	typedef Gob_PState Gob_StateLine[6];

	struct Gob_Object {
		int16 animation;
		int16 state;
		int16 stateColumn;
		int16 curFrame;
		int16 xPos;
		int16 yPos;
		int16 dirtyLeft;
		int16 dirtyTop;
		int16 dirtyRight;
		int16 dirtyBottom;
		int16 left;
		int16 top;
		int16 right;
		int16 bottom;
		int16 nextState;
		int16 multState;
		int16 actionStartState;
		int16 curLookDir;
		int16 pickable;
		int16 relaxTime;
		Gob_StateLine *stateMach;
		Gob_StateLine *realStateMach;
		char doAnim;
		int8 order;
		char noTick;
		char toRedraw;
		char type;
		char maxTick;
		char tick;
		char multObjIndex;
		char unk14;
		char visible;
	};

	struct Gob_Pos {
		char x;
		char y;
	};

#include "common/pack-end.h"	// END STRUCT PACKING

	Gob_Object *_goblins[4];
	int16 _currentGoblin;
	SoundDesc _soundData[16];
	int16 _gobStateLayer;
	char _goesAtTarget;
	char _readyToAct;
	int16 _gobAction;	// 0 - move, 3 - do action, 4 - pick
	// goblins: 0 - picker, 1 - fighter, 2 - mage
	Gob_Pos _gobPositions[3];
	int16 _gobDestX;
	int16 _gobDestY;
	int16 _pressedMapX;
	int16 _pressedMapY;
	char _pathExistence;

	// Pointers to interpreter variables
	int32 *_some0ValPtr;

	int32 *_gobRetVarPtr;
	int32 *_curGobVarPtr;
	int32 *_curGobXPosVarPtr;
	int32 *_curGobYPosVarPtr;
	int32 *_itemInPocketVarPtr;

	int32 *_curGobStateVarPtr;
	int32 *_curGobFrameVarPtr;
	int32 *_curGobMultStateVarPtr;
	int32 *_curGobNextStateVarPtr;
	int32 *_curGobScrXVarPtr;
	int32 *_curGobScrYVarPtr;
	int32 *_curGobLeftVarPtr;
	int32 *_curGobTopVarPtr;
	int32 *_curGobRightVarPtr;
	int32 *_curGobBottomVarPtr;
	int32 *_curGobDoAnimVarPtr;
	int32 *_curGobOrderVarPtr;
	int32 *_curGobNoTickVarPtr;
	int32 *_curGobTypeVarPtr;
	int32 *_curGobMaxTickVarPtr;
	int32 *_curGobTickVarPtr;
	int32 *_curGobActStartStateVarPtr;
	int32 *_curGobLookDirVarPtr;
	int32 *_curGobPickableVarPtr;
	int32 *_curGobRelaxVarPtr;
	int32 *_curGobMaxFrameVarPtr;

	int32 *_destItemStateVarPtr;
	int32 *_destItemFrameVarPtr;
	int32 *_destItemMultStateVarPtr;
	int32 *_destItemNextStateVarPtr;
	int32 *_destItemScrXVarPtr;
	int32 *_destItemScrYVarPtr;
	int32 *_destItemLeftVarPtr;
	int32 *_destItemTopVarPtr;
	int32 *_destItemRightVarPtr;
	int32 *_destItemBottomVarPtr;
	int32 *_destItemDoAnimVarPtr;
	int32 *_destItemOrderVarPtr;
	int32 *_destItemNoTickVarPtr;
	int32 *_destItemTypeVarPtr;
	int32 *_destItemMaxTickVarPtr;
	int32 *_destItemTickVarPtr;
	int32 *_destItemActStartStVarPtr;
	int32 *_destItemLookDirVarPtr;
	int32 *_destItemPickableVarPtr;
	int32 *_destItemRelaxVarPtr;
	int32 *_destItemMaxFrameVarPtr;

	int16 _destItemType;
	int16 _destItemState;
	int16 _itemToObject[20];
	Gob_Object *_objects[20];
	int16 _objCount;
	int16 _gobsCount;
	int16 _itemIndInPocket;
	int16 _itemIdInPocket;
	char _itemByteFlag;
	int16 _destItemId;
	int16 _destActionItem;
	Gob_Object *_actDestItemDesc;
	int16 _forceNextState[10];
	char _boreCounter;
	int16 _positionedGob;
	char _noPick;

	// Gob2:
	int16 _soundSlotsCount;
	int16 _soundSlots[60];
	bool _gob1Busy;
	bool _gob2Busy;
	int16 _gob1RelaxTimeVar;
	int16 _gob2RelaxTimeVar;
	bool _gob1NoTurn;
	bool _gob2NoTurn;

	// Functions
	char rotateState(int16 from, int16 to);
	void playSound(SoundDesc &snd, int16 repCount, int16 freq);
	void drawObjects(void);
	void animateObjects(void);
	int16 getObjMaxFrame(Gob_Object * obj);
	bool objIntersected(Gob_Object * obj1, Gob_Object * obj2);
	void setMultStates(Gob_Object * gobDesc);
	int16 nextLayer(Gob_Object * gobDesc);
	void showBoredom(int16 gobIndex);
	void switchGoblin(int16 index);
	void zeroObjects(void);
	void freeAllObjects(void);
	void loadObjects(const char *source);
	void initVarPointers(void);
	void saveGobDataToVars(int16 xPos, int16 yPos, int16 someVal);
	void loadGobDataFromVars(void);
	void pickItem(int16 indexToPocket, int16 idToPocket);
	void placeItem(int16 indexInPocket, int16 idInPocket);
	void swapItems(int16 indexToPick, int16 idToPick);
	void treatItemPick(int16 itemId);
	int16 treatItem(int16 action);
	int16 doMove(Gob_Object *gobDesc, int16 cont, int16 action);

	void setState(int16 index, int16 state);
	void updateLayer1(Mult::Mult_AnimData *animData);
	void updateLayer2(Mult::Mult_AnimData *animData);
	void move(int16 destX, int16 destY, int16 objIndex);
	void animate(Mult::Mult_Object *obj);

	virtual void handleGoblins(void) = 0;
	virtual void placeObject(Gob_Object * objDesc, char animated,
			int16 index, int16 x, int16 y, int16 state) = 0;
	virtual void freeObjects(void) = 0;
	virtual void initiateMove(Mult::Mult_Object *obj) = 0;
	virtual void moveAdvance(Mult::Mult_Object *obj, Gob_Object *gobDesc,
			int16 nextAct, int16 framesCount) = 0;

	Goblin(GobEngine *vm);
	virtual ~Goblin();

protected:
	Util::List *_objList;
	int16 _rotStates[4][4];

	GobEngine *_vm;

	int16 peekGoblin(Gob_Object *curGob);
	void initList(void);
	void sortByOrder(Util::List *list);
	void adjustDest(int16 posX, int16 posY);
	void adjustTarget(void);
	void targetDummyItem(Gob_Object *gobDesc);
	void targetItem(void);
	void moveFindItem(int16 posX, int16 posY);
	void moveCheckSelect(int16 framesCount, Gob_Object * gobDesc,
			int16 *pGobIndex, int16 *nextAct);
	void moveInitStep(int16 framesCount, int16 action, int16 cont,
					  Gob_Object *gobDesc, int16 *pGobIndex, int16 *pNextAct);
	void moveTreatRopeStairs(Gob_Object *gobDesc);
	void playSounds(Mult::Mult_Object *obj);

	virtual bool isMovement(int8 state) = 0;
	virtual void advMovement(Mult::Mult_Object *obj, int8 state) = 0;
	virtual void movePathFind(Mult::Mult_Object *obj,
			Gob_Object *gobDesc, int16 nextAct) = 0;
};

class Goblin_v1 : public Goblin {
public:
	virtual void handleGoblins(void) {}
	virtual void placeObject(Gob_Object * objDesc, char animated,
			int16 index, int16 x, int16 y, int16 state);
	virtual void freeObjects(void);
	virtual void initiateMove(Mult::Mult_Object *obj);
	virtual void moveAdvance(Mult::Mult_Object *obj, Gob_Object *gobDesc,
			int16 nextAct, int16 framesCount);

	Goblin_v1(GobEngine *vm);
	virtual ~Goblin_v1() {}

protected:
	virtual bool isMovement(int8 state) { return false; }
	virtual void advMovement(Mult::Mult_Object *obj, int8 state) {}
	virtual void movePathFind(Mult::Mult_Object *obj,
			Gob_Object *gobDesc, int16 nextAct);
};

class Goblin_v2 : public Goblin_v1 {
public:
	virtual void handleGoblins(void);
	virtual void placeObject(Gob_Object * objDesc, char animated,
			int16 index, int16 x, int16 y, int16 state);
	virtual void freeObjects(void);
	virtual void initiateMove(Mult::Mult_Object *obj);
	virtual void moveAdvance(Mult::Mult_Object *obj, Gob_Object *gobDesc,
			int16 nextAct, int16 framesCount);

	Goblin_v2(GobEngine *vm);
	virtual ~Goblin_v2() {}

protected:
	virtual bool isMovement(int8 state);
	virtual void advMovement(Mult::Mult_Object *obj, int8 state);
	virtual void movePathFind(Mult::Mult_Object *obj,
			Gob_Object *gobDesc, int16 nextAct);
};

class Goblin_v3 : public Goblin_v2 {
public:
	virtual void placeObject(Gob_Object * objDesc, char animated,
			int16 index, int16 x, int16 y, int16 state);

	Goblin_v3(GobEngine *vm);
	virtual ~Goblin_v3() {}

protected:
	virtual bool isMovement(int8 state);
	virtual void advMovement(Mult::Mult_Object *obj, int8 state);
};

} // End of namespace Gob

#endif // GOB_GOBLIN_H
