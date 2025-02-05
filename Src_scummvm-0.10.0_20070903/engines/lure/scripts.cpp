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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/engines/lure/scripts.cpp $
 * $Id: scripts.cpp 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

#include "lure/scripts.h"
#include "lure/res.h"
#include "lure/game.h"
#include "lure/room.h"
#include "common/stack.h"
#include "common/endian.h"

namespace Lure {

// This list of hotspot Ids are used by sequence method #5 to deallocate a set 
// of hotspot Ids at once
uint16 dealloc_list_1[] = {0x13F2, 0x418, 0x2711, 0x2712, 0x40D, 0x3EA, 0x411, 0};
uint16 dealloc_list_2[] = {0x2729, 0x272A, 0x272B, 0x272C, 0x272E, 0x272E, 0x272F, 0};
uint16 dealloc_list_3[] = {0x3EF, 0x3E9, 0x3EB, 0x3EC, 0x3ED, 0x3EE, 0x3F0, 0x3F1, 
	0x420, 0x429, 0x436, 0x2715, 0x2716, 0x2717, 0x2718, 0x2719, 0x271A, 0x271E, 
	0x271F, 0x2720, 0x2721, 0x2722, 0x2725, 0x2726, 0};
uint16 *hotspot_dealloc_set[4] = {&dealloc_list_1[0], &dealloc_list_2[0],
	&dealloc_list_3[1], &dealloc_list_3[0]};

/*------------------------------------------------------------------------*/
/*-  Script Method List                                                  -*/
/*-                                                                      -*/
/*------------------------------------------------------------------------*/

// activateHotspot
// Activates a hotspot entry for active use

void Script::activateHotspot(uint16 hotspotId, uint16 v2, uint16 v3) {
	Resources::getReference().activateHotspot(hotspotId);
}

// setHotspotScript
// Sets a hotspot's animation script offset from a master table of offsets

void Script::setHotspotScript(uint16 hotspotId, uint16 scriptIndex, uint16 v3) {
	Resources &res = Resources::getReference();
	uint16 offset = res.getHotspotScript(scriptIndex);
	Hotspot *hotspot = res.getActiveHotspot(hotspotId);

	if (hotspot != NULL) {
		hotspot->setHotspotScript(offset);
	} else {
		HotspotData *hs = res.getHotspot(hotspotId);
		assert(hs);
		hs->hotspotScriptOffset = offset;
	}
}

void Script::method2(uint16 v1, uint16 v2, uint16 v3) {
	// Not yet implemented
}

// Sets the bitmask indicating what group of rooms/hotspots to display when the
// player is issuing commands via the "Tell" action

void Script::setHotspotFlagMask(uint16 maskVal, uint16 v2, uint16 v3) {
	ValueTableData &fields = Resources::getReference().fieldList();
	fields.hdrFlagMask() = (uint8) maskVal;
}

// Clears the sequence delay list

void Script::clearSequenceDelayList(uint16 v1, uint16 scriptIndex, uint16 v3) {
	Resources::getReference().delayList().clear();
}

// Deactivates a set of predefined of hotspots in a given list index

void Script::deactivateHotspotSet(uint16 listIndex, uint16 v2, uint16 v3) {
	if (listIndex >= 3) 
		error("Script::deactiveHotspotSet - Invalid list index");
	Resources &res = Resources::getReference();
	uint16 *hotspotId = hotspot_dealloc_set[listIndex];

	while (*hotspotId != 0) {
		res.deactivateHotspot(*hotspotId);
		++hotspotId;
	}
}

// Resets the pathfinder

void Script::resetPathfinder(uint16 v1, uint16 v2, uint16 v3) {
	// Reset the pathfinding for any characters
	Resources &res = Resources::getReference();

	for (uint16 charId = PLAYER_ID + 1; charId < FIRST_NONCHARACTER_ID; ++charId) {
		Hotspot *charHotspot = res.getActiveHotspot(charId);
		if (charHotspot) charHotspot->pathFinder().clear();
	}
}

// deactivates the specified hotspot from active animation

void Script::deactivateHotspot(uint16 hotspotId, uint16 v2, uint16 v3) {
	Resources &rsc = Resources::getReference();
	if (hotspotId < START_NONVISUAL_HOTSPOT_ID) 
		rsc.deactivateHotspot(hotspotId);
	HotspotData *hs = rsc.getHotspot(hotspotId);
	hs->roomNumber = 0xffff;
	hs->flags |= 0x20;
	if (hotspotId < START_NONVISUAL_HOTSPOT_ID) 
		hs->layer = 0xff;
}

// Sets the offset for the table of action sequence offsets for the given
// hotspot

void Script::setDesc(uint16 hotspotId, uint16 descId, uint16 v3) {
	Resources &res = Resources::getReference();
	HotspotData *hotspot = res.getHotspot(hotspotId);
	assert(hotspot);
	hotspot->descId = descId;
}

// Add a sequence to be executed after a specified delay

void Script::addDelayedSequence(uint16 seqOffset, uint16 delay, uint16 canClear) {
	SequenceDelayList &list = Resources::getReference().delayList();
	list.add(delay, seqOffset, canClear != 0);
}

// Checks whether the given character is in the specified room, and stores
// the result in the general value field

void Script::characterInRoom(uint16 characterId, uint16 roomNumber, uint16 v3) {
	Resources &res = Resources::getReference();
	uint16 result = 0;
	if (characterId >= PLAYER_ID) {
		HotspotData *hotspot = res.getHotspot(characterId);
		if (hotspot->roomNumber == roomNumber)
			result = 1;
	}

	res.fieldList().setField(GENERAL, result);
}

// Changes the given hotspot's name to a new name

void Script::setHotspotName(uint16 hotspotId, uint16 nameId, uint16 v3) {
	HotspotData *hotspot = Resources::getReference().getHotspot(hotspotId);
	hotspot->nameId = nameId;
}

// Unsure about this method, but at the moment I think it plays a sound

void Script::playSound(uint16 v1, uint16 v2, uint16 v3) {
	// No implementation currently	
}

// Displays the given string resource Id in a dialog

void Script::displayDialog(uint16 stringId, uint16 v2, uint16 v3) {
	Dialog::show(stringId);
}

// Flags for remotely viewing a room

void Script::remoteRoomViewSetup(uint16 v1, uint16 v2, uint16 v3) {
	Hotspot *player = Resources::getReference().getActiveHotspot(PLAYER_ID);

	player->setTickProc(0); // disable player actions
	Resources::getReference().fieldList().setField(OLD_ROOM_NUMBER,
		player->roomNumber());
}

// Starts a character speaking out loud to themselves (ie. no destination character)

void Script::startSpeakingToNoone(uint16 characterId, uint16 stringId, uint16 v3) {
	Resources &res = Resources::getReference();
	Hotspot *charHotspot = res.getActiveHotspot(characterId);
	assert(charHotspot);

	charHotspot->converse(NOONE_ID, stringId, false);
}


void Script::playMusic(uint16 musicNum, uint16 v2, uint16 v3) {
	// TODO: Play a given music
	warning("TODO: Play music #%d", musicNum);
}

// Gets the current blocked state for the given door and stores it in the
// general value field

void Script::getDoorBlocked(uint16 hotspotId, uint16 v2, uint16 v3) {
	Resources &res = Resources::getReference();
	RoomExitJoinData *joinRec = res.getExitJoin(hotspotId);
	res.fieldList().setField(GENERAL, joinRec->blocked);
}

// Checks whether the Skorl is in the cell

void Script::isSkorlInCell(uint16 v1, uint16 v2, uint16 v3) {
	Resources &res = Resources::getReference();
	HotspotData *hotspot = res.getHotspot(SKORL_ID);
	uint16 v = (hotspot->roomNumber == 1) ? 0 : 1;
	res.fieldList().setField(GENERAL, v);
}

// Called by the script when Ratpouch is pushing the bricks in the Outer Cell

void Script::ratpouchPushBricks(uint16 v1, uint16 v2, uint16 v3)
{
	Resources &res = Resources::getReference();

	// Mark the bricks exit as now open
	RoomExitJoinData *joinRec = res.getExitJoin(BRICKS_ID);
	joinRec->blocked = 0;

	// Set Ratpouch moving through the new exit to room #7
	Hotspot *ratpouchHotspot = res.getActiveHotspot(RATPOUCH_ID);
	ratpouchHotspot->setActions(0); // Make sure he can't be interrupted
	ratpouchHotspot->currentActions().clear();
	ratpouchHotspot->currentActions().addBack(DISPATCH_ACTION, 7);
}

// Causes the current character to change to the specified room and position

void Script::characterChangeRoom(uint16 y, uint16 x, uint16 roomNumber) {
	Resources &res = Resources::getReference();
	ValueTableData &fields = res.fieldList();
	Hotspot *charHotspot = res.getActiveHotspot(fields.getField(CHARACTER_HOTSPOT_ID));
	assert(charHotspot);

	uint16 newRoomNumber = roomNumber & 0xff;
	Direction newDirection = (Direction)(roomNumber >> 8);

	Support::characterChangeRoom(*charHotspot, newRoomNumber, 
		(int16) (x - 0x80), (int16) (y - 0x80), newDirection);
}

// Pauses Ratpouch for a long period (as good as idefinite)

void Script::pauseRatpouch(uint16 v1, uint16 v2, uint16 v3)
{
	Resources &res = Resources::getReference();
	Hotspot *ratpouch = res.getActiveHotspot(RATPOUCH_ID);
	assert(ratpouch);
	ratpouch->setCharacterMode(CHARMODE_PAUSED);
	ratpouch->setDelayCtr(0x7fff);
}

// Sets a character to a given hotspot script, and sets the character's current 
// action to executing a script

void Script::setBlockingHotspotScript(uint16 charId, uint16 scriptIndex, uint16 v3) {
	Resources &r = Resources::getReference();
	uint16 offset = r.getHotspotScript(scriptIndex);

	Hotspot *hs = r.getActiveHotspot(charId);
	hs->setHotspotScript(offset);
	hs->currentActions().top().setAction(EXEC_HOTSPOT_SCRIPT);
	hs->setOccupied(true);
}

// Decrements the number of inventory itemst he player has

void Script::decrInventoryItems(uint16 v1, uint16 v2, uint16 v3) {
	// module doesn't use a static counter for the number of
	// inventory items, so don't do anything
}

void Script::setTalking(uint16 characterId, uint16 destHotspot, uint16 messageId) {
	Hotspot *hotspot = Resources::getReference().getActiveHotspot(characterId);
	if (hotspot)
		hotspot->converse(destHotspot, messageId, false);	
}

// Sets the current action ctr value for the given hotspot

void Script::setActionCtr(uint16 hotspotId, uint16 value, uint16 v3) {
	HotspotData *hotspot = Resources::getReference().getHotspot(hotspotId);
	assert(hotspot);
	hotspot->actionCtr = value;
}

// Starts the designated character speaking the given message to a given dest hotspot

void Script::startSpeaking(uint16 characterId, uint16 destHotspot, uint16 messageId) {
	Hotspot *hotspot = Resources::getReference().getActiveHotspot(characterId);
	hotspot->converse(destHotspot, messageId, false);
}

// Disables the given hotspot from being highlighted by the cursor

void Script::disableHotspot(uint16 hotspotId, uint16 v2, uint16 v3) {
	HotspotData *hotspot = Resources::getReference().getHotspot(hotspotId);
	hotspot->flags |= 0x20;	
}

// Called when the sack is cut with the knife

void Script::cutSack(uint16 hotspotId, uint16 v2, uint16 v3) {
	Resources &res = Resources::getReference();
	HotspotData *data = res.getHotspot(SACK_ID);
	data->startY = 138;
	Hotspot *activeHotspot = res.getActiveHotspot(SACK_ID);
	if (activeHotspot)
		activeHotspot->setPosition(data->startX, data->startY);
}

// Increase the player's number by the specified amount

void Script::increaseNumGroats(uint16 v1, uint16 numGroats, uint16 v3) {
	ValueTableData &fields = Resources::getReference().fieldList();
	fields.numGroats() += numGroats;
}

// Enables the flags for the given hotspot for it to be actively highlighted

void Script::enableHotspot(uint16 hotspotId, uint16 v2, uint16 v3) {
	HotspotData *hotspot = Resources::getReference().getHotspot(hotspotId);
	// Clear flag 0x20 and add flag 0x80
	hotspot->flags = (hotspot->flags & 0xdf) | 0x80;
}

// Display a message

void Script::displayMessage2(uint16 messageId, uint16 hotspotId, uint16 v3) {
	Hotspot *hotspot = Resources::getReference().getActiveHotspot(hotspotId);
	assert(hotspot);
	hotspot->showMessage(messageId);
}

void Script::startOilBurner(uint16 v1, uint16 v2, uint16 v3) {
	Hotspot *hotspot = Resources::getReference().getActiveHotspot(OIL_BURNER_ID);
	assert(hotspot);
	hotspot->setPosition(152, hotspot->y());
	hotspot->setTickProc(STANDARD_ANIM_TICK_PROC);
}

// Transforms the player

void Script::transformPlayer(uint16 v1, uint16 v2, uint16 v3) {
	Resources &res = Resources::getReference();
	HotspotData *hotspot = res.getHotspot(TRANSFORM_ID);
	HotspotData *player = res.getHotspot(PLAYER_ID);

	hotspot->roomNumber = player->roomNumber;
	hotspot->startX = player->startX - 14;
	hotspot->startY = player->startY - 10;
	
	Hotspot *activeHotspot = res.addHotspot(TRANSFORM_ID);
	activeHotspot->setActionCtr(0);
	activeHotspot->setHotspotScript(0x630);
}

// Marks the jail door in room 14 for closing

void Script::jailClose(uint16 v1, uint16 v2, uint16 v3) {
	RoomExitJoinData *joinRec = Resources::getReference().getExitJoin(0x2719);
	joinRec->blocked = 1;
}

// Sets the sequence result to 1 if the given secondary description for a 
// hotspot is empty (for inventory items, this gives the description before
// the item is initially picked up)

void Script::checkDroppedDesc(uint16 hotspotId, uint16 v2, uint16 v3) {
	Resources &res = Resources::getReference();
	HotspotData *hotspot = res.getHotspot(hotspotId);
	uint16 seqResult = (hotspot->descId2 == 0) ? 1 : 0;
	res.fieldList().setField(SEQUENCE_RESULT, seqResult);
}

// Marks the given door hotspot for closing

void Script::doorClose(uint16 hotspotId, uint16 v2, uint16 v3) {
	RoomExitJoinData *joinRec = Resources::getReference().getExitJoin(hotspotId);
	if (!joinRec) error("Tried to close a non-door");
	joinRec->blocked = 1;
}

// Marks the given door hotspot for opening

void Script::doorOpen(uint16 hotspotId, uint16 v2, uint16 v3) {
	RoomExitJoinData *joinRec = Resources::getReference().getExitJoin(hotspotId);
	if (!joinRec) error("Tried to close a non-door");
	joinRec->blocked = 0;
}

// Makes the specified NPC wait for the player to walk to them

void Script::npcWait(uint16 hotspotId, uint16 v2, uint16 v3) {
       Hotspot *hotspot = Resources::getReference().getActiveHotspot(hotspotId);
       assert(hotspot);
       hotspot->setCharacterMode(CHARMODE_WAIT_FOR_INTERACT);
       hotspot->setDelayCtr(130);
}

// Lookup the given message Id for the specified character and display in a dialog

void Script::displayMessage(uint16 messageId, uint16 characterId, uint16 unknownVal) {
	Hotspot *hotspot = Resources::getReference().getActiveHotspot(characterId);
	if (hotspot != NULL)
		hotspot->showMessage(messageId);
}

// Creates a new dispatch action with the given support data entry

void Script::setNewSupportData(uint16 hotspotId, uint16 index, uint16 v3) {
	Resources &res = Resources::getReference();
	uint16 dataId = res.getCharOffset(index);
	CharacterScheduleEntry *entry = res.charSchedules().getEntry(dataId);

	Hotspot *h = res.getActiveHotspot(hotspotId);
	h->setBlockedFlag(false);
	h->currentActions().addFront(DISPATCH_ACTION, entry, h->roomNumber());
	h->setActionCtr(0);
}

// Replaces the existing current action with a new dispatch data entry

void Script::setSupportData(uint16 hotspotId, uint16 index, uint16 v3) {
	Resources &res = Resources::getReference();
	uint16 dataId = res.getCharOffset(index);

	CharacterScheduleEntry *entry = res.charSchedules().getEntry(dataId);
	assert(entry != NULL);
	Hotspot *h = res.getActiveHotspot(hotspotId);
	assert(h != NULL);
	assert(!h->currentActions().isEmpty());

	h->currentActions().pop();
	h->currentActions().addFront(DISPATCH_ACTION, entry, h->roomNumber());
}

// Assign the given hotspot item to the player's inventory

void Script::givePlayerItem(uint16 hotspotId, uint16 v2, uint16 v3) {
	HotspotData *hotspot = Resources::getReference().getHotspot(hotspotId);
	hotspot->roomNumber = PLAYER_ID;
	hotspot->flags |= 0x80;
}

// Decrease the number of graots the player has

void Script::decreaseNumGroats(uint16 characterId, uint16 numGroats, uint16 v3) {
	ValueTableData &fields = Resources::getReference().fieldList();
	fields.numGroats() -= numGroats;
}

// Sets a character moving to the player's room (if they're not already there)

void Script::moveCharacterToPlayer(uint16 characterId, uint16 v2, uint16 v3) {
	Resources &res = Resources::getReference();
	Hotspot *playerHotspot = res.getActiveHotspot(PLAYER_ID);
	Hotspot *charHotspot = res.getActiveHotspot(characterId);
	assert(charHotspot); 

	// If character in same room as player, then no need to do anything
	if (!charHotspot->currentActions().isEmpty() &&
		(charHotspot->currentActions().top().roomNumber() == playerHotspot->roomNumber()))
		return;

	uint16 destRoom = playerHotspot->roomNumber();
	RoomTranslationRecord *rec; 
	for (rec = &roomTranslations[0]; rec->srcRoom != 0; ++rec) {
		if (rec->srcRoom == destRoom) {
			destRoom = rec->destRoom;
			break;
		}
	}

	if (charHotspot->currentActions().isEmpty())
		charHotspot->currentActions().addFront(DISPATCH_ACTION,  destRoom);
	else
		charHotspot->currentActions().top().setRoomNumber(destRoom);
}

// Sets the tick handler for the village Skorl to an alternate handler

void Script::setVillageSkorlTickProc(uint16 v1, uint16 v2, uint16 v3) {
	HotspotData *hotspot = Resources::getReference().getHotspot(0x3F1);
	hotspot->tickProcOffset = 0x7efa;
}

// Free Goewin from captivity

void Script::freeGoewin(uint16 v1, uint16 v2, uint16 v3) {
	HotspotData *goewin = Resources::getReference().getHotspot(GOEWIN_ID);
	goewin->actions = 0x820C00;   // Enable Talk To, Give, Bribe, and Ask for
	goewin->actionCtr = 1;
}

// Barman serving the player

void Script::barmanServe(uint16 v1, uint16 v2, uint16 v3) {
	Resources &res = Resources::getReference();
	Hotspot *player = res.getActiveHotspot(PLAYER_ID);
	BarEntry &barEntry = res.barmanLists().getDetails(player->roomNumber());
	
	for (int index = 0; index < NUM_SERVE_CUSTOMERS; ++index) {
		if (barEntry.customers[index].hotspotId == PLAYER_ID) {
			barEntry.customers[index].serveFlags |= 5;
			break;
		}
	}
}

// Stores the current number of groats in the general field

void Script::getNumGroats(uint16 v1, uint16 v2, uint16 v3) {
	ValueTableData fields = Resources::getReference().fieldList();
	fields.setField(GENERAL, fields.numGroats());
}

// Enables the talk action on the two gargoyles

void Script::enableGargoylesTalk(uint16 v1, uint16 v2, uint16 v3) {
	Resources &res = Resources::getReference();
	HotspotData *g1 = res.getHotspot(0x42C);
	HotspotData *g2 = res.getHotspot(0x42D);
	g1->actions = 1 << (TALK_TO - 1);
	g2->actions = 1 << (TALK_TO - 1);
}

// Flags the player as dead

void Script::killPlayer(uint16 v1, uint16 v2, uint16 v3) {
	Game &game = Game::getReference();
	game.setState(GS_RESTORE_RESTART);
}

// Loads the specified animation, completely bypassing the standard process
// of checking for a load proc/sequence

void Script::animationLoad(uint16 hotspotId, uint16 v2, uint16 v3) {
	Resources::getReference().addHotspot(hotspotId);
}

// Adds the passed actions to the available actions for the given hotspot

void Script::addActions(uint16 hotspotId, uint16 actions, uint16 v3) {
	HotspotData *hotspot = Resources::getReference().getHotspot(hotspotId);
	hotspot->actions |= actions;
}

// Generates a random number and stores it in the general field

void Script::randomToGeneral(uint16 maxVal, uint16 minVal, uint16 v3) {
	Common::RandomSource rnd;
	uint16 v = minVal + rnd.getRandomNumber(maxVal - minVal);
	Resources::getReference().fieldList().setField(GENERAL, v);
}

// Checks the status of the cell door, and starts music depending on it's state

void Script::checkCellDoor(uint16 v1, uint16 v2, uint16 v3) {
	// In the original game, this method checks to see if the cell door
	// is currently open, if it is, starts a music sequence. 
	// TODO: Implement starting music if cell door is open
}

// Has something to do with music handling 

void Script::method66(uint16 hotspotId, uint16 actions, uint16 v3) {
	// For now, simply set the general value field so that the Skorl schedule
	// will work properly
	Resources::getReference().fieldList().setField(GENERAL, 0);
}

typedef void(*SequenceMethodPtr)(uint16, uint16, uint16);

struct SequenceMethodRecord {
	uint8 methodIndex;
	SequenceMethodPtr proc;
};

SequenceMethodRecord scriptMethods[] = {
	{0, Script::activateHotspot}, 
	{1, Script::setHotspotScript},
	{3, Script::setHotspotFlagMask},
	{4, Script::clearSequenceDelayList},
	{5, Script::deactivateHotspotSet},
	{6, Script::deactivateHotspot},
	{7, Script::resetPathfinder},
	{8, Script::addDelayedSequence},
	{10, Script::characterInRoom},
	{11, Script::setDesc},
	{12, Script::setHotspotName},
	{13, Script::playSound},
	{16, Script::displayDialog},
	{18, Script::remoteRoomViewSetup},
	{19, Script::startSpeakingToNoone},
	{20, Script::checkCellDoor},
	{21, Script::playMusic},
	{22, Script::getDoorBlocked},
	{23, Script::isSkorlInCell},
	{24, Script::ratpouchPushBricks},
	{25, Script::characterChangeRoom},
	{26, Script::pauseRatpouch},
	{27, Script::setBlockingHotspotScript},
	{28, Script::decrInventoryItems},
	{29, Script::setTalking},
	{30, Script::setActionCtr},
	{31, Script::startSpeaking},
	{32, Script::disableHotspot},
	{33, Script::cutSack},
	{34, Script::increaseNumGroats},
	{35, Script::enableHotspot},
	{36, Script::displayMessage2},
	{37, Script::startOilBurner},
	{38, Script::transformPlayer},
	{39, Script::jailClose},
	{40, Script::checkDroppedDesc},
	{42, Script::doorClose},
	{44, Script::doorOpen},
	{45, Script::npcWait},
	{47, Script::displayMessage},
	{48, Script::setNewSupportData},
	{49, Script::setSupportData},
	{50, Script::givePlayerItem},
	{51, Script::decreaseNumGroats},
	{53, Script::moveCharacterToPlayer},
	{54, Script::setVillageSkorlTickProc},
	{55, Script::freeGoewin},
	{56, Script::barmanServe},
	{57, Script::getNumGroats},
	{59, Script::enableGargoylesTalk},
	{61, Script::killPlayer},
	{62, Script::animationLoad},
	{63, Script::addActions},
	{64, Script::randomToGeneral},
	{65, Script::checkCellDoor},
	{66, Script::method66},
	{0xff, NULL}};

const char *scriptOpcodes[] = {
	"ABORT", "ADD", "SUBTRACT", "MULTIPLY", "DIVIDE", "EQUALS", "NOT_EQUALS",
		"LT", "GT", "LTE", "GTE", "AND", "OR", "LOGICAL_AND", "LOGICAL_OR",
		"GET_FIELD", "SET_FIELD", "PUSH", "SUBROUTINE", "EXEC", "END",
		"COND_JUMP", "JUMP", "ABORT2", "ABORT3", "RANDOM"
};

const char *scriptMethodNames[67] = {
	"ACTIVATE HOTSPOT", "SET HOTSPOT SCRIPT", NULL, "SET HOTSPOT FLAG MASK", 
	"CLEAR SEQUENCE DELAY LIST", "DEACTIVATE HOTSPOT SET", "DEACTIVATE HOTSPOT", 
	"RESET PATHFINDER",	"ADD DELAYED SCRIPT", NULL,

	"IS CHARACTER IN ROOM", "SET HOTSPOT DESC", "SET HOTSPOT NAME",
	"PLAY SOUND", NULL, NULL, "DISPLAY DIALOG", NULL, "REMOTE ROOM VIEW SETUP",
	"SET CHAR SPEAKING TO ITSELF",

	"CHECK CELL DOOR", "PLAY MUSIC", "IS DOOR BLOCKED", "IS SKORL IN CELL",
	"PUSH BRICKS", "CHARACTER CHANGE ROOM", "PAUSE RATPOUCH", "SET BLOCKING HOTSPOT SCRIPT", 
	"DECREMENT # INVENTORY ITEMS", "SET TALKING",

	"SET ACTION CTR", "START SPEAKING", "DISABLE HOTSPOT", "CUT SACK",
	"INCREASE # GROATS", "ENABLE HOTSPOT", "DISPLAY MESSAGE 2", "START OIL BURNER"
	"TRANSFORM PLAYER", "JAIL CLOSE",

	"CHECK DROPPED DESC", NULL, "CLOSE DOOR", NULL, "OPEN DOOR", "NPC WAIT", NULL,
	"DISPLAY MESSAGE", "SET NEW ACTION SUPPORT DATA", "SET ACTION SUPPORT DATA",
	
	"GIVE PLAYER ITEM", "DECREASE # GROATS", NULL, NULL,
	"SET VILLAGE SKORL TICK PROC", "FREE GOEWIN", "BARMAN SERVE", "GET # GROATS", 
	NULL, "ENABLE GARGOYLE TALK",

	NULL, "KILL PLAYER", "ANIMATION LOAD", "ADD ACTIONS", "RANDOM TO GENERAL", 
	"CHECK CELL DOOR", "METHOD 66"
};


/*------------------------------------------------------------------------*/
/*-  Script Execution                                                    -*/
/*-                                                                      -*/
/*------------------------------------------------------------------------*/

uint16 Script::execute(uint16 startOffset) {
	Resources &r = Resources::getReference();
	ValueTableData &fields = r.fieldList();
	MemoryBlock *scriptData = r.scriptData();
	byte *scripts = scriptData->data();
	Common::Stack<uint16> stack;
	Common::Stack<uint16> methodStack;
	byte opcode;
	bool hasParam;
	uint16 param, v1, v2;
	uint16 param1, param2, param3;
	uint16 fieldNum;
	uint32 tempVal;
	SequenceMethodPtr ptr;
	SequenceMethodRecord *rec;
	char debugInfo[MAX_DESC_SIZE];

	uint16 offset = startOffset;
	bool breakFlag = false;

	param = 0;
	fields.setField(SEQUENCE_RESULT, 0);

	debugC(ERROR_BASIC, kLureDebugScripts, "Executing script %xh", startOffset);
	strcpy(debugInfo, "");

	while (!breakFlag) {
		if (offset >= scriptData->size()) 
			error("Script failure in script %d - invalid offset %d", startOffset, offset);

		if (gDebugLevel >= ERROR_DETAILED)
			sprintf(debugInfo, "%xh - ", offset);

		// Get opcode byte and separate into opcode and has parameter bit flag
		opcode = scripts[offset++];
		hasParam = (opcode & 1) != 0;
		opcode >>= 1;

		if (gDebugLevel >= ERROR_DETAILED)
			strcat(debugInfo, (opcode > S_OPCODE_RANDOM) ? "INVALID" :
				scriptOpcodes[opcode]);

		if (hasParam) {
			// Flag to read next two bytes as active parameter
			if (offset >= scriptData->size()-2) 
				error("Script failure in script %d - invalid offset %d", startOffset, offset);

			param = READ_LE_UINT16(scripts + offset);
			offset += 2;

			if (gDebugLevel >= ERROR_DETAILED)
				sprintf(debugInfo + strlen(debugInfo), " [%d]", 
					((opcode == S_OPCODE_GET_FIELD) || (opcode == S_OPCODE_SET_FIELD)) ?
					param >> 1 : param);
		}

		if (gDebugLevel >= ERROR_DETAILED) {
			switch (opcode) {
			case S_OPCODE_ADD:
			case S_OPCODE_SUBTRACT:
			case S_OPCODE_MULTIPLY:
			case S_OPCODE_DIVIDE:
			case S_OPCODE_EQUALS:
			case S_OPCODE_NOT_EQUALS:
			case S_OPCODE_LT:
			case S_OPCODE_GT:
			case S_OPCODE_LTE:
			case S_OPCODE_GTE:
			case S_OPCODE_AND:
			case S_OPCODE_OR:
			case S_OPCODE_LOGICAL_AND:
			case S_OPCODE_LOGICAL_OR:
				sprintf(debugInfo + strlen(debugInfo), 
					" %d, %d", stack[stack.size() - 1], stack[stack.size() - 2]);
				break;
			
			case S_OPCODE_SET_FIELD:
				sprintf(debugInfo + strlen(debugInfo), " <= ST (%d)", stack[stack.size() - 1]);
				break;

			default:
				break;
			}
		}

		switch (opcode) {
		case S_OPCODE_ABORT:
		case S_OPCODE_ABORT2:
		case S_OPCODE_ABORT3:
			methodStack.clear();
			break;

		case S_OPCODE_ADD:
			stack.push(stack.pop() + stack.pop());
			break;

		case S_OPCODE_SUBTRACT:
			v1 = stack.pop();
			v2 = stack.pop();
			stack.push(v2 - v1);
			break;

		case S_OPCODE_MULTIPLY:
			tempVal = stack.pop() * stack.pop();
			stack.push(tempVal & 0xffff);
			param = (uint16) (tempVal >> 16);
			break;

		case S_OPCODE_DIVIDE:
			v1 = stack.pop();
			v2 = stack.pop();
			stack.push(v2 / v1);
			param = v2 % v1;      // remainder
			break;

		case S_OPCODE_EQUALS:
			stack.push((stack.pop() == stack.pop()) ? 1 : 0);
			break;

		case S_OPCODE_NOT_EQUALS:
			stack.push((stack.pop() != stack.pop()) ? 1 : 0);
			break;

		case S_OPCODE_LT:
			v1 = stack.pop();
			v2 = stack.pop();
			stack.push(v2 < v1 ? 1 : 0);
			break;

		case S_OPCODE_GT:
			v1 = stack.pop();
			v2 = stack.pop();
			stack.push(v2 > v1 ? 1 : 0);
			break;

		case S_OPCODE_LTE:
			v1 = stack.pop();
			v2 = stack.pop();
			stack.push(v2 <= v1 ? 1 : 0);
			break;

		case S_OPCODE_GTE:
			v1 = stack.pop();
			v2 = stack.pop();
			stack.push(v2 >= v1 ? 1 : 0);
			break;

		case S_OPCODE_AND:
			stack.push(stack.pop() & stack.pop());
			break;

		case S_OPCODE_OR:
			stack.push(stack.pop() | stack.pop());
			break;

		case S_OPCODE_LOGICAL_AND:
			v1 = stack.pop();
			v2 = stack.pop();
			stack.push(((v1 != 0) && (v2 != 0)) ? 1 : 0);
			break;

		case S_OPCODE_LOGICAL_OR:
			v1 = stack.pop();
			v2 = stack.pop();
			stack.push(((v1 != 0) || (v2 != 0)) ? 1 : 0);
			break;

		case S_OPCODE_GET_FIELD:
			fieldNum = param >> 1;
			v1 = fields.getField(fieldNum);
			stack.push(v1);
			break;

		case S_OPCODE_SET_FIELD:
			fieldNum = param >> 1;
			v1 = stack.pop();
			fields.setField(fieldNum, v1);
			break;

		case S_OPCODE_PUSH:
			stack.push(param);
			break;

		case S_OPCODE_SUBROUTINE:
			methodStack.push(offset);
			offset = param;
			break;

		case S_OPCODE_EXEC:
			// Search for the method in the method list
			rec = &scriptMethods[0];
			while ((rec->methodIndex != 0xff) && (rec->methodIndex != param))
				++rec;

			if (gDebugLevel >= ERROR_DETAILED) {
				// Set up the debug string for the method call
				if (rec->methodIndex == 0xff) strcat(debugInfo, " INVALID INDEX");
				else if (scriptMethodNames[param] == NULL) strcat(debugInfo, " UNKNOWN METHOD");
				else {
					strcat(debugInfo, " ");
					strcat(debugInfo, scriptMethodNames[param]);
				}

				// Any params
				if (stack.size() >= 3)
					sprintf(debugInfo + strlen(debugInfo), " (%d,%d,%d)",
						stack[stack.size()-1], stack[stack.size()-2], stack[stack.size()-3]);
				if (stack.size() == 2)
					sprintf(debugInfo + strlen(debugInfo), " (%d,%d)",
						stack[stack.size()-1], stack[stack.size()-2]);
				else if (stack.size() == 1)
					sprintf(debugInfo + strlen(debugInfo), " (%d)", stack[stack.size()-1]);
				strcat(debugInfo, ")");
				
				debugC(ERROR_DETAILED, kLureDebugScripts, debugInfo);
			}

			param1 = 0; param2 = 0; param3 = 0;
			if (!stack.empty()) param1 = stack.pop();
			if (!stack.empty()) param2 = stack.pop();
			if (!stack.empty()) param3 = stack.pop();

			if (rec->methodIndex == 0xff) 
				warning("Undefined script method %d", param);
			else {
				ptr = rec->proc;
				ptr(param1, param2, param3);
			}
			break;

		case S_OPCODE_COND_JUMP:
			v1 = stack.pop();
			if (v1 == 0) offset += (int16) param;
			break;

		case S_OPCODE_JUMP:
			offset += (int16) param;
			break;

		case S_OPCODE_RANDOM:
			param = r.random() >> 8; // make number between 0 to 255
			break;

		case S_OPCODE_END:
			// Signal to end the execution
			if (!methodStack.empty()) 
				offset = methodStack.pop();
			else
				breakFlag = true;
			break;

		default:
			error("Unknown script opcode %d", opcode);
			break;
		}

		if ((gDebugLevel >= ERROR_DETAILED) && (opcode != S_OPCODE_EXEC)) {
			switch (opcode) {
			case S_OPCODE_ADD:
			case S_OPCODE_SUBTRACT:
			case S_OPCODE_MULTIPLY:
			case S_OPCODE_DIVIDE:
			case S_OPCODE_NOT_EQUALS:
			case S_OPCODE_EQUALS:
			case S_OPCODE_GT:
			case S_OPCODE_LT:
			case S_OPCODE_LTE:
			case S_OPCODE_GTE:
			case S_OPCODE_AND:
			case S_OPCODE_OR:
			case S_OPCODE_LOGICAL_AND:
			case S_OPCODE_LOGICAL_OR:
			case S_OPCODE_GET_FIELD:
				sprintf(debugInfo + strlen(debugInfo), " => ST (%d)", stack[stack.size()-1]);
				break;

			case S_OPCODE_PUSH:
				strcat(debugInfo, " => ST");
				break;

			default:
				break;
			}

			debugC(ERROR_DETAILED, kLureDebugScripts, debugInfo);
		}
	}

	uint16 result = fields.getField(SEQUENCE_RESULT);
	debugC(ERROR_DETAILED, kLureDebugScripts, "Script result = %d", result); 
	return result;
}

/*------------------------------------------------------------------------*/
/*-  Hotspot Script Handler                                              -*/
/*-                                                                      -*/
/*------------------------------------------------------------------------*/

int16 HotspotScript::nextVal(MemoryBlock *data, uint16 &offset) {
	if (offset >= data->size() - 1) 
		error("Script failure - invalid offset");
	int16 value = READ_LE_UINT16(data->data() + offset);
	offset += 2;
	return value;
}

bool HotspotScript::execute(Hotspot *h)
{
	Resources &r = Resources::getReference();
	MemoryBlock *scriptData = r.hotspotScriptData();
	uint16 offset = h->hotspotScript();
	int16 opcode = 0;
	int16 param1, param2;
	uint32 actions;
	bool breakFlag = false;

	debugC(ERROR_BASIC, kLureDebugScripts, "Executing hotspot %xh script pos=%xh", 
		h->hotspotId(), offset);

	while (!breakFlag) {
		opcode = nextVal(scriptData, offset);

		switch (opcode) {
		case S2_OPCODE_FRAME_CTR:
			param1 = nextVal(scriptData, offset);
			debugC(ERROR_DETAILED, kLureDebugScripts, "SET FRAME_CTR = %d", param1);

			h->setTickCtr(param1);
			h->setHotspotScript(offset);
			breakFlag = true;
			break;

		case S2_OPCODE_POSITION:
			param1 = nextVal(scriptData, offset);
			param2 = nextVal(scriptData, offset);
			debugC(ERROR_DETAILED, kLureDebugScripts, "SET POSITION = (%d,%d)", 
				param1 - 0x80, param2 - 0x80);

			h->setPosition(param1 - 0x80, param2 - 0x80);
			break;
			
		case S2_OPCODE_CHANGE_POS:
			param1 = nextVal(scriptData, offset);
			param2 = nextVal(scriptData, offset);
			debugC(ERROR_DETAILED, kLureDebugScripts, "CHANGE POSITION BY = (%d,%d)", 
				param1, param2);

			h->setPosition(h->x() + param1, h->y() + param2);
			break;

		case S2_OPCODE_UNLOAD:
			debugC(ERROR_DETAILED, kLureDebugScripts, "UNLOAD HOTSPOT");
			breakFlag = true;
			break;

		case S2_OPCODE_DIMENSIONS:
			param1 = nextVal(scriptData, offset) << 4;
			param2 = nextVal(scriptData, offset);
			debugC(ERROR_DETAILED, kLureDebugScripts, "SET SIZE = (%d,%d)",
				param1, param2);

			h->setSize((uint16) param1, (uint16) param2);
			break;

		case S2_OPCODE_JUMP:
			offset = (uint16) nextVal(scriptData, offset);
			debugC(ERROR_DETAILED, kLureDebugScripts, "JUMP OFFSET = %xh", offset);
			break;

		case S2_OPCODE_ANIMATION:
			param1 = nextVal(scriptData, offset);
			debugC(ERROR_DETAILED, kLureDebugScripts, "SET ANIMATION ID = %xh", param1);

			h->setAnimation(param1);
			break;

		case S2_OPCODE_PLAY_SOUND:
			param1 = nextVal(scriptData, offset);
			param2 = nextVal(scriptData, offset);
			debugC(ERROR_DETAILED, kLureDebugScripts, "PLAY_SOUND(%d,%d)", param1, param2);
//			warning("UNKNOWN_247 stub called");
			break;

		case S2_OPCODE_STOP_SOUND:
			param1 = nextVal(scriptData, offset);
			debugC(ERROR_DETAILED, kLureDebugScripts, "STOP_SOUND()");
//			warning("UNKNOWN_258 stub called");
			break;

		case S2_OPCODE_ACTIONS:
			param1 = nextVal(scriptData, offset);
			param2 = nextVal(scriptData, offset);
			actions = (uint32) param1 | ((uint32) param2 << 16);

			debugC(ERROR_DETAILED, kLureDebugScripts, "SET ACTIONS = %xh", actions);
			h->setActions(actions);
			break;

		default:
			// Set the animation frame number
			debugC(ERROR_DETAILED, kLureDebugScripts, "SET FRAME NUMBER = %d", opcode);

			h->setFrameNumber(opcode);
			h->setHotspotScript(offset);
			breakFlag = true;
			break;
		}
	}

	return (opcode == S2_OPCODE_UNLOAD);
}

} // end of namespace Lure
