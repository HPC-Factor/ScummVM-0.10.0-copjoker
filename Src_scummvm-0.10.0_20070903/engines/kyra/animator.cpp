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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/engines/kyra/animator.cpp $
 * $Id: animator.cpp 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

#include "common/stdafx.h"
#include "common/endian.h"

#include "kyra/kyra.h"
#include "kyra/screen.h"
#include "kyra/animator.h"
#include "kyra/sprites.h"

#include "common/system.h"

namespace Kyra {
ScreenAnimator::ScreenAnimator(KyraEngine *vm, OSystem *system) {
	_vm = vm;
	_screen = vm->screen();
	_initOk = false;
	_updateScreen = false;
	_system = system;
	_screenObjects = _actors = _items = _sprites = _objectQueue = 0;
	_noDrawShapesFlag = 0;

	_actorBkgBackUp[0] = new uint8[_screen->getRectSize(8, 69)];
	memset(_actorBkgBackUp[0], 0, _screen->getRectSize(8, 69));
	_actorBkgBackUp[1] = new uint8[_screen->getRectSize(8, 69)];
	memset(_actorBkgBackUp[1], 0, _screen->getRectSize(8, 69));
}

ScreenAnimator::~ScreenAnimator() {
	close();
	delete [] _actorBkgBackUp[0];
	delete [] _actorBkgBackUp[1];
}

void ScreenAnimator::init(int actors_, int items_, int sprites_) {
	debugC(9, kDebugLevelAnimator, "ScreenAnimator::init(%d, %d, %d)", actors_, items_, sprites_);
	_screenObjects = new AnimObject[actors_ + items_ + sprites_];
	assert(_screenObjects);
	memset(_screenObjects, 0, sizeof(AnimObject) * (actors_ + items_ + sprites_));
	_actors = _screenObjects;
	_sprites = &_screenObjects[actors_];
	_items = &_screenObjects[actors_ + items_];
	_brandonDrawFrame = 113;

	_initOk = true;
}

void ScreenAnimator::close() {
	debugC(9, kDebugLevelAnimator, "ScreenAnimator::close()");
	if (_initOk) {
		_initOk = false;
		delete [] _screenObjects;
		_screenObjects = _actors = _items = _sprites = _objectQueue = 0;
	}
}

void ScreenAnimator::initAnimStateList() {
	AnimObject *animStates = _screenObjects;
	animStates[0].index = 0;
	animStates[0].active = 1;
	animStates[0].flags = 0x800;
	animStates[0].background = _actorBkgBackUp[0];
	animStates[0].rectSize = _screen->getRectSize(4, 48);
	animStates[0].width = 4;
	animStates[0].height = 48;
	animStates[0].width2 = 4;
	animStates[0].height2 = 3;
	
	for (int i = 1; i <= 4; ++i) {
		animStates[i].index = i;
		animStates[i].active = 0;
		animStates[i].flags = 0x800;
		animStates[i].background = _actorBkgBackUp[1];
		animStates[i].rectSize = _screen->getRectSize(4, 64);
		animStates[i].width = 4;
		animStates[i].height = 48;
		animStates[i].width2 = 4;
		animStates[i].height2 = 3;
	}
	
	for (int i = 5; i < 16; ++i) {
		animStates[i].index = i;
		animStates[i].active = 0;
		animStates[i].flags = 0;
	}
	
	for (int i = 16; i < 28; ++i) {
		animStates[i].index = i;
		animStates[i].flags = 0;
		animStates[i].background = _vm->_shapes[345+i];
		animStates[i].rectSize = _screen->getRectSize(3, 24);
		animStates[i].width = 3;
		animStates[i].height = 16;
		animStates[i].width2 = 0;
		animStates[i].height2 = 0;
	}
}

void ScreenAnimator::preserveAllBackgrounds() {
	debugC(9, kDebugLevelAnimator, "ScreenAnimator::preserveAllBackgrounds()");
	uint8 curPage = _screen->_curPage;
	_screen->_curPage = 2;

	AnimObject *curObject = _objectQueue;
	while (curObject) {
		if (curObject->active && !curObject->disable) {
			preserveOrRestoreBackground(curObject, false);
			curObject->bkgdChangeFlag = 0;
		}
		curObject = curObject->nextAnimObject;
	}
	_screen->_curPage = curPage;
}

void ScreenAnimator::flagAllObjectsForBkgdChange() {
	debugC(9, kDebugLevelAnimator, "ScreenAnimator::flagAllObjectsForBkgdChange()");
	AnimObject *curObject = _objectQueue;
	while (curObject) {
		curObject->bkgdChangeFlag = 1;
		curObject = curObject->nextAnimObject;
	}
}

void ScreenAnimator::flagAllObjectsForRefresh() {
	debugC(9, kDebugLevelAnimator, "ScreenAnimator::flagAllObjectsForRefresh()");
	AnimObject *curObject = _objectQueue;
	while (curObject) {
		curObject->refreshFlag = 1;
		curObject = curObject->nextAnimObject;
	}
}

void ScreenAnimator::restoreAllObjectBackgrounds() {
	debugC(9, kDebugLevelAnimator, "ScreenAnimator::restoreAllObjectBackground()");
	AnimObject *curObject = _objectQueue;
	_screen->_curPage = 2;
	
	while (curObject) {
		if (curObject->active && !curObject->disable) {
			preserveOrRestoreBackground(curObject, true);
			curObject->x2 = curObject->x1;
			curObject->y2 = curObject->y1;
		}
		curObject = curObject->nextAnimObject;
	}
	
	_screen->_curPage = 0;
}

void ScreenAnimator::preserveAnyChangedBackgrounds() {
	debugC(9, kDebugLevelAnimator, "ScreenAnimator::preserveAnyChangedBackgrounds()");
	AnimObject *curObject = _objectQueue;
	_screen->_curPage = 2;
	
	while (curObject) {
		if (curObject->active && !curObject->disable && curObject->bkgdChangeFlag) {
			preserveOrRestoreBackground(curObject, false);
			curObject->bkgdChangeFlag = 0;
		}
		curObject = curObject->nextAnimObject;
	}
	
	_screen->_curPage = 0;
}

void ScreenAnimator::preserveOrRestoreBackground(AnimObject *obj, bool restore) {
	debugC(9, kDebugLevelAnimator, "ScreenAnimator::preserveOrRestoreBackground(%p, %d)", (const void *)obj, restore);
	int x = 0, y = 0, width = obj->width, height = obj->height;
	
	if (restore) {
		x = obj->x2 >> 3;
		y = obj->y2;
	} else {
		x = obj->x1 >> 3;
		y = obj->y1;
	}
	
	if (x < 0)
		x = 0;
	if (y < 0)
		y = 0;
	
	int temp;
	
	temp = x + width;
	if (temp >= 39)
		x = 39 - width;
	temp = y + height;
	if (temp >= 136)
		y = 136 - height;

	if (restore)
		_screen->copyBlockToPage(_screen->_curPage, x << 3, y, width << 3, height, obj->background);
	else
		_screen->copyRegionToBuffer(_screen->_curPage, x << 3, y, width << 3, height, obj->background);
}

void ScreenAnimator::prepDrawAllObjects() {
	debugC(9, kDebugLevelAnimator, "ScreenAnimator::prepDrawAllObjects()");
	AnimObject *curObject = _objectQueue;
	int drawPage = 2;
	int flagUnk1 = 0, flagUnk2 = 0, flagUnk3 = 0;
	if (_noDrawShapesFlag)
		return;
	if (_vm->_brandonStatusBit & 0x20)
		flagUnk1 = 0x200;
	if (_vm->_brandonStatusBit & 0x40)
		flagUnk2 = 0x4000;

	while (curObject) {
		if (curObject->active) {
			int xpos = curObject->x1;
			int ypos = curObject->y1;
			
			int drawLayer = 0;
			if (!(curObject->flags & 0x800))
				drawLayer = 7;
			else if (curObject->disable)
				drawLayer = 0;
			else
				drawLayer = _vm->_sprites->getDrawLayer(curObject->drawY);
			
			// talking head functionallity
			if (_vm->_talkingCharNum != -1 && (_vm->_currentCharacter->currentAnimFrame != 88 || curObject->index != 0)) {
				const int16 baseAnimFrameTable1[] = { 0x11, 0x35, 0x59, 0x00, 0x00, 0x00 };
				const int16 baseAnimFrameTable2[] = { 0x15, 0x39, 0x5D, 0x00, 0x00, 0x00 };
				const int8 xOffsetTable1[] = { 2, 4, 0, 5, 2, 0, 0, 0 };
				const int8 xOffsetTable2[] = { 6, 4, 8, 3, 6, 0, 0, 0 };
				const int8 yOffsetTable1[] = { 0, 8, 1, 1, 0, 0, 0, 0 };
				const int8 yOffsetTable2[] = { 0, 8, 1, 1, 0, 0, 0, 0 };
				if (curObject->index == 0 || curObject->index <= 4) {
					int shapesIndex = 0;
					if (curObject->index == _vm->_charSayUnk3) {
						shapesIndex = _vm->_currHeadShape + baseAnimFrameTable1[curObject->index];
					} else {
						shapesIndex = baseAnimFrameTable2[curObject->index];
						int temp2 = 0;
						if (curObject->index == 2) {
							if (_vm->_characterList[2].sceneId == 77 || _vm->_characterList[2].sceneId == 86)
								temp2 = 1;
							else
								temp2 = 0;
						} else {
							temp2 = 1;
						}
						
						if (!temp2)
							shapesIndex = -1;
					}
						
					xpos = curObject->x1;
					ypos = curObject->y1;
						
					int tempX = 0, tempY = 0;
					if (curObject->flags & 0x1) {
						tempX = (xOffsetTable1[curObject->index] * _brandonScaleX) >> 8;
						tempY = yOffsetTable1[curObject->index];
					} else {
						tempX = (xOffsetTable2[curObject->index] * _brandonScaleX) >> 8;
						tempY = yOffsetTable2[curObject->index];
					}
					tempY = (tempY * _brandonScaleY) >> 8;
					xpos += tempX;
					ypos += tempY;
					
					if (_vm->_scaleMode && _brandonScaleX != 256)
						++xpos;
					
					if (curObject->index == 0 && shapesIndex != -1) {
						if (!(_vm->_brandonStatusBit & 2)) {
							flagUnk3 = 0x100;
							if ((flagUnk1 & 0x200) || (flagUnk2 & 0x4000))
								flagUnk3 = 0;
							
							int tempFlags = 0;
							if (flagUnk3 & 0x100) {
								tempFlags = curObject->flags & 1;
								tempFlags |= 0x800 | flagUnk1 | 0x100;
							}
							
							if (!(flagUnk3 & 0x100) && (flagUnk2 & 0x4000)) {
								tempFlags = curObject->flags & 1;
								tempFlags |= 0x900 | flagUnk1 | 0x4000;
								_screen->drawShape(drawPage, _vm->_shapes[shapesIndex], xpos, ypos, 2, tempFlags | 4, _vm->_brandonPoisonFlagsGFX, int(1), int(_vm->_brandonInvFlag), drawLayer, _brandonScaleX, _brandonScaleY);
							} else {
								if (!(flagUnk2 & 0x4000)) {
									tempFlags = curObject->flags & 1;
									tempFlags |= 0x900 | flagUnk1;
								}
								
								_screen->drawShape(drawPage, _vm->_shapes[shapesIndex], xpos, ypos, 2, tempFlags | 4, _vm->_brandonPoisonFlagsGFX, int(1), drawLayer, _brandonScaleX, _brandonScaleY);
							}
						}
					} else {
						if (shapesIndex != -1) {
							int tempFlags = 0;
							if (curObject->flags & 1)
								tempFlags = 1;
							_screen->drawShape(drawPage, _vm->_shapes[shapesIndex], xpos, ypos, 2, tempFlags | 0x800, drawLayer); 							
						}
					}
				}
			}
			
			xpos = curObject->x1;
			ypos = curObject->y1;
			
			curObject->flags |= 0x800;
			if (curObject->index == 0) {
				flagUnk3 = 0x100;
				
				if (flagUnk1 & 0x200 || flagUnk2 & 0x4000)
					flagUnk3 = 0;
				
				if (_vm->_brandonStatusBit & 2)
					curObject->flags &= 0xFFFFFFFE;
				
				if (!_vm->_scaleMode) {
					if (flagUnk3 & 0x100)
						_screen->drawShape(drawPage, curObject->sceneAnimPtr, xpos, ypos, 2, curObject->flags | flagUnk1 | 0x100, (uint8*)_vm->_brandonPoisonFlagsGFX, int(1), drawLayer);
					else if (flagUnk2 & 0x4000)
						_screen->drawShape(drawPage, curObject->sceneAnimPtr, xpos, ypos, 2, curObject->flags | flagUnk1 | 0x4000, int(_vm->_brandonInvFlag), drawLayer);
					else
						_screen->drawShape(drawPage, curObject->sceneAnimPtr, xpos, ypos, 2, curObject->flags | flagUnk1, drawLayer);
				} else {
					if (flagUnk3 & 0x100)
						_screen->drawShape(drawPage, curObject->sceneAnimPtr, xpos, ypos, 2, curObject->flags | flagUnk1 | 0x104, (uint8*)_vm->_brandonPoisonFlagsGFX, int(1), drawLayer, _brandonScaleX, _brandonScaleY);
					else if (flagUnk2 & 0x4000)
						_screen->drawShape(drawPage, curObject->sceneAnimPtr, xpos, ypos, 2, curObject->flags | flagUnk1 | 0x4004, int(_vm->_brandonInvFlag), drawLayer, _brandonScaleX, _brandonScaleY);
					else
						_screen->drawShape(drawPage, curObject->sceneAnimPtr, xpos, ypos, 2, curObject->flags | flagUnk1 | 0x4, drawLayer, _brandonScaleX, _brandonScaleY);
				}
			} else {
				if (curObject->index >= 16 && curObject->index <= 27)
					_screen->drawShape(drawPage, curObject->sceneAnimPtr, xpos, ypos, 2, curObject->flags | 4, drawLayer, (int)_vm->_scaleTable[curObject->drawY], (int)_vm->_scaleTable[curObject->drawY]);
				else
					_screen->drawShape(drawPage, curObject->sceneAnimPtr, xpos, ypos, 2, curObject->flags, drawLayer);
			}
		}
		curObject = curObject->nextAnimObject;
	}
}

void ScreenAnimator::copyChangedObjectsForward(int refreshFlag) {
	debugC(9, kDebugLevelAnimator, "ScreenAnimator::copyChangedObjectsForward(%d)", refreshFlag);

	for (AnimObject *curObject = _objectQueue; curObject; curObject = curObject->nextAnimObject) {
		if (curObject->active) {
			if (curObject->refreshFlag || refreshFlag) {
				int xpos = 0, ypos = 0, width = 0, height = 0;
				xpos = (curObject->x1>>3) - (curObject->width2>>3) - 1;
				ypos = curObject->y1 - curObject->height2;
				width = curObject->width + (curObject->width2>>3) + 2;
				height = curObject->height + curObject->height2*2;
				
				if (xpos < 1)
					xpos = 1;
				else if (xpos > 39)
					continue;

				if (xpos + width > 39)
					width = 39 - xpos;
				
				if (ypos < 8)
					ypos = 8;
				else if (ypos > 136)
					continue;

				if (ypos + height > 136)
					height = 136 - ypos;
				
				_screen->copyRegion(xpos << 3, ypos, xpos << 3, ypos, width << 3, height, 2, 0, Screen::CR_CLIPPED);
				curObject->refreshFlag = 0;
				_updateScreen = true;
			}
		}
	}

	if (_updateScreen) {
		_screen->updateScreen();
		_updateScreen = false;
	}
}

void ScreenAnimator::updateAllObjectShapes() {
	debugC(9, kDebugLevelAnimator, "ScreenAnimator::updateAllObjectShapes()");
	restoreAllObjectBackgrounds();
	preserveAnyChangedBackgrounds();
	prepDrawAllObjects();
	copyChangedObjectsForward(0);
}

void ScreenAnimator::animRemoveGameItem(int index) {
	debugC(9, kDebugLevelAnimator, "ScreenAnimator::animRemoveGameItem(%d)", index);
	restoreAllObjectBackgrounds();
	
	AnimObject *animObj = &_items[index];
	animObj->sceneAnimPtr = 0;
	animObj->animFrameNumber = -1;
	animObj->refreshFlag = 1;
	animObj->bkgdChangeFlag = 1;	
	updateAllObjectShapes();
	animObj->active = 0;
	
	objectRemoveQueue(_objectQueue, animObj);
}

void ScreenAnimator::animAddGameItem(int index, uint16 sceneId) {
	debugC(9, kDebugLevelAnimator, "ScreenAnimator::animRemoveGameItem(%d, %d)", index, sceneId);
	restoreAllObjectBackgrounds();
	assert(sceneId < _vm->_roomTableSize);
	Room *currentRoom = &_vm->_roomTable[sceneId];
	AnimObject *animObj = &_items[index];
	animObj->active = 1;
	animObj->refreshFlag = 1;
	animObj->bkgdChangeFlag = 1;
	animObj->drawY = currentRoom->itemsYPos[index];
	animObj->sceneAnimPtr = _vm->_shapes[216+currentRoom->itemsTable[index]];
	animObj->animFrameNumber = -1;
	animObj->x1 = currentRoom->itemsXPos[index];
	animObj->y1 = currentRoom->itemsYPos[index];
	animObj->x1 -= fetchAnimWidth(animObj->sceneAnimPtr, _vm->_scaleTable[animObj->drawY]) >> 1;
	animObj->y1 -= fetchAnimHeight(animObj->sceneAnimPtr, _vm->_scaleTable[animObj->drawY]);
	animObj->x2 = animObj->x1;
	animObj->y2 = animObj->y1;
	animObj->width2 = 0;
	animObj->height2 = 0;
	_objectQueue = objectQueue(_objectQueue, animObj);
	preserveAnyChangedBackgrounds();
	animObj->refreshFlag = 1;
	animObj->bkgdChangeFlag = 1;
}

void ScreenAnimator::animAddNPC(int character) {
	debugC(9, kDebugLevelAnimator, "ScreenAnimator::animAddNPC(%d)", character);
	restoreAllObjectBackgrounds();
	AnimObject *animObj = &_actors[character];
	const Character *ch = &_vm->_characterList[character];
	
	animObj->active = 1;
	animObj->refreshFlag = 1;
	animObj->bkgdChangeFlag = 1;
	animObj->drawY = ch->y1;
	animObj->sceneAnimPtr = _vm->_shapes[ch->currentAnimFrame];
	animObj->x1 = animObj->x2 = ch->x1 + _vm->_defaultShapeTable[ch->currentAnimFrame-7].xOffset;
	animObj->y1 = animObj->y2 = ch->y1 + _vm->_defaultShapeTable[ch->currentAnimFrame-7].yOffset;

	if (ch->facing >= 1 && ch->facing <= 3)
		animObj->flags |= 1;
	else if (ch->facing >= 5 && ch->facing <= 7)
		animObj->flags &= 0xFFFFFFFE;

	_objectQueue = objectQueue(_objectQueue, animObj);
	preserveAnyChangedBackgrounds();
	animObj->refreshFlag = 1;
	animObj->bkgdChangeFlag = 1;
}

AnimObject *ScreenAnimator::objectRemoveQueue(AnimObject *queue, AnimObject *rem) {
	debugC(9, kDebugLevelAnimator, "ScreenAnimator::objectRemoveQueue(%p, %p)", (const void *)queue, (const void *)rem);
	AnimObject *cur = queue;
	AnimObject *prev = queue;

	while (cur != rem && cur) {
		AnimObject *temp = cur->nextAnimObject;
		if (!temp)
			break;
		prev = cur;
		cur = temp;
	}
	
	if (cur == queue) {
		if (!cur)
			return 0;
		return cur->nextAnimObject;
	}
	
	if (!cur->nextAnimObject) {
		if (cur == rem) {
			if (!prev)
				return 0;
			else
				prev->nextAnimObject = 0;
		}
	} else {
		if (cur == rem)
			prev->nextAnimObject = rem->nextAnimObject;
	}
	
	return queue;
}

AnimObject *ScreenAnimator::objectAddHead(AnimObject *queue, AnimObject *head) {
	debugC(9, kDebugLevelAnimator, "ScreenAnimator::objectAddHead(%p, %p)", (const void *)queue, (const void *)head);
	head->nextAnimObject = queue;
	return head;
}

AnimObject *ScreenAnimator::objectQueue(AnimObject *queue, AnimObject *add) {
	debugC(9, kDebugLevelAnimator, "ScreenAnimator::objectQueue(%p, %p)", (const void *)queue, (const void *)add);
	if (add->drawY <= queue->drawY || !queue) {
		add->nextAnimObject = queue;
		return add;
	}
	AnimObject *cur = queue;
	AnimObject *prev = queue;
	while (add->drawY > cur->drawY) {
		AnimObject *temp = cur->nextAnimObject;
		if (!temp)
			break;
		prev = cur;
		cur = temp;
	}
	
	if (add->drawY <= cur->drawY) {
		prev->nextAnimObject = add;
		add->nextAnimObject = cur;
	} else {
		cur->nextAnimObject = add;
		add->nextAnimObject = 0;
	}
	return queue;
}

void ScreenAnimator::addObjectToQueue(AnimObject *object) {
	debugC(9, kDebugLevelAnimator, "ScreenAnimator::addObjectToQueue(%p)", (const void *)object);
	if (!_objectQueue)
		_objectQueue = objectAddHead(0, object);
	else
		_objectQueue = objectQueue(_objectQueue, object);
}

void ScreenAnimator::refreshObject(AnimObject *object) {
	debugC(9, kDebugLevelAnimator, "ScreenAnimator::refreshObject(%p)", (const void *)object);
	_objectQueue = objectRemoveQueue(_objectQueue, object);
	if (_objectQueue)
		_objectQueue = objectQueue(_objectQueue, object);
	else
		_objectQueue = objectAddHead(0, object);
}

void ScreenAnimator::makeBrandonFaceMouse() {
	debugC(9, kDebugLevelAnimator, "ScreenAnimator::makeBrandonFaceMouse()");
	Common::Point mouse = _vm->getMousePos();
	if (mouse.x >= _vm->_currentCharacter->x1)
		_vm->_currentCharacter->facing = 3;
	else
		_vm->_currentCharacter->facing = 5;
	animRefreshNPC(0);
	updateAllObjectShapes();
}

int16 ScreenAnimator::fetchAnimWidth(const uint8 *shape, int16 mult) {
	debugC(9, kDebugLevelAnimator, "ScreenAnimator::fetchAnimWidth(%p, %d)", (const void *)shape, mult);
	if (_vm->gameFlags().useAltShapeHeader)
		shape += 2;
	return (((int16)READ_LE_UINT16((shape+3))) * mult) >> 8;
}

int16 ScreenAnimator::fetchAnimHeight(const uint8 *shape, int16 mult) {
	debugC(9, kDebugLevelAnimator, "ScreenAnimator::fetchAnimHeight(%p, %d)", (const void *)shape, mult);
	if (_vm->gameFlags().useAltShapeHeader)
		shape += 2;
	return (int16)(((int8)*(shape+2)) * mult) >> 8;
}

void ScreenAnimator::setBrandonAnimSeqSize(int width, int height) {
	debugC(9, kDebugLevelAnimator, "ScreenAnimator::setBrandonAnimSeqSize(%d, %d)", width, height);
	restoreAllObjectBackgrounds();
	_brandonAnimSeqSizeWidth = _actors[0].width;
	_brandonAnimSeqSizeHeight = _actors[0].height;
	_actors[0].width = width + 1;
	_actors[0].height = height;
	preserveAllBackgrounds();
}

void ScreenAnimator::resetBrandonAnimSeqSize() {
	debugC(9, kDebugLevelAnimator, "ScreenAnimator::resetBrandonAnimSeqSize()");
	restoreAllObjectBackgrounds();
	_actors[0].width = _brandonAnimSeqSizeWidth;
	_actors[0].height = _brandonAnimSeqSizeHeight;
	preserveAllBackgrounds();
}

void ScreenAnimator::animRefreshNPC(int character) {
	debugC(9, kDebugLevelAnimator, "ScreenAnimator::animRefreshNPC(%d)", character);
	AnimObject *animObj = &_actors[character];
	Character *ch = &_vm->characterList()[character];

	animObj->refreshFlag = 1;
	animObj->bkgdChangeFlag = 1;
	int facing = ch->facing;
	if (facing >= 1 && facing <= 3)
		animObj->flags |= 1;
	else if (facing >= 5 && facing <= 7)
		animObj->flags &= 0xFFFFFFFE;
	
	animObj->drawY = ch->y1;
	animObj->sceneAnimPtr = _vm->shapes()[ch->currentAnimFrame];
	animObj->animFrameNumber = ch->currentAnimFrame;
	if (character == 0) {
		if (_vm->brandonStatus() & 10) {
			animObj->animFrameNumber = 88;
			ch->currentAnimFrame = 88;
		}
		if (_vm->brandonStatus() & 2) {
			animObj->animFrameNumber = _brandonDrawFrame;
			ch->currentAnimFrame = _brandonDrawFrame;
			animObj->sceneAnimPtr = _vm->shapes()[_brandonDrawFrame];
			if (_vm->_brandonStatusBit0x02Flag) {
				++_brandonDrawFrame;
				// TODO: check this
				if (_brandonDrawFrame >= 122) {
					_brandonDrawFrame = 113;
					_vm->_brandonStatusBit0x02Flag = 0;
				}
			}
		}
	}
	
	int xOffset = _vm->_defaultShapeTable[ch->currentAnimFrame-7].xOffset;
	int yOffset = _vm->_defaultShapeTable[ch->currentAnimFrame-7].yOffset;
	
	if (_vm->_scaleMode) {
		animObj->x1 = ch->x1;
		animObj->y1 = ch->y1;
		
		int newScale = _vm->_scaleTable[ch->y1];
		_brandonScaleX = newScale;
		_brandonScaleY = newScale;

		animObj->x1 += (_brandonScaleX * xOffset) >> 8;
		animObj->y1 += (_brandonScaleY * yOffset) >> 8;
	} else {
		animObj->x1 = ch->x1 + xOffset;
		animObj->y1 = ch->y1 + yOffset;
	}
	animObj->width2 = 4;
	animObj->height2 = 3;

	refreshObject(animObj);
}

void ScreenAnimator::setCharacterDefaultFrame(int character) {
	debugC(9, kDebugLevelAnimator, "ScreenAnimator::setCharacterDefaultFrame()");
	static uint16 initFrameTable[] = {
		7, 41, 77, 0, 0
	};
	assert(character < ARRAYSIZE(initFrameTable));
	Character *edit = &_vm->characterList()[character];
	edit->sceneId = 0xFFFF;
	edit->facing = 0;
	edit->currentAnimFrame = initFrameTable[character];
	// edit->unk6 = 1;
}

void ScreenAnimator::setCharactersHeight() {
	debugC(9, kDebugLevelAnimator, "ScreenAnimator::setCharactersHeight()");
	static int8 initHeightTable[] = {
		48, 40, 48, 47, 56,
		44, 42, 47, 38, 35,
		40
	};
	for (int i = 0; i < 11; ++i)
		_vm->characterList()[i].height = initHeightTable[i];
}

} // end of namespace Kyra

