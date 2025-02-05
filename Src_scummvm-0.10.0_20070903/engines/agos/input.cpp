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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/engines/agos/input.cpp $
 * $Id: input.cpp 27279 2007-06-10 05:40:20Z Kirben $
 *
 */

#include "common/stdafx.h"

#include "common/config-manager.h"
#include "common/file.h"

#include "agos/intern.h"
#include "agos/agos.h"
#include "agos/vga.h"

namespace AGOS {

uint AGOSEngine::setVerbText(HitArea *ha) {
	uint id = 0xFFFF;

	if (getGameType() == GType_ELVIRA1 || getGameType() == GType_ELVIRA2)
		return id;

	if (ha->flags & kBFTextBox) {
		if (getGameType() == GType_PP)
			id = ha->id;
		else if (getGameType() == GType_FF && (ha->flags & kBFHyperBox))
			id = ha->data;
		else
			id = ha->flags / 256;
	}
	if (getGameType() == GType_PP)
		_variableArray[199] = id;
	else if (getGameType() == GType_WW)
		_variableArray[10] = id;
	else
		_variableArray[60] = id;

	return id;
}

void AGOSEngine::setup_cond_c_helper() {
	HitArea *last;

	_noRightClick = 1;

	if (getGameType() == GType_WW)
		clearMenuStrip();

	if (getGameType() == GType_FF) {
		int cursor = 5;
		int animMax = 16;

		if (getBitFlag(200)) {
			cursor = 11;
			animMax = 5;
		} else if (getBitFlag(201)) {
			cursor = 12;
			animMax = 5;
		} else if (getBitFlag(202)) {
			cursor = 13;
			animMax = 5;
		} else if (getBitFlag(203)) {
			cursor = 14;
			animMax = 9;
		} else if (getBitFlag(205)) {
			cursor = 17;
			animMax = 11;
		} else if (getBitFlag(206)) {
			cursor = 16;
			animMax = 2;
		} else if (getBitFlag(208)) {
			cursor = 26;
			animMax = 2;
		} else if (getBitFlag(209)) {
			cursor = 27;
			animMax = 9;
		} else if (getBitFlag(210)) {
			cursor = 28;
			animMax = 9;
		}

		_animatePointer = 0;
		_mouseCursor = cursor;
		_mouseAnimMax = animMax;
		_mouseAnim = 1;
		_needHitAreaRecalc++;
	}

	if (getGameType() == GType_SIMON2) {
		_mouseCursor = 0;
		if (_defaultVerb != 999) {
			_mouseCursor = 9;
			_needHitAreaRecalc++;
			_defaultVerb = 0;
		}
	}

	_lastHitArea = 0;
	_hitAreaObjectItem = NULL;
	_nameLocked = 0;

	last = _lastNameOn;
	clearName();
	_lastNameOn = last;

	for (;;) {
		_lastHitArea = NULL;
		_lastHitArea3 = 0;
		_leftButtonDown = 0;

		do {
			if (_exitCutscene && getBitFlag(9)) {
				endCutscene();
				goto out_of_here;
			}

			if (getGameType() == GType_FF) {
				if (_variableArray[254] == 63) {
					hitarea_stuff_helper_2();
				} else if (_variableArray[254] == 75) {
					hitarea_stuff_helper_2();
					_variableArray[60] = 9999;
					goto out_of_here;
				}
			}

			delay(100);
		} while (_lastHitArea3 == (HitArea *) -1 || _lastHitArea3 == 0);

		if (_lastHitArea == NULL) {
		} else if (_lastHitArea->id == 0x7FFB) {
			inventoryUp(_lastHitArea->window);
		} else if (_lastHitArea->id == 0x7FFC) {
			inventoryDown(_lastHitArea->window);
		} else if (_lastHitArea->item_ptr != NULL) {
			_hitAreaObjectItem = _lastHitArea->item_ptr;
			setVerbText(_lastHitArea);
			break;
		}
	}

out_of_here:
	_lastHitArea3 = 0;
	_lastHitArea = 0;
	_lastNameOn = NULL;

	_mouseCursor = 0;
	_noRightClick = 0;
}

void AGOSEngine::waitForInput() {
	HitArea *ha;
	uint id;

	_leftButtonDown = 0;
	_lastHitArea = 0;
	//_lastClickRem = 0;
	_verbHitArea = 0;
	_hitAreaSubjectItem = NULL;
	_hitAreaObjectItem = NULL;
	_clickOnly = 0;
	_nameLocked = 0;

	if (getGameType() == GType_WW) {
		_mouseCursor = 0;
		_needHitAreaRecalc++;
		clearMenuStrip();
	} else {
		resetVerbs();
	}

	for (;;) {
		_lastHitArea = NULL;
		_lastHitArea3 = NULL;
		_dragAccept = 1;

		for (;;) {
			if ((getGameType() == GType_SIMON1 || getGameType() == GType_SIMON2) && _keyPressed == 35)
				displayBoxStars();
			if (processSpecialKeys()) {
				if ((getGameType() == GType_PP && getGameId() != GID_DIMP) ||
					getGameType() == GType_WW)
					goto out_of_here;
			}
			if (_lastHitArea3 == (HitArea *) -1) {
				_lastHitArea = NULL;
				_lastHitArea3 = NULL;
				_dragAccept = 1;
			} else {
				if (_lastHitArea3 || _dragMode)
					break;
				hitarea_stuff_helper();
				delay(100);
			}
		}

		if (!_lastHitArea3 && _dragMode) {
			ha = _lastClickRem;

			if (ha == 0 || ha->item_ptr == NULL || !(ha->flags & kBFDragBox)) {
				_dragFlag = 0;
				_dragMode = 0;
				_dragCount = 0;
				_dragEnd = 0;
				continue;
			}
		
			_hitAreaSubjectItem = ha->item_ptr;
			_verbHitArea = 500;
		
			do {
				processSpecialKeys();
				hitarea_stuff_helper();
				delay(100);
		
				if (_dragFlag == 0) {
					_dragFlag = 0;
					_dragMode = 0;
					_dragCount = 0;
					_dragEnd = 0;
					continue;
				}
			} while (!_dragEnd);
		
			_dragFlag = 0;
			_dragMode = 0;
			_dragCount = 0;
			_dragEnd = 0;
		
			boxController(_mouse.x, _mouse.y, 1);
		
			if (_currentBox != NULL) {
				_hitAreaObjectItem = _currentBox->item_ptr;
				setVerbText(ha);
			}
		
			break;
		}

		ha = _lastHitArea;
		if (ha == NULL) {
		} else if (ha->id == 0x7FFB) {
			inventoryUp(ha->window);
		} else if (ha->id == 0x7FFC) {
			inventoryDown(ha->window);
		} else if ((getGameType() == GType_SIMON1 || getGameType() == GType_SIMON2) &&
			(ha->id >= 101 && ha->id < 113)) {
			_verbHitArea = ha->verb;
			setVerb(ha);
			_defaultVerb = 0;
		} else {
			if (getGameType() == GType_WW) {
				if (_mouseCursor == 3)
					_verbHitArea = 236;

				if (ha->id == 98) {
					animate(2, 0, 110, 0, 0, 0);
					waitForSync(34);
				} else if (ha->id == 108) {
					animate(2, 0, 106, 0, 0, 0);
					waitForSync(34);
				} else if (ha->id == 109) {
					animate(2, 0, 107, 0, 0, 0);
					waitForSync(34);
				} else if (ha->id == 115) {
					animate(2, 0, 109, 0, 0, 0);
					waitForSync(34);
				} else if (ha->id == 116) {
					animate(2, 0, 113, 0, 0, 0);
					waitForSync(34);
				} else if (ha->id == 117) {
					animate(2, 0, 112, 0, 0, 0);
					waitForSync(34);
				} else if (ha->id == 118) {
					animate(2, 0, 108, 0, 0, 0);
					waitForSync(34);
				} else if (ha->id == 119) {
					animate(2, 0, 111, 0, 0, 0);
					waitForSync(34);
				}
			}
			if (ha->item_ptr && (!ha->verb || _verbHitArea ||
					(_hitAreaSubjectItem != ha->item_ptr && (ha->flags & kBFBoxItem)))
				) {
				_hitAreaSubjectItem = ha->item_ptr;
				id = setVerbText(ha);
				_nameLocked = 2;
				displayName(ha);
				_nameLocked = 1;

				if (_verbHitArea) {
					break;
				}

				if (getGameType() == GType_WW)
					doMenuStrip(menuFor_ww(ha->item_ptr, id));
				else if (getGameType() == GType_ELVIRA2)
					doMenuStrip(menuFor_e2(ha->item_ptr));
				else if (getGameType() == GType_ELVIRA1)
					lightMenuStrip(getUserFlag1(ha->item_ptr, 6));
			} else {
				if (ha->verb) {
					if (getGameType() == GType_WW && _mouseCursor && _mouseCursor < 4) {
						_hitAreaSubjectItem = ha->item_ptr;
						break;
					}

					_verbHitArea = ha->verb & 0xBFFF;
					if (ha->verb & 0x4000) {
						_hitAreaSubjectItem = ha->item_ptr;
						break;
					}
					if (_hitAreaSubjectItem != NULL)
						break;

					if (getGameType() == GType_WW) {
						if (ha->id == 109) {
							_mouseCursor = 2;
							_needHitAreaRecalc++;
						} else if (ha->id == 117) {
							_mouseCursor = 3;
							_needHitAreaRecalc++;
						}
					}
				}
			}
		}
	}

out_of_here:
	if (getGameType() == GType_ELVIRA2 || getGameType() == GType_WW)
		clearMenuStrip();
	else if (getGameType() == GType_ELVIRA1)
		unlightMenuStrip();

	_nameLocked = 0;
	_needHitAreaRecalc++;
	_dragAccept = 0;

	if (getGameType() == GType_WW && _mouseCursor < 3)
		_mouseCursor = 0;
}

void AGOSEngine::hitarea_stuff_helper() {
	time_t cur_time;

	if (getGameType() == GType_SIMON2 || getGameType() == GType_FF ||
		getGameType() == GType_PP) {
		if (_variableArray[254] || _variableArray[249]) {
			hitarea_stuff_helper_2();
		}
	} else if (getGameType() == GType_ELVIRA2 || getGameType() == GType_WW || 
		getGameType() == GType_SIMON1) {
		uint subr_id = (uint16)_variableArray[254];
		if (subr_id) {
			Subroutine *sub = getSubroutineByID(subr_id);
			if (sub != NULL) {
				startSubroutineEx(sub);
				permitInput();
			}
			_variableArray[254] = 0;
			_runScriptReturn1 = false;
		}
	}

	time(&cur_time);
	if ((uint) cur_time != _lastTime) {
		_lastTime = cur_time;
		if (kickoffTimeEvents())
			permitInput();
	}

	if (getGameId() == GID_DIMP)
		delay(200);
}

void AGOSEngine::hitarea_stuff_helper_2() {
	uint subr_id;
	Subroutine *sub;

	subr_id = (uint16)_variableArray[249];
	if (subr_id) {
		sub = getSubroutineByID(subr_id);
		if (sub != NULL) {
			_variableArray[249] = 0;
			startSubroutineEx(sub);
			permitInput();
		}
		_variableArray[249] = 0;
	}

	subr_id = (uint16)_variableArray[254];
	if (subr_id) {
		sub = getSubroutineByID(subr_id);
		if (sub != NULL) {
			_variableArray[254] = 0;
			startSubroutineEx(sub);
			permitInput();
		}
		_variableArray[254] = 0;
	}

	_runScriptReturn1 = false;
}

void AGOSEngine::permitInput() {
	if (!_mortalFlag)
		return;


	_mortalFlag = true;
	justifyOutPut(0);

	if (getGameType() == GType_ELVIRA1 || getGameType() == GType_ELVIRA2 || getGameType() == GType_WW) {
		int n = 0;
		while (n < 8) {
			if ((_fcsData1[n]) && (_windowArray[n]) && (_windowArray[n]->flags & 128)) {
				_textWindow = _windowArray[n];
				waitWindow(_textWindow);
				clsCheck(_textWindow);
			}
			_fcsData1[n]=0;
			n++;
		}

		restartAnimation();
	}

	_curWindow = 0;
	if (_windowArray[0]) {
		_textWindow = _windowArray[0];
		justifyStart();
	}
	_mortalFlag = false;

}

bool AGOSEngine::processSpecialKeys() {
	bool verbCode = false;

	if (getGameId() == GID_DIMP) {
		static time_t lastMinute = 0;
		time_t t;
		time_t t1;
		t = time(&t);
		t1 = t / 30;
		if (!lastMinute)
			lastMinute = t1;
		if (t1 - lastMinute) {
			_variableArray[120] += (t1 - lastMinute);
			lastMinute = t1;
		}
	}

	switch (_keyPressed) {
	case 17: // Up
		if (getGameType() == GType_PP)
			_verbHitArea = 302;
		else if (getGameType() == GType_WW)
			_verbHitArea = 239;
		verbCode = true;
		break;
	case 18: // Down
		if (getGameType() == GType_PP)
			_verbHitArea = 304;
		else if (getGameType() == GType_WW)
			_verbHitArea = 241;
		verbCode = true;
		break;
	case 19: // Right
		if (getGameType() == GType_PP)
			_verbHitArea = 303;
		else if (getGameType() == GType_WW)
			_verbHitArea = 240;
		verbCode = true;
		break;
	case 20: // Left
		if (getGameType() == GType_PP)
			_verbHitArea = 301;
		else if (getGameType() == GType_WW)
			_verbHitArea = 242;
		verbCode = true;
		break;
	case 27: // escape
		_exitCutscene = true;
		break;
	case 59: // F1
		if (getGameType() == GType_SIMON2) {
			vcWriteVar(5, 50);
			vcWriteVar(86, 0);
		} else if (getGameType() == GType_SIMON1) {
			vcWriteVar(5, 40);
			vcWriteVar(86, 0);
		}
		break;
	case 60: // F2
		if (getGameType() == GType_SIMON2) {
			vcWriteVar(5, 75);
			vcWriteVar(86, 1);
		} else if (getGameType() == GType_SIMON1) {
			vcWriteVar(5, 60);
			vcWriteVar(86, 1);
		}
		break;
	case 61: // F3
		if (getGameType() == GType_SIMON2) {
			vcWriteVar(5, 125);
			vcWriteVar(86, 2);
		} else if (getGameType() == GType_SIMON1) {
			vcWriteVar(5, 100);
			vcWriteVar(86, 2);
		}
		break;
	case 63: // F5
		if (getGameType() == GType_SIMON2 || getGameType() == GType_FF)
			_exitCutscene = true;
		break;
	case 65: // F7
		if (getGameType() == GType_FF && getBitFlag(76))
			_variableArray[254] = 70;
		break;
	case 67: // F9
		if (getGameType() == GType_FF)
			setBitFlag(73, !getBitFlag(73));
		break;
	case 37: // F12
		if (getGameType() == GType_PP && getGameId() != GID_DIMP) {
			if (!getBitFlag(110)) {
				setBitFlag(107, !getBitFlag(107));
				_vgaPeriod = (getBitFlag(107) != 0) ? 15 : 30;
			}
		}
		break;
	case 'p':
		pause();
		break;
	case 't':
		if (getGameType() == GType_FF || (getGameType() == GType_SIMON2 && (getFeatures() & GF_TALKIE)) ||
			((getFeatures() & GF_TALKIE) && _language != Common::EN_ANY && _language != Common::DE_DEU)) {
			if (_speech)
				_subtitles ^= 1;
		}
		break;
	case 'v':
		if (getGameType() == GType_FF || (getGameType() == GType_SIMON2 && (getFeatures() & GF_TALKIE))) {
			if (_subtitles)
				_speech ^= 1;
		}
	case '+':
		if (_midiEnabled) {
			_midi.setVolume(_midi.getVolume() + 16);
		}
		_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, _mixer->getVolumeForSoundType(Audio::Mixer::kMusicSoundType) + 16);
		break;
	case '-':
		if (_midiEnabled) {
			_midi.setVolume(_midi.getVolume() - 16);
		}
		_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, _mixer->getVolumeForSoundType(Audio::Mixer::kMusicSoundType) - 16);
		break;
	case 'm':
		_musicPaused ^= 1;
		if (_midiEnabled) {
			_midi.pause(_musicPaused);
		}
		_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, (_musicPaused) ? 0 : ConfMan.getInt("music_volume"));
		break;
	case 's':
		if (getGameId() == GID_SIMON1DOS) {
			_midi._enable_sfx ^= 1;
		} else {
			_sound->effectsPause(_effectsPaused ^= 1);
		}
		break;
	case 'b':
		_sound->ambientPause(_ambientPaused ^= 1);
		break;
	case 'r':
		if (_debugMode)
			_startMainScript ^= 1;
		break;
	case 'o':
		if (_debugMode)
			_continousMainScript ^= 1;
		break;
	case 'a':
		if (_debugMode)
			_startVgaScript ^= 1;
		break;
	case 'g':
		if (_debugMode)
			_continousVgaScript ^= 1;
		break;
	case 'd':
		if (_debugMode)
			_dumpImages ^=1;
		break;
	}

	_keyPressed = 0;
	return verbCode;
}

} // End of namespace AGOS


