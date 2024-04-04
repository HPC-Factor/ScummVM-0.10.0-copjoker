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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/engines/gob/scenery_v1.cpp $
 * $Id: scenery_v1.cpp 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

#include "common/stdafx.h"
#include "common/endian.h"

#include "gob/gob.h"
#include "gob/scenery.h"
#include "gob/util.h"
#include "gob/cdrom.h"

namespace Gob {

Scenery_v1::Scenery_v1(GobEngine *vm) : Scenery(vm) {
}

int16 Scenery_v1::loadAnim(char search) {
	if (_vm->_cdrom->_cdPlaying) {
		while (_vm->_cdrom->getTrackPos() != -1)
		    _vm->_util->longDelay(50);

		_vm->_cdrom->_cdPlaying = false;
	}

	return Scenery::loadAnim(search);
}

} // End of namespace Gob
