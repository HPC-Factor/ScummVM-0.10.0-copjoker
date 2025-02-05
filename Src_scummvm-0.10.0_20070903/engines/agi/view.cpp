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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/engines/agi/view.cpp $
 * $Id: view.cpp 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

#include "agi/agi.h"
#include "agi/sprite.h"

namespace Agi {

void AgiEngine::lSetCel(VtEntry *v, int n) {
	ViewLoop *currentVl;
	ViewCel *currentVc;

	v->currentCel = n;

	currentVl = &_game.views[v->currentView].loop[v->currentLoop];

	/* Added by Amit Vainsencher <amitv@subdimension.com> to prevent
	 * crash in KQ1 -- not in the Sierra interpreter
	 */
	if (currentVl->numCels == 0)
		return;

	// WORKAROUND: This is a very nasty hack to fix a bug in the KQ4 introduction
	// In its original form, it caused a lot of regressions, including KQ4 bugs and crashes
	// Refer to Sarien bug #588899 for the original issue
	// Modifying this workaround to only work for a specific view in the KQ4 intro fixes several 
	// ScummVM bugs. Refer to bugs #1660486, #1660169, #1660192, #1660162 and #1660354
	// FIXME: Remove this workaround and investigate the reason for the erroneous actor behavior
	// in the KQ4 introduction
	// It seems there's either a bug with KQ4's logic script 120 (the intro script)
	// or flag 64 is not set correctly, which causes the erroneous behavior from the actors
	if (getGameID() == GID_KQ4 && !(v->flags & UPDATE) && (v->currentView == 172))
		return;

	currentVc = &currentVl->cel[n];
	v->celData = currentVc;
	v->xSize = currentVc->width;
	v->ySize = currentVc->height;
}

void AgiEngine::lSetLoop(VtEntry *v, int n) {
	ViewLoop *currentVl;
	debugC(7, kDebugLevelResources, "vt entry #%d, loop = %d", v->entry, n);

	/* Added to avoid crash when leaving the arcade machine in MH1
	 * -- not in AGI 2.917
	 */
	if (n >= v->numLoops)
		n = 0;

	v->currentLoop = n;
	currentVl = &_game.views[v->currentView].loop[v->currentLoop];

	v->numCels = currentVl->numCels;
	if (v->currentCel >= v->numCels)
		v->currentCel = 0;

	v->loopData = &_game.views[v->currentView].loop[n];
}

void AgiEngine::updateView(VtEntry *v) {
	int cel, lastCel;

	if (v->flags & DONTUPDATE) {
		v->flags &= ~DONTUPDATE;
		return;
	}

	cel = v->currentCel;
	lastCel = v->numCels - 1;

	switch (v->cycle) {
	case CYCLE_NORMAL:
		if (++cel > lastCel)
			cel = 0;
		break;
	case CYCLE_END_OF_LOOP:
		if (cel < lastCel) {
			debugC(5, kDebugLevelResources, "cel %d (last = %d)", cel + 1, lastCel);
			if (++cel != lastCel)
				break;
		}
		setflag(v->parm1, true);
		v->flags &= ~CYCLING;
		v->direction = 0;
		v->cycle = CYCLE_NORMAL;
		break;
	case CYCLE_REV_LOOP:
		if (cel) {
			if (--cel)
				break;
		}
		setflag(v->parm1, true);
		v->flags &= ~CYCLING;
		v->direction = 0;
		v->cycle = CYCLE_NORMAL;
		break;
	case CYCLE_REVERSE:
		if (cel == 0) {
			cel = lastCel;
		} else {
			cel--;
		}
		break;
	}

	setCel(v, cel);
}

/*
 * Public functions
 */

/**
 * Decode an AGI view resource.
 * This function decodes the raw data of the specified AGI view resource
 * and fills the corresponding views array element.
 * @param n number of view resource to decode
 */
int AgiEngine::decodeView(int n) {
	int loop, cel;
	uint8 *v, *lptr;
	uint16 lofs, cofs;
	ViewLoop *vl;
	ViewCel *vc;

	debugC(5, kDebugLevelResources, "decode_view(%d)", n);
	v = _game.views[n].rdata;

	assert(v != NULL);

	_game.views[n].descr = READ_LE_UINT16(v + 3) ? (char *)(v + READ_LE_UINT16(v + 3)) : (char *)(v + 3);

	/* if no loops exist, return! */
	if ((_game.views[n].numLoops = *(v + 2)) == 0)
		return errNoLoopsInView;

	/* allocate memory for all views */
	_game.views[n].loop = (ViewLoop *)
			calloc(_game.views[n].numLoops, sizeof(ViewLoop));

	if (_game.views[n].loop == NULL)
		return errNotEnoughMemory;

	/* decode all of the loops in this view */
	lptr = v + 5;		/* first loop address */

	for (loop = 0; loop < _game.views[n].numLoops; loop++, lptr += 2) {
		lofs = READ_LE_UINT16(lptr);	/* loop header offset */
		vl = &_game.views[n].loop[loop];	/* the loop struct */

		vl->numCels = *(v + lofs);
		debugC(6, kDebugLevelResources, "view %d, num_cels = %d", n, vl->numCels);
		vl->cel = (ViewCel *)calloc(vl->numCels, sizeof(ViewCel));
		if (vl->cel == NULL) {
			free(_game.views[n].loop);
			_game.views[n].numLoops = 0;
			return errNotEnoughMemory;
		}

		/* decode the cells */
		for (cel = 0; cel < vl->numCels; cel++) {
			cofs = lofs + READ_LE_UINT16(v + lofs + 1 + (cel * 2));
			vc = &vl->cel[cel];

			vc->width = *(v + cofs);
			vc->height = *(v + cofs + 1);
			vc->transparency = *(v + cofs + 2) & 0xf;
			vc->mirrorLoop = (*(v + cofs + 2) >> 4) & 0x7;
			vc->mirror = (*(v + cofs + 2) >> 7) & 0x1;

			/* skip over width/height/trans|mirror data */
			cofs += 3;

			vc->data = v + cofs;
			/* If mirror_loop is pointing to the current loop,
			 * then this is the original.
			 */
			if (vc->mirrorLoop == loop)
				vc->mirror = 0;
		}		/* cel */
	}			/* loop */

	return errOK;
}

/**
 * Unloads all data in a view resource
 * @param n number of view resource
 */
void AgiEngine::unloadView(int n) {
	int x;

	debugC(5, kDebugLevelResources, "discard view %d", n);
	if (~_game.dirView[n].flags & RES_LOADED)
		return;

	/* Rebuild sprite list, see Sarien bug #779302 */
	_sprites->eraseBoth();
	_sprites->blitBoth();
	_sprites->commitBoth();

	/* free all the loops */
	for (x = 0; x < _game.views[n].numLoops; x++)
		free(_game.views[n].loop[x].cel);

	free(_game.views[n].loop);
	free(_game.views[n].rdata);

	_game.dirView[n].flags &= ~RES_LOADED;
}

/**
 * Set a view table entry to use the specified cel of the current loop.
 * @param v pointer to view table entry
 * @param n number of cel
 */
void AgiEngine::setCel(VtEntry *v, int n) {
	assert(v->viewData != NULL);
	assert(v->numCels >= n);

	lSetCel(v, n);

	/* If position isn't appropriate, update it accordingly */
	if (v->xPos + v->xSize > _WIDTH) {
		v->flags |= UPDATE_POS;
		v->xPos = _WIDTH - v->xSize;
	}
	if (v->yPos - v->ySize + 1 < 0) {
		v->flags |= UPDATE_POS;
		v->yPos = v->ySize - 1;
	}
	if (v->yPos <= _game.horizon && (~v->flags & IGNORE_HORIZON)) {
		v->flags |= UPDATE_POS;
		v->yPos = _game.horizon + 1;
	}
}

/**
 * Set a view table entry to use the specified loop of the current view.
 * @param v pointer to view table entry
 * @param n number of loop
 */
void AgiEngine::setLoop(VtEntry *v, int n) {
	assert(v->viewData != NULL);
	assert(v->numLoops >= n);
	lSetLoop(v, n);
	setCel(v, v->currentCel);
}

/**
 * Set a view table entry to use the specified view resource.
 * @param v pointer to view table entry
 * @param n number of AGI view resource
 */
void AgiEngine::setView(VtEntry *v, int n) {

	uint16 viewFlags = 0;

	// When setting a view to the view table, if there's already another view set in that
	// view table entry and it's still drawn, erase the existing view before setting the new one
	// Fixes bug #1658643: AGI: SQ1 (2.2 DOS ENG) Graphic error, ego leaves behind copy
	// Update: Apparently, this makes ego dissapear at times, e.g. when textboxes are shown
	// Therefore, it's limited to view 118 in SQ1 (Roger climbing the ladder)
	// Fixes bug #1715284: Roger sometimes disappears
	if (v->viewData != NULL) {
		if (v->currentView == 118 && v->flags & DRAWN && getGameID() == GID_SQ1) {
			viewFlags = v->flags;			// Store the flags for the view
			_sprites->eraseUpdSprites();
			if (v->flags & UPDATE) {
				v->flags &= ~DRAWN;
			} else {
				_sprites->eraseNonupdSprites();
				v->flags &= ~DRAWN;
				_sprites->blitNonupdSprites();
			}
			_sprites->blitUpdSprites();

			_sprites->commitBlock(v->xPos, v->yPos - v->ySize + 1, v->xPos + v->xSize - 1, v->yPos);
			v->flags = viewFlags;			// Restore the view's flags
		}
	}

	v->viewData = &_game.views[n];
	v->currentView = n;
	v->numLoops = v->viewData->numLoops;
	setLoop(v, v->currentLoop >= v->numLoops ? 0 : v->currentLoop);
}

/**
 * Set the view table entry as updating.
 * @param v pointer to view table entry
 */
void AgiEngine::startUpdate(VtEntry *v) {
	if (~v->flags & UPDATE) {
		_sprites->eraseBoth();
		v->flags |= UPDATE;
		_sprites->blitBoth();
	}
}

/**
 * Set the view table entry as non-updating.
 * @param v pointer to view table entry
 */
void AgiEngine::stopUpdate(VtEntry *v) {
	if (v->flags & UPDATE) {
		_sprites->eraseBoth();
		v->flags &= ~UPDATE;
		_sprites->blitBoth();
	}
}

/* loops to use according to direction and number of loops in
 * the view resource
 */
static int loopTable2[] = {
	0x04, 0x04, 0x00, 0x00, 0x00, 0x04, 0x01, 0x01, 0x01
};

static int loopTable4[] = {
	0x04, 0x03, 0x00, 0x00, 0x00, 0x02, 0x01, 0x01, 0x01
};

/**
 * Update view table entries.
 * This function is called at the end of each interpreter cycle
 * to update the view table entries and blit the sprites.
 */
void AgiEngine::updateViewtable() {
	VtEntry *v;
	int i, loop;

	i = 0;
	for (v = _game.viewTable; v < &_game.viewTable[MAX_VIEWTABLE]; v++) {
		if ((v->flags & (ANIMATED | UPDATE | DRAWN)) != (ANIMATED | UPDATE | DRAWN)) {
			continue;
		}

		i++;

		loop = 4;
		if (~v->flags & FIX_LOOP) {
			switch (v->numLoops) {
			case 2:
			case 3:
				loop = loopTable2[v->direction];
				break;
			case 4:
				loop = loopTable4[v->direction];
				break;
			default:
				/* for KQ4 */
				if (agiGetRelease() == 0x3086)
					loop = loopTable4[v->direction];
				break;
			}
		}

		/* AGI 2.272 (ddp, xmas) doesn't test step_time_count! */
		if (loop != 4 && loop != v->currentLoop) {
			if (agiGetRelease() <= 0x2272 ||
			    v->stepTimeCount == 1) {
				setLoop(v, loop);
			}
		}

		if (~v->flags & CYCLING)
			continue;

		if (v->cycleTimeCount == 0)
			continue;

		if (--v->cycleTimeCount == 0) {
			updateView(v);
			v->cycleTimeCount = v->cycleTime;
		}
	}

	if (i) {
		_sprites->eraseUpdSprites();
		updatePosition();
		_sprites->blitUpdSprites();
		_sprites->commitUpdSprites();
		_game.viewTable[0].flags &= ~(ON_WATER | ON_LAND);
	}
}

bool AgiEngine::isEgoView(const VtEntry* v) {
	return v == _game.viewTable;
}

} // End of namespace Agi
