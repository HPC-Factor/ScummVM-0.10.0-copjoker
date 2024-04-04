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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/engines/saga/events.h $
 * $Id: events.h 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

// Event management module header file

#ifndef SAGA_EVENT_H
#define SAGA_EVENT_H

#include "saga/list.h"

namespace Saga {

enum EventTypes {
	kEvTOneshot,    // Event takes no time
	kEvTContinuous, // Event takes time; next event starts immediately
	kEvTInterval,   // Not yet implemented
	kEvTImmediate   // Event takes time; next event starts when event is done
};

enum EventFlags {
	kEvFSignaled = 0x8000,
	kEvFNoDestory = 0x4000
};

enum EventCodes {
	kBgEvent = 1,
	kAnimEvent,
	kMusicEvent,
	kVoiceEvent,
	kSoundEvent,
	kSceneEvent,
	kTextEvent,
	kPalEvent,
	kPalAnimEvent,
	kTransitionEvent,
	kInterfaceEvent,
	kActorEvent,
	kScriptEvent,
	kCursorEvent,
	kGraphicsEvent
};

enum EventOps {
	// INSTANTANEOUS events
	// BG events
	kEventDisplay = 1,
	// ANIM events
	// kEventPlay = 1, // reused
	// kEventStop = 2, // reused
	kEventFrame = 3,
	kEventSetFlag = 4,
	kEventClearFlag = 5,
	// MUISC & SOUND events
	kEventPlay = 1,
	kEventStop = 2,
	// SCENE events
	kEventEnd = 2,
	// TEXT events
	kEventHide = 2,
	kEventRemove = 3,
	// PALANIM events
	kEventCycleStart = 1,
	kEventCycleStep = 2,
	// INTERFACE events
	kEventActivate = 1,
	kEventDeactivate = 2,
	kEventSetStatus = 3,
	kEventClearStatus = 4,
	kEventSetFadeMode = 5,
	// ACTOR events
	kEventMove = 1,
	// SCRIPT events
	kEventExecBlocking = 1,
	kEventExecNonBlocking = 2,
	kEventThreadWake = 3,
	// CURSOR events
	kEventShow = 1,
	// kEventHide = 2, // reused
	kEventSetNormalCursor = 3,
	kEventSetBusyCursor = 4,
	// GRAPHICS events
	kEventFillRect = 1,
	// kEventSetFlag = 4, // reused
	// kEventClearFlag = 5, // reused

	// CONTINUOUS events
	// PALETTE events
	kEventPalToBlack = 1,
	kEventBlackToPal = 2,
	// TRANSITION events
	kEventDissolve = 1,
	kEventDissolveBGMask = 2
};

enum EventParams {
	kEvPNoSetPalette,
	kEvPSetPalette
};

struct Event {
	unsigned int type;
	unsigned int code; // Event operation category & flags
	int op;            // Event operation
	long param;        // Optional event parameter
	long param2;
	long param3;
	long param4;
	long param5;
	long param6;
	void *data;        // Optional event data
	long time;         // Elapsed time until event
	long duration;     // Duration of event
	long d_reserved;

	Event *chain;    // Event chain (For consecutive events)
	Event() {
		memset(this, 0, sizeof(*this));
	}
};

typedef SortedList<Event> EventList;

#define EVENT_WARNINGCOUNT 1000
#define EVENT_MASK 0x00FF

enum EventStatusCode {
	kEvStInvalidCode = 0,
	kEvStDelete,
	kEvStContinue,
	kEvStBreak
};

class Events {
 public:
	Events(SagaEngine *vm);
	~Events(void);
	int handleEvents(long msec);
	int clearList();
	int freeList();
	Event *queue(Event *event);
	Event *chain(Event *headEvent, Event *addEvent);

 private:
	int handleContinuous(Event *event);
	int handleOneShot(Event *event);
	int handleInterval(Event *event);
	int handleImmediate(Event *event);
	int processEventTime(long msec);
	int initializeEvent(Event *event);

 private:
	SagaEngine *_vm;
	bool _initialized;

	EventList _eventList;
};

} // End of namespace Saga

#endif
