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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/engines/parallaction/inventory.h $
 * $Id: inventory.h 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

#ifndef PARALLACTION_INVENTORY_H
#define PARALLACTION_INVENTORY_H



namespace Parallaction {


struct Cnv;

struct InventoryItem {
	uint32		_id;            // lowest 16 bits are always zero
	uint16		_index;
};

#define MAKE_INVENTORY_ID(x) (((x) & 0xFFFF) << 16)


extern InventoryItem _inventory[];

void initInventory();
void destroyInventory();
void openInventory();
void closeInventory();
int16 isItemInInventory(int32 v);
void cleanInventory();
void addInventoryItem(uint16 item);

void highlightInventoryItem(int16 pos, byte color);
void refreshInventory(const char *character);

void extractInventoryGraphics(int16 pos, byte *dst);


} // namespace Parallaction

#endif
