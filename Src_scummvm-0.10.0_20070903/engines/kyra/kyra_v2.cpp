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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/engines/kyra/kyra_v2.cpp $
 * $Id: kyra_v2.cpp 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

#include "kyra/kyra.h"
#include "kyra/kyra_v2.h"
#include "kyra/screen.h"
#include "kyra/resource.h"
#include "kyra/wsamovie.h"
#include "kyra/sound.h"

#include "common/system.h"

namespace Kyra {

KyraEngine_v2::KyraEngine_v2(OSystem *system, const GameFlags &flags) : KyraEngine(system, flags) {
	memset(_gameShapes, 0, sizeof(_gameShapes));
	_mouseSHPBuf = 0;
}

KyraEngine_v2::~KyraEngine_v2() {
	delete [] _mouseSHPBuf;
}

int KyraEngine_v2::init() {
	KyraEngine::init();

	if (_res->getFileSize("6.FNT"))
		_screen->loadFont(Screen::FID_6_FNT, "6.FNT");
	if (_res->getFileSize("8FAT.FNT"))
		_screen->loadFont(Screen::FID_8_FNT, "8FAT.FNT");
	_screen->loadFont(Screen::FID_GOLDFONT_FNT, "GOLDFONT.FNT");
	_screen->setAnimBlockPtr(3500);
	_screen->setScreenDim(0);
	
	assert(_introStringsSize == 21);
	for (int i = 0; i < 21; i++)
		_introStringsDuration[i] = strlen(_introStrings[i]) * 8;
	
	// No mouse display in demo
	if (_flags.isDemo)
		return 0;

	_mouseSHPBuf = _res->fileData("PWGMOUSE.SHP", 0);
	assert(_mouseSHPBuf);

	for (int i = 0; i < 2; i++) {
		_gameShapes[i] = _screen->getPtrToShape(_mouseSHPBuf, i);
		assert(_gameShapes[i]);
	}

	_screen->setMouseCursor(0, 0, _gameShapes[0]);
	return 0;
}

int KyraEngine_v2::go() {	
	if (_flags.isDemo) {
		static const char *soundFileList[] = {
			"K2_DEMO",
			"LOLSYSEX"
		};
		_sound->setSoundFileList(soundFileList, 2);
	} else {
		// TODO: move this to proper place
		static const char *soundFileList[] = {
			"K2INTRO"
		};
		_sound->setSoundFileList(soundFileList, 1);
	}
	_sound->loadSoundFile(0);

	// Temporary measure to work around the fact that there's 
	// several WSA files with identical names in different PAK files.
	_res->unloadPakFile("OUTFARM.PAK");
	_res->unloadPakFile("FLYTRAP.PAK");

	seq_playSequences(kSequenceVirgin, kSequenceWestwood);
	mainMenu();

	return 0;
}

void KyraEngine_v2::mainMenu() {
	bool running = true;

	while (running && !_quitFlag) {
		seq_playSequences(kSequenceTitle);
		_screen->showMouse();
		
		switch (gui_handleMainMenu()) {
			case 0:
				break;
			case 1:
				seq_playSequences(kSequenceOverview, kSequenceLibrary); 
				break;
			case 2:
				break;
			case 3:
				running = false;
				break;
			default:
				break;
		}
		_screen->hideMouse();
	}	
}

} // end of namespace Kyra

