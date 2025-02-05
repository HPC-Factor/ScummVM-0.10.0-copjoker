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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/engines/gob/map_v2.cpp $
 * $Id: map_v2.cpp 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

#include "common/stdafx.h"
#include "common/stream.h"

#include "gob/gob.h"
#include "gob/map.h"
#include "gob/global.h"
#include "gob/goblin.h"
#include "gob/inter.h"
#include "gob/game.h"
#include "gob/parse.h"
#include "gob/mult.h"

namespace Gob {

Map_v2::Map_v2(GobEngine *vm) : Map_v1(vm) {
}

Map_v2::~Map_v2() {
	_passMap = 0;
}

void Map_v2::init(void) {
}

void Map_v2::loadMapObjects(const char *avjFile) {
	uint8 wayPointsCount;
	int16 var;
	int16 id;
	int16 mapWidth, mapHeight;
	int16 tmp;
	byte *variables;
	byte *extData;
	uint32 tmpPos;
	uint32 passPos;

	var = _vm->_parse->parseVarIndex();
	variables = _vm->_global->_inter_variables + var;

	id = _vm->_inter->load16();

	if (id == -1) {
		_passMap = (int8 *)(_vm->_global->_inter_variables + var);
		return;
	}

	extData = _vm->_game->loadExtData(id, 0, 0);
	Common::MemoryReadStream mapData(extData, 4294967295U);

	if (mapData.readByte() == 3) {
		_screenWidth = 640;
		_passWidth = 65;
	} else {
		_screenWidth = 320;
		_passWidth = 40;
	}
	_wayPointsCount = mapData.readByte();
	_tilesWidth = mapData.readSint16LE();
	_tilesHeight = mapData.readSint16LE();

	_bigTiles = !(_tilesHeight & 0xFF00);
	_tilesHeight &= 0xFF;

	_mapWidth = _screenWidth / _tilesWidth;
	_mapHeight = 200 / _tilesHeight;

	passPos = mapData.pos();
	mapData.skip(_mapWidth * _mapHeight);

	if (*extData == 1)
		wayPointsCount = _wayPointsCount = 40;
	else
		wayPointsCount = _wayPointsCount == 0 ? 1 : _wayPointsCount;

	if (_wayPoints)
		delete[] _wayPoints;

	_wayPoints = new Point[wayPointsCount];
	for (int i = 0; i < _wayPointsCount; i++) {
		_wayPoints[i].x = mapData.readSByte();
		_wayPoints[i].y = mapData.readSByte();
		_wayPoints[i].notWalkable = mapData.readSByte();
	}

	// In the original asm, this writes byte-wise into the variables-array
	tmpPos = mapData.pos();
	mapData.seek(passPos);
	if (variables != _vm->_global->_inter_variables) {
		byte *sizes;

		_passMap = (int8 *) variables;
		mapHeight = 200 / _tilesHeight;
		mapWidth = _screenWidth / _tilesWidth;
		sizes = _vm->_global->_inter_variablesSizes +
			(((byte *) _passMap) - _vm->_global->_inter_variables);
		for (int i = 0; i < mapHeight; i++) {
			for (int j = 0; j < mapWidth; j++)
				setPass(j, i, mapData.readSByte());
			memset(sizes + i * _passWidth, 0, mapWidth);
		}
	}
	mapData.seek(tmpPos);

	tmp = mapData.readSint16LE();
	mapData.skip(tmp * 14);
	tmp = mapData.readSint16LE();
	mapData.skip(tmp * 14 + 28);
	tmp = mapData.readSint16LE();
	mapData.skip(tmp * 14);

	_vm->_goblin->_gobsCount = tmp;
	for (int i = 0; i < _vm->_goblin->_gobsCount; i++)
		loadGoblinStates(mapData, i);

	_vm->_goblin->_soundSlotsCount = _vm->_inter->load16();
	for (int i = 0; i < _vm->_goblin->_soundSlotsCount; i++)
		_vm->_goblin->_soundSlots[i] = _vm->_inter->loadSound(1);

	delete[] extData;
}

void Map_v2::loadGoblinStates(Common::SeekableReadStream &data, int index) {
	Mult::Mult_GobState *statesPtr;
	Mult::Mult_GobState *gobState;
	int8 indices[102];
	uint8 statesCount;
	uint8 dataCount;
	int16 state;
	uint32 tmpPos;

	memset(indices, -1, 101);
	_vm->_mult->_objects[index].goblinStates = new Mult::Mult_GobState*[101];
	memset(_vm->_mult->_objects[index].goblinStates, 0,
			101 * sizeof(Mult::Mult_GobState *));

	data.read(indices, 100);
	tmpPos = data.pos();
	statesCount = 0;
	for (int i = 0; i < 100; i++) {
		if (indices[i] != -1) {
			statesCount++;
			data.skip(4);
			dataCount = data.readByte();
			statesCount += dataCount;
			data.skip(dataCount * 9);
		}
	}

	data.seek(tmpPos);

	statesPtr = new Mult::Mult_GobState[statesCount];
	_vm->_mult->_objects[index].goblinStates[0] = statesPtr;
	for (int i = 0; i < 100; i++) {
		state = indices[i];
		if (state != -1) {
			_vm->_mult->_objects[index].goblinStates[state] = statesPtr++;
			gobState = _vm->_mult->_objects[index].goblinStates[state];

			gobState[0].animation = data.readSint16LE();
			gobState[0].layer = data.readSint16LE();
			dataCount = data.readByte();
			gobState[0].dataCount = dataCount;
			for (uint8 j = 1; j <= dataCount; j++) {
				data.skip(1);
				gobState[j].sndItem = data.readSByte();
				data.skip(1);
				gobState[j].sndFrame = data.readByte();
				data.skip(1);
				gobState[j].freq = data.readSint16LE();
				gobState[j].repCount = data.readSByte();
				gobState[j].speaker = data.readByte();
				statesPtr++;
			}
		}
	}
}

void Map_v2::findNearestToGob(Mult::Mult_Object *obj) {
	int16 wayPoint = findNearestWayPoint(obj->goblinX, obj->goblinY);

	if (wayPoint != -1)
		obj->nearestWayPoint = wayPoint;
}

void Map_v2::findNearestToDest(Mult::Mult_Object *obj) {
	int16 wayPoint = findNearestWayPoint(obj->destX, obj->destY);

	if (wayPoint != -1)
		obj->nearestDest = wayPoint;
}

void Map_v2::optimizePoints(Mult::Mult_Object *obj, int16 x, int16 y) {
	if (obj->nearestWayPoint < obj->nearestDest) {
		for (int i = obj->nearestWayPoint; i <= obj->nearestDest; i++) {
			if (checkDirectPath(obj, x, y, _wayPoints[i].x, _wayPoints[i].y) == 1)
				obj->nearestWayPoint = i;
		}
	} else {
		for (int i = obj->nearestWayPoint;
		     i >= obj->nearestDest && (_wayPoints[i].notWalkable != 1); i--) {
			if (checkDirectPath(obj, x, y, _wayPoints[i].x, _wayPoints[i].y) == 1)
				obj->nearestWayPoint = i;
		}
	}
}

} // End of namespace Gob
