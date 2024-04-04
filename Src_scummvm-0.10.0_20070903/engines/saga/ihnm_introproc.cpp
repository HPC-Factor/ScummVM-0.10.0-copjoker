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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/engines/saga/ihnm_introproc.cpp $
 * $Id: ihnm_introproc.cpp 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

// "I Have No Mouth" Intro sequence scene procedures

#include "saga/saga.h"
#include "saga/gfx.h"

#include "saga/animation.h"
#include "saga/events.h"
#include "saga/interface.h"
#include "saga/sndres.h"
#include "saga/music.h"

#include "saga/scene.h"

namespace Saga {

SceneResourceData IHNM_IntroMovie1RL[] = {
	{30, 2, 0, 0, false} ,
	{31, 14, 0, 0, false}
};

SceneDescription IHNM_IntroMovie1Desc = {
	0, 0, 0, 0, 0, 0, 0, 0,
	IHNM_IntroMovie1RL,
	ARRAYSIZE(IHNM_IntroMovie1RL)
};

SceneResourceData IHNM_IntroMovie2RL[] = {
	{32, 2, 0, 0, false} ,
	{33, 14, 0, 0, false}
};

SceneDescription IHNM_IntroMovie2Desc = {
	0, 0, 0, 0, 0, 0, 0, 0,
	IHNM_IntroMovie2RL,
	ARRAYSIZE(IHNM_IntroMovie2RL)
};

SceneResourceData IHNM_IntroMovie3RL[] = {
	{34, 2, 0, 0, false},
	{35, 14, 0, 0, false}
};

SceneDescription IHNM_IntroMovie3Desc = {
	0, 0, 0, 0, 0, 0, 0, 0,
	IHNM_IntroMovie3RL,
	ARRAYSIZE(IHNM_IntroMovie3RL)
};

SceneResourceData IHNM_IntroMovie4RL[] = {
	{1227, 2, 0, 0, false},
	{1226, 14, 0, 0, false}
};

SceneDescription IHNM_IntroMovie4Desc = {
	0, 0, 0, 0, 0, 0, 0, 0,
	IHNM_IntroMovie4RL,
	ARRAYSIZE(IHNM_IntroMovie4RL)
};

LoadSceneParams IHNM_IntroList[] = {
	{0, kLoadByDescription, &IHNM_IntroMovie1Desc, Scene::SC_IHNMIntroMovieProc1, false, kTransitionNoFade, 0, NO_CHAPTER_CHANGE},
	{0, kLoadByDescription, &IHNM_IntroMovie2Desc, Scene::SC_IHNMIntroMovieProc2, false, kTransitionNoFade, 0, NO_CHAPTER_CHANGE},
	{0, kLoadByDescription, &IHNM_IntroMovie3Desc, Scene::SC_IHNMIntroMovieProc3, false, kTransitionNoFade, 0, NO_CHAPTER_CHANGE},
};

int Scene::IHNMStartProc() {
	size_t n_introscenes;
	size_t i;

	LoadSceneParams firstScene;

	// The original used the "play video" mechanism for the first part of
	// the intro. We just use that panel mode.

	_vm->_interface->setMode(kPanelVideo);

	n_introscenes = ARRAYSIZE(IHNM_IntroList);

	for (i = 0; i < n_introscenes; i++) {
		_vm->_scene->queueScene(&IHNM_IntroList[i]);
	}

	firstScene.loadFlag = kLoadBySceneNumber;
	firstScene.sceneDescriptor = -1;
	firstScene.sceneDescription = NULL;
	firstScene.sceneSkipTarget = true;
	firstScene.sceneProc = NULL;
	firstScene.transitionType = kTransitionFade;
	firstScene.actorsEntrance = 0;
	firstScene.chapter = -1;

	_vm->_scene->queueScene(&firstScene);

	return SUCCESS;
}

int Scene::SC_IHNMIntroMovieProc1(int param, void *refCon) {
	return ((Scene *)refCon)->IHNMIntroMovieProc1(param);
}

int Scene::IHNMIntroMovieProc1(int param) {
	Event event;
	Event *q_event;

	switch (param) {
	case SCENE_BEGIN:
		// Background for intro scene is the first frame of the
		// intro animation; display it and set the palette
		event.type = kEvTOneshot;
		event.code = kBgEvent;
		event.op = kEventDisplay;
		event.param = kEvPSetPalette;
		event.time = 0;

		q_event = _vm->_events->queue(&event);

		_vm->_anim->setFrameTime(0, IHNM_INTRO_FRAMETIME);
		_vm->_anim->setFlag(0, ANIM_FLAG_ENDSCENE);

		event.type = kEvTOneshot;
		event.code = kAnimEvent;
		event.op = kEventPlay;
		event.param = 0;
		event.time = 0;

		q_event = _vm->_events->chain(q_event, &event);
		break;
	default:
		break;
	}

	return 0;
}

int Scene::SC_IHNMIntroMovieProc2(int param, void *refCon) {
	return ((Scene *)refCon)->IHNMIntroMovieProc2(param);
}

int Scene::IHNMIntroMovieProc2(int param) {
	Event event;
	Event *q_event;
	PalEntry *pal;

	static PalEntry current_pal[PAL_ENTRIES];

	switch (param) {
	case SCENE_BEGIN:
		// Fade to black out of the intro CyberDreams logo anim
		_vm->_gfx->getCurrentPal(current_pal);

		event.type = kEvTContinuous;
		event.code = kPalEvent;
		event.op = kEventPalToBlack;
		event.time = 0;
		event.duration = IHNM_PALFADE_TIME;
		event.data = current_pal;

		q_event = _vm->_events->queue(&event);

		// Background for intro scene is the first frame of the
		// intro animation; display it but don't set palette
		event.type = kEvTOneshot;
		event.code = kBgEvent;
		event.op = kEventDisplay;
		event.param = kEvPNoSetPalette;
		event.time = 0;

		q_event = _vm->_events->chain(q_event, &event);

		_vm->_anim->setCycles(0, -1);

		// Unlike the original, we keep the logo spinning during the
		// palette fades. We don't have to, but I think it looks better
		// that way.

		event.type = kEvTOneshot;
		event.code = kAnimEvent;
		event.op = kEventPlay;
		event.param = 0;
		event.time = 0;

		q_event = _vm->_events->chain(q_event, &event);

		// Fade in from black to the scene background palette
		_vm->_scene->getBGPal(pal);

		event.type = kEvTContinuous;
		event.code = kPalEvent;
		event.op = kEventBlackToPal;
		event.time = 0;
		event.duration = IHNM_PALFADE_TIME;
		event.data = pal;

		q_event = _vm->_events->chain(q_event, &event);

		// Fade to black after looping animation for a while
		event.type = kEvTContinuous;
		event.code = kPalEvent;
		event.op = kEventPalToBlack;
		event.time = IHNM_DGLOGO_TIME;
		event.duration = IHNM_PALFADE_TIME;
		event.data = pal;

		q_event = _vm->_events->chain(q_event, &event);

		// Queue end of scene
		event.type = kEvTOneshot;
		event.code = kSceneEvent;
		event.op = kEventEnd;
		event.time = 0;

		q_event = _vm->_events->chain(q_event, &event);
		break;
	default:
		break;
	}

	return 0;
}

int Scene::SC_IHNMIntroMovieProc3(int param, void *refCon) {
	return ((Scene *)refCon)->IHNMIntroMovieProc3(param);
}

int Scene::IHNMIntroMovieProc3(int param) {
	Event event;
	Event *q_event;
	PalEntry *pal;
	static PalEntry current_pal[PAL_ENTRIES];

	switch (param) {
	case SCENE_BEGIN:
		// Fade to black out of the intro DG logo anim
		_vm->_gfx->getCurrentPal(current_pal);

		event.type = kEvTContinuous;
		event.code = kPalEvent;
		event.op = kEventPalToBlack;
		event.time = 0;
		event.duration = IHNM_PALFADE_TIME;
		event.data = current_pal;

		q_event = _vm->_events->queue(&event);

		// Music, maestro

		// In the GM file, this music also appears as tracks 7, 13, 19,
		// 25 and 31, but only track 1 sounds right with the FM music.

		event.type = kEvTOneshot;
		event.code = kMusicEvent;
		event.param = 1;
		event.param2 = MUSIC_NORMAL;
		event.op = kEventPlay;
		event.time = 0;

		q_event = _vm->_events->chain(q_event, &event);

		// Background for intro scene is the first frame of the intro
		// animation; display it but don't set palette
		event.type = kEvTOneshot;
		event.code = kBgEvent;
		event.op = kEventDisplay;
		event.param = kEvPNoSetPalette;
		event.time = 0;

		q_event = _vm->_events->chain(q_event, &event);

		// Fade in from black to the scene background palette
		_vm->_scene->getBGPal(pal);

		event.type = kEvTContinuous;
		event.code = kPalEvent;
		event.op = kEventBlackToPal;
		event.time = 0;
		event.duration = IHNM_PALFADE_TIME;
		event.data = pal;

		q_event = _vm->_events->chain(q_event, &event);

		event.type = kEvTOneshot;
		event.code = kAnimEvent;
		event.op = kEventPlay;
		event.param = 0;
		event.time = 0;

		q_event = _vm->_events->chain(q_event, &event);

		// Queue end of scene after a while
		// TODO: I've increased the delay so the speech won't start
		// until the music has ended. Could someone verify if that's
		// the correct behaviour?
		event.type = kEvTOneshot;
		event.code = kSceneEvent;
		event.op = kEventEnd;
		event.time = _vm->_music->hasAdlib() ? IHNM_TITLE_TIME_FM : IHNM_TITLE_TIME_GM;

		q_event = _vm->_events->chain(q_event, &event);
		break;
	default:
		break;
	}

	return 0;
}

} // End of namespace Saga
