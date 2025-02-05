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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/engines/saga/events.cpp $
 * $Id: events.cpp 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

// Event management module

#include "saga/saga.h"
#include "saga/gfx.h"

#include "saga/animation.h"
#include "saga/console.h"
#include "saga/scene.h"
#include "saga/interface.h"
#include "saga/palanim.h"
#include "saga/render.h"
#include "saga/sndres.h"
#include "saga/music.h"
#include "saga/actor.h"

#include "saga/events.h"

namespace Saga {

Events::Events(SagaEngine *vm) : _vm(vm), _initialized(false) {
	debug(8, "Initializing event subsystem...");
	_initialized = true;
}

Events::~Events(void) {
	debug(8, "Shutting down event subsystem...");
	freeList();
}

// Function to process event list once per frame.
// First advances event times, then processes each event with the appropriate
// handler depending on the type of event.
int Events::handleEvents(long msec) {
	Event *event_p;

	long delta_time;
	int result;

	// Advance event times
	processEventTime(msec);

	// Process each event in list
	for (EventList::iterator eventi = _eventList.begin(); eventi != _eventList.end(); ++eventi) {
		event_p = (Event *)eventi.operator->();

		// Call the appropriate event handler for the specific event type
		switch (event_p->type) {

		case kEvTOneshot:
			result = handleOneShot(event_p);
			break;

		case kEvTContinuous:
			result = handleContinuous(event_p);
			break;

		case kEvTInterval:
			result = handleInterval(event_p);
			break;

		case kEvTImmediate:
			result = handleImmediate(event_p);
			break;

		default:
			result = kEvStInvalidCode;
			warning("Invalid event code encountered");
			break;
		}

		// Process the event appropriately based on result code from
		// handler
		if ((result == kEvStDelete) || (result == kEvStInvalidCode)) {
			// If there is no event chain, delete the base event.
			if (event_p->chain == NULL) {
				eventi = _eventList.eraseAndPrev(eventi);
			} else {
				// If there is an event chain present, move the next event
				// in the chain up, adjust it by the previous delta time,
				// and reprocess the event
				delta_time = event_p->time;
				Event *from_chain = event_p->chain;
				memcpy(event_p, from_chain, sizeof(*event_p));
				free(from_chain);

				event_p->time += delta_time;
				--eventi;
			}
		} else if (result == kEvStBreak) {
			break;
		}
	}

	return SUCCESS;
}

int Events::handleContinuous(Event *event) {
	double event_pc = 0.0; // Event completion percentage
	int event_done = 0;

	Surface *backGroundSurface;
	BGInfo bgInfo;
	Rect rect;
	if (event->duration != 0) {
		event_pc = ((double)event->duration - event->time) / event->duration;
	} else {
		event_pc = 1.0;
	}

	if (event_pc >= 1.0) {
		// Cap percentage to 100
		event_pc = 1.0;
		event_done = 1;
	}

	if (event_pc < 0.0) {
		// Event not signaled, skip it
		return kEvStContinue;
	} else if (!(event->code & kEvFSignaled)) {
		// Signal event
		event->code |= kEvFSignaled;
		event_pc = 0.0;
	}

	switch (event->code & EVENT_MASK) {
	case kPalEvent:
		switch (event->op) {
		case kEventBlackToPal:
			_vm->_gfx->blackToPal((PalEntry *)event->data, event_pc);
			break;

		case kEventPalToBlack:
			_vm->_gfx->palToBlack((PalEntry *)event->data, event_pc);
			break;
		default:
			break;
		}
		break;
	case kTransitionEvent:
		switch (event->op) {
		case kEventDissolve:
			backGroundSurface = _vm->_render->getBackGroundSurface();
			_vm->_scene->getBGInfo(bgInfo);
			rect.left = rect.top = 0;
			rect.right = bgInfo.bounds.width();
			rect.bottom = bgInfo.bounds.height();
			backGroundSurface->transitionDissolve(bgInfo.buffer, rect, 0, event_pc);
			break;
		case kEventDissolveBGMask:
			// we dissolve it centered.
			// set flag of Dissolve to 1. It is a hack to simulate zero masking.
			int w, h;
			byte *maskBuffer;
			size_t len;

			backGroundSurface = _vm->_render->getBackGroundSurface();
			_vm->_scene->getBGMaskInfo(w, h, maskBuffer, len);
			rect.left = (_vm->getDisplayWidth() - w) / 2;
			rect.top = (_vm->getDisplayHeight() - h) / 2;
			rect.setWidth(w);
			rect.setHeight(h);

			backGroundSurface->transitionDissolve( maskBuffer, rect, 1, event_pc);
			break;
		default:
			break;
		}
		break;
	default:
		break;

	}

	if (event_done) {
		return kEvStDelete;
	}

	return kEvStContinue;
}

int Events::handleImmediate(Event *event) {
	double event_pc = 0.0; // Event completion percentage
	bool event_done = false;

	// Duration might be 0 so dont do division then
	if (event->duration != 0) {
		event_pc = ((double)event->duration - event->time) / event->duration;
	} else {
		// Just make sure that event_pc is 1.0 so event_done is true
		event_pc = 1.0;
	}

	if (event_pc >= 1.0) {
		// Cap percentage to 100
		event_pc = 1.0;
		event_done = true;
	}

	if (event_pc < 0.0) {
		// Event not signaled, skip it
		return kEvStBreak;
	} else if (!(event->code & kEvFSignaled)) {
		// Signal event
		event->code |= kEvFSignaled;
		event_pc = 0.0;
	}

	switch (event->code & EVENT_MASK) {
	case kPalEvent:
		switch (event->op) {
		case kEventBlackToPal:
			_vm->_gfx->blackToPal((PalEntry *)event->data, event_pc);
			break;

		case kEventPalToBlack:
			_vm->_gfx->palToBlack((PalEntry *)event->data, event_pc);
			break;
		default:
			break;
		}
		break;
	case kScriptEvent:
	case kBgEvent:
	case kInterfaceEvent:
		handleOneShot(event);
		event_done = true;
		break;
	default:
		break;

	}

	if (event_done) {
		return kEvStDelete;
	}

	return kEvStBreak;
}

int Events::handleOneShot(Event *event) {
	Surface *backBuffer;
	ScriptThread *sthread;
	Rect rect;


	if (event->time > 0) {
		return kEvStContinue;
	}

	// Event has been signaled

	switch (event->code & EVENT_MASK) {
	case kTextEvent:
		switch (event->op) {
		case kEventDisplay:
			((TextListEntry *)event->data)->display = true;
			break;
		case kEventRemove:
			_vm->_scene->_textList.remove((TextListEntry *)event->data);
			break;
		default:
			break;
		}

		break;
	case kSoundEvent:
		_vm->_sound->stopSound();
		if (event->op == kEventPlay)
			_vm->_sndRes->playSound(event->param, event->param2, event->param3 != 0);
		break;
	case kVoiceEvent:
		_vm->_sndRes->playVoice(event->param);
		break;
	case kMusicEvent:
		_vm->_music->stop();
		if (event->op == kEventPlay)
			_vm->_music->play(event->param, (MusicFlags)event->param2);
		break;
	case kBgEvent:
		{
			Surface *backGroundSurface;
			BGInfo bgInfo;

			if (!(_vm->_scene->getFlags() & kSceneFlagISO)) {

				backBuffer = _vm->_gfx->getBackBuffer();
				backGroundSurface = _vm->_render->getBackGroundSurface();
				_vm->_scene->getBGInfo(bgInfo);

				backGroundSurface->blit(bgInfo.bounds, bgInfo.buffer);

				// If it is inset scene then draw black border
				if (bgInfo.bounds.width() < _vm->getDisplayWidth() || bgInfo.bounds.height() < _vm->_scene->getHeight()) {
					Common::Rect rect1(2, bgInfo.bounds.height() + 4);
					Common::Rect rect2(bgInfo.bounds.width() + 4, 2);
					Common::Rect rect3(2, bgInfo.bounds.height() + 4);
					Common::Rect rect4(bgInfo.bounds.width() + 4, 2);
					rect1.moveTo(bgInfo.bounds.left - 2, bgInfo.bounds.top - 2);
					rect2.moveTo(bgInfo.bounds.left - 2, bgInfo.bounds.top - 2);
					rect3.moveTo(bgInfo.bounds.right, bgInfo.bounds.top - 2);
					rect4.moveTo(bgInfo.bounds.left - 2, bgInfo.bounds.bottom);

					backGroundSurface->drawRect(rect1, kITEColorBlack);
					backGroundSurface->drawRect(rect2, kITEColorBlack);
					backGroundSurface->drawRect(rect3, kITEColorBlack);
					backGroundSurface->drawRect(rect4, kITEColorBlack);
				}

				if (event->param == kEvPSetPalette) {
					PalEntry *palPointer;
					_vm->_scene->getBGPal(palPointer);
					_vm->_gfx->setPalette(palPointer);
				}
			}
		}
		break;
	case kAnimEvent:
		switch (event->op) {
		case kEventPlay:
			_vm->_anim->play(event->param, event->time, true);
			break;
		case kEventStop:
			_vm->_anim->stop(event->param);
			break;
		case kEventFrame:
			_vm->_anim->play(event->param, event->time, false);
			break;
		case kEventSetFlag:
			_vm->_anim->setFlag(event->param, event->param2);
			break;
		case kEventClearFlag:
			_vm->_anim->clearFlag(event->param, event->param2);
			break;
		default:
			break;
		}
		break;
	case kSceneEvent:
		switch (event->op) {
		case kEventEnd:
			_vm->_scene->nextScene();
			return kEvStBreak;
		default:
			break;
		}
		break;
	case kPalAnimEvent:
		switch (event->op) {
		case kEventCycleStart:
			_vm->_palanim->cycleStart();
			break;
		case kEventCycleStep:
			_vm->_palanim->cycleStep(event->time);
			break;
		default:
			break;
		}
		break;
	case kInterfaceEvent:
		switch (event->op) {
		case kEventActivate:
			_vm->_interface->activate();
			break;
		case kEventDeactivate:
			_vm->_interface->deactivate();
			break;
		case kEventSetStatus:
			_vm->_interface->setStatusText((const char*)event->data);
			_vm->_interface->drawStatusBar();
			break;
		case kEventClearStatus:
			_vm->_interface->setStatusText("");
			_vm->_interface->drawStatusBar();
			break;
		case kEventSetFadeMode:
			_vm->_interface->setFadeMode(event->param);
			break;
		default:
			break;
		}
		break;
	case kScriptEvent:
		switch (event->op) {
		case kEventExecBlocking:
		case kEventExecNonBlocking:
			debug(6, "Exec module number %ld script entry number %ld", event->param, event->param2);

			sthread = _vm->_script->createThread(event->param, event->param2);
			if (sthread == NULL) {
				_vm->_console->DebugPrintf("Thread creation failed.\n");
				break;
			}

			sthread->_threadVars[kThreadVarAction] = event->param3;
			sthread->_threadVars[kThreadVarObject] = event->param4;
			sthread->_threadVars[kThreadVarWithObject] = event->param5;
			sthread->_threadVars[kThreadVarActor] = event->param6;

			if (event->op == kEventExecBlocking)
				_vm->_script->completeThread();

			break;
		case kEventThreadWake:
			_vm->_script->wakeUpThreads(event->param);
			break;
		}
		break;
	case kCursorEvent:
		switch (event->op) {
		case kEventShow:
			_vm->_gfx->showCursor(true);
			break;
		case kEventHide:
			_vm->_gfx->showCursor(false);
			break;
		case kEventSetNormalCursor:
			// in ITE there is just one cursor
			if (_vm->getGameType() == GType_IHNM)
				_vm->_gfx->setCursor(kCursorNormal);
			break;
		case kEventSetBusyCursor:
			if (_vm->getGameType() == GType_IHNM)
				_vm->_gfx->setCursor(kCursorBusy);
			break;
		default:
			break;
		}
		break;
	case kGraphicsEvent:
		switch (event->op) {
		case kEventFillRect:
			rect.top = event->param2;
			rect.bottom = event->param3;
			rect.left = event->param4;
			rect.right = event->param5;
			((Surface *)event->data)->drawRect(rect, event->param);
			break;
		case kEventSetFlag:
			_vm->_render->setFlag(event->param);
			break;
		case kEventClearFlag:
			_vm->_render->clearFlag(event->param);
			break;
		default:
			break;
		}
	default:
		break;
	}

	return kEvStDelete;
}

int Events::handleInterval(Event *event) {
	return kEvStDelete;
}

// Schedules an event in the event list; returns a pointer to the scheduled
// event suitable for chaining if desired.
Event *Events::queue(Event *event) {
	Event *queuedEvent;

	queuedEvent = _eventList.pushBack(*event).operator->();
	initializeEvent(queuedEvent);

	return queuedEvent;
}

// Places a 'add_event' on the end of an event chain given by 'head_event'
// (head_event may be in any position in the event chain)
Event *Events::chain(Event *headEvent, Event *addEvent) {
	if (headEvent == NULL) {
		return queue(addEvent);
	}

	Event *walkEvent;
	for (walkEvent = headEvent; walkEvent->chain != NULL; walkEvent = walkEvent->chain) {
		continue;
	}

	walkEvent->chain = (Event *)malloc(sizeof(*walkEvent->chain));
	*walkEvent->chain = *addEvent;
	initializeEvent(walkEvent->chain);

	return walkEvent->chain;
}

int Events::initializeEvent(Event *event) {
	event->chain = NULL;
	switch (event->type) {
	case kEvTOneshot:
		break;
	case kEvTContinuous:
	case kEvTImmediate:
		event->time += event->duration;
		break;
	case kEvTInterval:
		break;
	default:
		return FAILURE;
	}

	return SUCCESS;
}

int Events::clearList() {
	Event *chain_walk;
	Event *next_chain;
	Event *event_p;

	// Walk down event list
	for (EventList::iterator eventi = _eventList.begin(); eventi != _eventList.end(); ++eventi) {
		event_p = (Event *)eventi.operator->();

		// Only remove events not marked kEvFNoDestory (engine events)
		if (!(event_p->code & kEvFNoDestory)) {
			// Remove any events chained off this one */
			for (chain_walk = event_p->chain; chain_walk != NULL; chain_walk = next_chain) {
				next_chain = chain_walk->chain;
				free(chain_walk);
			}
			eventi = _eventList.eraseAndPrev(eventi);
		}
	}

	return SUCCESS;
}

// Removes all events from the list (even kEvFNoDestory)
int Events::freeList() {
	Event *chain_walk;
	Event *next_chain;
	Event *event_p;

	// Walk down event list
	EventList::iterator eventi = _eventList.begin();
	while (eventi != _eventList.end()) {
		event_p = (Event *)eventi.operator->();

		// Remove any events chained off this one */
		for (chain_walk = event_p->chain; chain_walk != NULL; chain_walk = next_chain) {
			next_chain = chain_walk->chain;
			free(chain_walk);
		}
		eventi=_eventList.erase(eventi);
	}

	return SUCCESS;
}

// Walks down the event list, updating event times by 'msec'.
int Events::processEventTime(long msec) {
	Event *event_p;
	uint16 event_count = 0;

	for (EventList::iterator eventi = _eventList.begin(); eventi != _eventList.end(); ++eventi) {
		event_p = (Event *)eventi.operator->();

		event_p->time -= msec;
		event_count++;

		if (event_p->type == kEvTImmediate)
			break;

		if (event_count > EVENT_WARNINGCOUNT) {
			warning("Event list exceeds %u", EVENT_WARNINGCOUNT);
		}
	}

	return SUCCESS;
}

} // End of namespace Saga
