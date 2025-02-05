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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/engines/agos/saveload.cpp $
 * $Id: saveload.cpp 27273 2007-06-10 02:24:11Z Kirben $
 *
 */

#include "common/stdafx.h"

#include "common/savefile.h"
#include "common/system.h"

#include "gui/about.h"
#include "gui/message.h"

#include "agos/agos.h"
#include "agos/intern.h"

namespace AGOS {

int AGOSEngine::countSaveGames() {
	Common::InSaveFile *f;
	uint i = 1;
	bool marks[256];

	char *prefix = genSaveName(998);
	prefix[strlen(prefix)-3] = '\0';
	_saveFileMan->listSavefiles(prefix, marks, 256);

	while (i < 256) {
		if (marks[i] &&
		    (f = _saveFileMan->openForLoading(genSaveName(i)))) {
			i++;
			delete f;
		} else
			break;
	}
	return i;
}

char *AGOSEngine::genSaveName(int slot) {
	static char buf[15];

	if (getGameId() == GID_DIMP) {
		sprintf(buf, "dimp.sav");
	} else if (getGameType() == GType_PP) {
		sprintf(buf, "swampy.sav");
	} else if (getGameType() == GType_FF) {
		sprintf(buf, "feeble.%.3d", slot);
	} else if (getGameType() == GType_SIMON2) {
		sprintf(buf, "simon2.%.3d", slot);
	} else if (getGameType() == GType_SIMON1) {
		sprintf(buf, "simon1.%.3d", slot);
	} else if (getGameType() == GType_WW) {
		sprintf(buf, "waxworks.%.3d", slot);
	} else if (getGameType() == GType_ELVIRA2) {
		sprintf(buf, "elvira2.%.3d", slot);
	} else if (getGameType() == GType_ELVIRA1) {
		sprintf(buf, "elvira1.%.3d", slot);
	}
	return buf;
}

void AGOSEngine::quickLoadOrSave() {
	// Quick load & save is only supported complete version of Simon the Sorcerer 1/2
	if (getGameType() == GType_PP || getGameType() == GType_FF ||
		(getFeatures() & GF_DEMO)) {
		return;
	}

	bool success;
	char buf[50];

	char *filename = genSaveName(_saveLoadSlot);
	if (_saveLoadType == 2) {
		Subroutine *sub;
		success = loadGame(genSaveName(_saveLoadSlot));
		if (!success) {
			sprintf(buf, "Failed to load game state to file:\n\n%s", filename);
		} else if (getGameType() == GType_SIMON1 || getGameType() == GType_SIMON2) {
			drawIconArray(2, me(), 0, 0);
			setBitFlag(97, true);
			sub = getSubroutineByID(100);
			startSubroutine(sub);
		} else if (getGameType() == GType_WW) {
			sub = getSubroutineByID(66);
			startSubroutine(sub);
		} else if (getGameType() == GType_ELVIRA2) {
			sub = getSubroutineByID(87);
			startSubroutine(sub);
			setBitFlag(7, false);
			sub = getSubroutineByID(19);
			startSubroutine(sub);
			printStats();
			sub = getSubroutineByID(28);
			startSubroutine(sub);
			setBitFlag(17, false);
			sub = getSubroutineByID(207);
			startSubroutine(sub);
			sub = getSubroutineByID(71);
			startSubroutine(sub);
		} else if (getGameType() == GType_ELVIRA1) {
			drawIconArray(2, me(), 0, 0);
			sub = getSubroutineByID(265);
			startSubroutine(sub);
			sub = getSubroutineByID(129);
			startSubroutine(sub);
			sub = getSubroutineByID(131);
			startSubroutine(sub);
		}
	} else {
		success = saveGame(_saveLoadSlot, _saveLoadName);
		if (!success)
			sprintf(buf, "Failed to save game state to file:\n\n%s", filename);
	}

	if (!success) {
		GUI::MessageDialog dialog(buf, "OK");
		dialog.runModal();

	} else if (_saveLoadType == 1) {
		sprintf(buf, "Successfully saved game state in file:\n\n%s", filename);
		GUI::TimedMessageDialog dialog(buf, 1500);
		dialog.runModal();

	}

	_saveLoadType = 0;
}

bool AGOSEngine::confirmOverWrite(WindowBlock *window) {
	if (getGameType() == GType_WW) {
		Subroutine *sub = getSubroutineByID(80);
		if (sub != NULL)
			startSubroutineEx(sub);

		if (_variableArray[253] == 0)
			return true;
	} else if (getGameType() == GType_ELVIRA2) {
		// Original verison never confirmed
		return true;
	} else if (getGameType() == GType_ELVIRA1) {
		const char *message1, *message2, *message3;

		switch (_language) {
		case Common::FR_FRA:
			message1 = "\rFichier d/j; existant.\r\r";
			message2 = "  Ecrire pardessus ?\r\r";
			message3 = "     Oui      Non";
			break;
		case Common::DE_DEU:
			message1 = "\rDatei existiert bereits.\r\r";
			message2 = "     berschreiben ?\r\r";
			message3 = "     Ja        Nein";
			break;
		default:
			message1 = "\r File already exists.\r\r";
			message2 = "    Overwrite it ?\r\r";
			message3 = "     Yes       No";
			break;
		}

		printScroll();
		window->textColumn = 0;
		window->textRow = 0;
		window->textColumnOffset = 0;
		window->textLength = 0;		// Difference

		for (; *message1; message1++)
			windowPutChar(window, *message1);
		for (; *message2; message2++)
			windowPutChar(window, *message2);
		for (; *message3; message3++)
			windowPutChar(window, *message3);

		if (confirmYesOrNo(120, 78) == 0x7FFF)
			return true;
	}

	return false;
}

int16 AGOSEngine::matchSaveGame(const char *name, uint16 max) {
	Common::InSaveFile *in;
	char dst[8];
	uint16 slot;

	for (slot = 0; slot < max; slot++) {
		if ((in = _saveFileMan->openForLoading(genSaveName(slot)))) {
			in->read(dst, 8);
			delete in;

			if (!scumm_stricmp(name, dst)) {
				return slot;
			}
		}
	}

	return -1;
}

void AGOSEngine::userGame(bool load) {
	WindowBlock *window = _windowArray[4];
	const char *message1;
	int i, numSaveGames;
	char *name;
	char buf[8];

	numSaveGames = countSaveGames();
	
	time_t saveTime = time(NULL);
	haltAnimation();

restart:
	printScroll();
	window->textColumn = 0;
	window->textRow = 0;
	window->textColumnOffset = 0;
	window->textLength = 0;		// Difference

	switch (_language) {
	case Common::FR_FRA:
		message1 = "\rIns/rez disquette de\rsauvegarde de jeux &\rentrez nom de fichier:\r\r   ";
		break;
	case Common::DE_DEU:
		message1 = "\rLege Spielstandsdiskette ein. Dateinamen eingeben:\r\r   ";
		break;
	default:
		message1 = "\r Insert savegame data disk & enter filename:\r\r   ";
		break;
	}

	for (; *message1; message1++)
		windowPutChar(window, *message1);

	memset(buf, 0, 8);
	name = buf;
	_saveGameNameLen = 0;

	for (;;) {
		windowPutChar(window, 128);
		_keyPressed = 0;

		for (;;) {
			delay(10);
			if (_keyPressed && _keyPressed < 128) {
				i = _keyPressed;
				break;
			}
		}

		userGameBackSpace(_windowArray[4], 8);
		if (i == 10 || i == 13) {
			break;
		} else if (i == 8) {
			// do_backspace
			if (_saveGameNameLen) {
				_saveGameNameLen--;
				name[_saveGameNameLen] = 0;
				userGameBackSpace(_windowArray[4], 8);
			}
		} else if (i >= 32 && _saveGameNameLen != 8) {
			name[_saveGameNameLen++] = i;
			windowPutChar(_windowArray[4], i);
		}
	}

	if (_saveGameNameLen != 0) {
		int16 slot = matchSaveGame(name, numSaveGames);
		if (!load) {
			if (slot >= 0 && !confirmOverWrite(window))
				goto restart;

			if (slot < 0)
				slot =  numSaveGames;

			if (!saveGame(slot, name))
				fileError(_windowArray[4], true);
		} else {
			if (slot < 0) {
				fileError(_windowArray[4], false);
			} else {
				if (!loadGame(genSaveName(slot)))
					fileError(_windowArray[4], false);
			}
		}

		printStats();
	}

	restartAnimation();
	_gameStoppedClock = time(NULL) - saveTime + _gameStoppedClock;
}

void AGOSEngine_Elvira2::listSaveGames(char *dst) {
	Common::InSaveFile *in;
	uint y, slot;

	const uint8 num = (getGameType() == GType_WW) ? 3 : 4;

	disableFileBoxes();

	WindowBlock *window = _windowArray[num];
	window->textRow = 0;
	window->textColumn = 0;
	window->textColumnOffset = 4;

	windowPutChar(window, 12);

	memset(dst, 0, 200);

	slot = _saveLoadRowCurPos;
	for (y = 0; y < 8; y++) {
		window->textColumn = 0;
		window->textColumnOffset = (getGameType() == GType_ELVIRA2) ? 4 : 0;
		window->textLength = 0;
		if ((in = _saveFileMan->openForLoading(genSaveName(slot++)))) {
			in->read(dst, 8);
			delete in;

			const char *name = dst;
			for (; *name; name++)
				windowPutChar(window, *name);

			enableBox(200 + y * 3 + 0);
		}
		dst+= 8;

		if (getGameType() == GType_WW) {
			window->textColumn = 7;
			window->textColumnOffset = 4;
		} else if (getGameType() == GType_ELVIRA2) {
			window->textColumn = 8;
			window->textColumnOffset = 0;
		}
		window->textLength = 0;
		if ((in = _saveFileMan->openForLoading(genSaveName(slot++)))) {
			in->read(dst, 8);
			delete in;

			const char *name = dst;
			for (; *name; name++)
				windowPutChar(window, *name);

			enableBox(200 + y * 3 + 1);
		}
		dst+= 8;

		window->textColumn = 15;
		window->textColumnOffset = (getGameType() == GType_ELVIRA2) ? 4 : 0;
		window->textLength = 0;
		if ((in = _saveFileMan->openForLoading(genSaveName(slot++)))) {
			in->read(dst, 8);
			delete in;

			const char *name = dst;
			for (; *name; name++)
				windowPutChar(window, *name);

			enableBox(200 + y * 3 + 2);
		}
		dst+= 8;

		windowPutChar(window, 13);
	}

	window->textRow = 9;
	window->textColumn = 0;
	window->textColumnOffset = 4;
	window->textLength = 0;

	_saveGameNameLen = 0;
}

void AGOSEngine_Elvira2::userGame(bool load) {
	time_t saveTime;
	int i, numSaveGames;
	char *name;
	bool b;
	char buf[200];

	_saveOrLoad = load;

	saveTime = time(NULL);

	if (getGameType() == GType_ELVIRA2)
		haltAnimation();

	numSaveGames = countSaveGames();
	_numSaveGameRows = numSaveGames;
	_saveLoadRowCurPos = 1;
	_saveLoadEdit = false;

	const uint8 num = (getGameType() == GType_WW) ? 3 : 4;

	listSaveGames(buf);

	if (!load) {
		WindowBlock *window = _windowArray[num];
		int16 slot = -1;

		name = buf + 192;

		for (;;) {
			windowPutChar(window, 128);

			_saveLoadEdit = true;

			i = userGameGetKey(&b, buf, 128);
			if (b) {
				if (i <= 223) {
					if (!confirmOverWrite(window)) {
						listSaveGames(buf);
						continue;
					}

					if (!saveGame(_saveLoadRowCurPos + i, buf + i * 8))
						fileError(_windowArray[num], true);
				}

				goto get_out;
			}

			userGameBackSpace(_windowArray[num], 8);
			if (i == 10 || i == 13) {
				slot = matchSaveGame(name, numSaveGames);
				if (slot >= 0) {
					if (!confirmOverWrite(window)) {
						listSaveGames(buf);
						continue;
					}
				}
				break;
			} else if (i == 8) {
				// do_backspace
				if (_saveGameNameLen) {
					_saveGameNameLen--;
					name[_saveGameNameLen] = 0;
					userGameBackSpace(_windowArray[num], 8);
				}
			} else if (i >= 32 && _saveGameNameLen != 8) {
				name[_saveGameNameLen++] = i;
				windowPutChar(_windowArray[num], i);
			}
		}

		if (_saveGameNameLen != 0) {
			if (slot < 0)
				slot = numSaveGames;

			if (!saveGame(slot, buf + 192))
				fileError(_windowArray[num], true);
		}
	} else {
		i = userGameGetKey(&b, buf, 128);
		if (i != 225) {
			if (!loadGame(genSaveName(_saveLoadRowCurPos + i)))
				fileError(_windowArray[num], false);
		}
	}

get_out:;
	disableFileBoxes();

	_gameStoppedClock = time(NULL) - saveTime + _gameStoppedClock;

	if (getGameType() == GType_ELVIRA2)
		restartAnimation();
}

int AGOSEngine_Elvira2::userGameGetKey(bool *b, char *buf, uint maxChar) {
	HitArea *ha;
	*b = true;

	_keyPressed = 0;

	for (;;) {
		_lastHitArea = NULL;
		_lastHitArea3 = NULL;

		do {
			if (_saveLoadEdit && _keyPressed && _keyPressed < maxChar) {
				*b = false;
				return _keyPressed;
			}
			delay(10);
		} while (_lastHitArea3 == 0);

		ha = _lastHitArea;
		if (ha == NULL || ha->id < 200) {
		} else if (ha->id == 225) {
			return ha->id;
		} else if (ha->id == 224) {
			_saveGameNameLen = 0;
			_saveLoadRowCurPos += 24;
			if (_saveLoadRowCurPos >= _numSaveGameRows)
				_saveLoadRowCurPos = 1;

			listSaveGames(buf);
		} else if (ha->id < 224) {
			return ha->id - 200;
		}
	}
}

void AGOSEngine_Simon1::listSaveGames(char *dst) {
	Common::InSaveFile *in;
	uint i, slot, lastSlot;

	disableFileBoxes();

	showMessageFormat("\xC");

	memset(dst, 0, 108);

	slot = _saveLoadRowCurPos;
	while (_saveLoadRowCurPos + 6 > slot) {
		if (!(in = _saveFileMan->openForLoading(genSaveName(slot))))
			break;

		in->read(dst, 18);
		delete in;

		lastSlot = slot;
		if (slot < 10) {
			showMessageFormat(" ");
		} else if (_language == Common::HB_ISR) {
			lastSlot = (slot % 10) * 10;
			lastSlot += slot / 10;
		}

		showMessageFormat("%d", lastSlot);
		if (_language == Common::HB_ISR && !(slot % 10))
			showMessageFormat("0");
		showMessageFormat(".%s\n", dst);
		dst += 18;
		slot++;
	}

	if (!_saveOrLoad) {
		if (_saveLoadRowCurPos + 6 == slot) {
			slot++;
		} else {
			if (slot < 10)
				showMessageFormat(" ");
			showMessageFormat("%d.\n", slot);
		}
	} else {
		if (_saveLoadRowCurPos + 6 == slot) {
			if ((in = _saveFileMan->openForLoading(genSaveName(slot)))) {
				slot++;
				delete in;
			}
		}
	}

	_saveDialogFlag = true;

	i = slot - _saveLoadRowCurPos;
	if (i != 7) {
		i++;
		if (!_saveOrLoad)
			i++;
		_saveDialogFlag = false;
	}

	if (!--i)
		return;

	do {
		enableBox(208 + i - 1);
	} while (--i);
}

const byte hebrewKeyTable[96] = {
	32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 90, 45, 85, 47, 48, 49, 50,
	51, 52, 53, 54, 55, 56, 57, 83, 83, 90, 61, 85, 63, 35, 89, 80, 65, 66, 87,
	75, 82, 73, 79, 71, 76, 74, 86, 78, 77, 84, 47, 88, 67, 64, 69, 68, 44, 81,
	72, 70, 91, 92, 93, 94, 95, 96, 89, 80, 65, 66, 87, 75, 82, 73, 79, 71, 76,
	74, 86, 78, 77, 84, 47, 88, 67, 64, 69, 68, 44, 81, 72, 70,
	123, 124, 125, 126, 127,
};

void AGOSEngine_Simon1::userGame(bool load) {
	time_t saveTime;
	int i, numSaveGames, result;
	WindowBlock *window;
	char *name;
	bool b;
	char buf[108];
	int maxChar = (_language == Common::HB_ISR) ? 155: 128;

	_saveOrLoad = load;

	saveTime = time(NULL);

	numSaveGames = countSaveGames();
	if (!load)
		numSaveGames++;
	numSaveGames -= 6;
	if (numSaveGames < 0)
		numSaveGames = 0;
	numSaveGames++;
	_numSaveGameRows = numSaveGames;

	_saveLoadRowCurPos = 1;
	if (!load)
		_saveLoadRowCurPos = numSaveGames;

	_saveLoadEdit = false;

restart:;
	i = userGameGetKey(&b, buf, maxChar);

	if (i == 205)
		goto get_out;
	if (!load) {
		// if_1
	if_1:;
		result = i;

		disableBox(208 + i);
		leaveHitAreaById(208 + i);

		window = _windowArray[5];

		window->textRow = result;

		// init x offset with a 2 character savegame number + a period (18 pix)
		if (_language == Common::HB_ISR) {
			window->textColumn = 3;
			window->textColumnOffset = 6;
		} else {
			window->textColumn = 2;
			window->textColumnOffset = 2;
		}
		window->textLength = 3;

		name = buf + i * 18;

		// now process entire savegame name to get correct x offset for cursor
		_saveGameNameLen = 0;
		while (name[_saveGameNameLen]) {
			if (_language == Common::HB_ISR) {
				byte width = 6;
				if (name[_saveGameNameLen] >= 64 && name[_saveGameNameLen] < 91)
					width = _hebrewCharWidths [name[_saveGameNameLen] - 64];
				window->textLength++;
				window->textColumnOffset -= width;
				if (window->textColumnOffset < width) {
					window->textColumnOffset += 8;
					window->textColumn++;
				}
			} else {
				window->textLength++;
				window->textColumnOffset += 6;
				if (name[_saveGameNameLen] == 'i' || name[_saveGameNameLen] == 'l')
					window->textColumnOffset -= 2;
				if (window->textColumnOffset >= 8) {
					window->textColumnOffset -= 8;
					window->textColumn++;
				}
			}
			_saveGameNameLen++;
		}

		for (;;) {
			windowPutChar(window, 127);

			_saveLoadEdit = true;

			i = userGameGetKey(&b, buf, maxChar);

			if (b) {
				if (i == 205)
					goto get_out;
				enableBox(208 + result);
				if (_saveLoadEdit) {
					userGameBackSpace(_windowArray[5], 8);
				}
				goto if_1;
			}

			if (!_saveLoadEdit) {
				enableBox(208 + result);
				goto restart;
			}

			if (_language == Common::HB_ISR) {
				if (i >= 128)
					i -= 64;
				else if (i >= 32)
					i = hebrewKeyTable[i - 32];
			}

			userGameBackSpace(_windowArray[5], 8);
			if (i == 10 || i == 13) {
				break;
			} else if (i == 8) {
				// do_backspace
				if (_saveGameNameLen) {
					byte m, x;

					_saveGameNameLen--;
					m = name[_saveGameNameLen];

					if (_language == Common::HB_ISR)
						x = 8;
					else
						x = (name[_saveGameNameLen] == 'i' || name[_saveGameNameLen] == 'l') ? 1 : 8;

					name[_saveGameNameLen] = 0;

					userGameBackSpace(_windowArray[5], x, m);
				}
			} else if (i >= 32 && _saveGameNameLen != 17) {
				name[_saveGameNameLen++] = i;

				windowPutChar(_windowArray[5], i);
			}
		}

		if (!saveGame(_saveLoadRowCurPos + result, buf + result * 18))
			fileError(_windowArray[5], true);
	} else {
		if (!loadGame(genSaveName(_saveLoadRowCurPos + i)))
			fileError(_windowArray[5], false);
	}

get_out:;
	disableFileBoxes();

	_gameStoppedClock = time(NULL) - saveTime + _gameStoppedClock;
}

int AGOSEngine_Simon1::userGameGetKey(bool *b, char *buf, uint maxChar) {
	HitArea *ha;
	*b = true;

	if (!_saveLoadEdit) {
		listSaveGames(buf);
	}

	_keyPressed = 0;

	for (;;) {
		_lastHitArea = NULL;
		_lastHitArea3 = NULL;

		do {
			if (_saveLoadEdit && _keyPressed && _keyPressed < maxChar) {
				*b = false;
				return _keyPressed;
			}
			delay(10);
		} while (_lastHitArea3 == 0);

		ha = _lastHitArea;
		if (ha == NULL || ha->id < 205) {
		} else if (ha->id == 205) {
			return ha->id;
		} else if (ha->id == 206) {
			if (_saveLoadRowCurPos != 1) {
				if (_saveLoadRowCurPos < 7)
					_saveLoadRowCurPos = 1;
				else
					_saveLoadRowCurPos -= 6;

				_saveLoadEdit = false;
				listSaveGames(buf);
			}
		} else if (ha->id == 207) {
			if (_saveDialogFlag) {
				_saveLoadRowCurPos += 6;
				if (_saveLoadRowCurPos >= _numSaveGameRows)
					_saveLoadRowCurPos = _numSaveGameRows;

				_saveLoadEdit = false;
				listSaveGames(buf);
			}
		} else if (ha->id < 214) {
			return ha->id - 208;
		}
	}
}

void AGOSEngine::disableFileBoxes() {
	int i;

	if (getGameType() == GType_SIMON1 || getGameType() == GType_SIMON2) {
		for (i = 208; i != 214; i++)
			disableBox(i);
	} else {
		for (i = 200; i != 224; i++)
			disableBox(i);
	}
}

void AGOSEngine::userGameBackSpace(WindowBlock *window, int x, byte b) {
	byte old_text;

	windowPutChar(window, x, b);
	old_text = window->text_color;
	window->text_color = window->fill_color;

	if (_language == Common::HB_ISR) {
		x = 128;
	} else {
		x += 120;
		if (x != 128)
			x = 129;
	}

	windowPutChar(window, x);

	window->text_color = old_text;
	windowPutChar(window, 8);
}

void AGOSEngine::fileError(WindowBlock *window, bool save_error) {
	const char *message1, *message2;

	if (save_error) {
		switch (_language) {
		case Common::RU_RUS:
			if (getGameType() == GType_SIMON2) {
				message1 = "\r   Mf sowrap+fts+.";
				message2 = "\r  Nzjb#a ejs#a.";
			} else {
				message1 = "\r   Mf sowrap]fts].";
				message2 = "\r   Nzjb_a ejs_a.";
			}
			break;
		case Common::PL_POL:
			message1 = "\r      Blad zapisu.    ";
			message2 = "\rBlad dysku.                       ";
			break;
		case Common::ES_ESP:
			message1 = "\r     Error al salvar";
			message2 = "\r  Intenta con otro disco";
			break;
		case Common::IT_ITA:
			message1 = "\r  Salvataggio non riuscito";
			message2 = "\r    Prova un""\x27""altro disco";
			break;
		case Common::FR_FRA:
			message1 = "\r    Echec sauvegarde";
			message2 = "\rEssayez une autre disquette";
			break;
		case Common::DE_DEU:
			message1 = "\r  Sicherung erfolglos.";
			message2 = "\rVersuche eine andere     Diskette.";
			break;
		default:
			message1 = "\r       Save failed.";
			message2 = "\r       Disk error.";
			break;
		}
	} else {
		switch (_language) {
		case Common::RU_RUS:
			if (getGameType() == GType_SIMON2) {
				message1 = "\r  Mf ^adruhafts+.";
				message2 = "\r   Takm pf pakefp.";
			} else {
				message1 = "\r   Mf ^adruhafts].";
				message2 = "\r   Takm pf pakefp.";
			}
			break;
		case Common::PL_POL:
			message1 = "\r   Blad odczytu.    ";
			message2 = "\r  Nie znaleziono pliku.";
			break;
		case Common::ES_ESP:
			message1 = "\r     Error al cargar";
			message2 = "\r  Archivo no encontrado";
			break;
		case Common::IT_ITA:
			message1 = "\r  Caricamento non riuscito";
			message2 = "\r      File non trovato";
			break;
		case Common::FR_FRA:
			message1 = "\r    Echec chargement";
			message2 = "\r  Fichier introuvable";
			break;
		case Common::DE_DEU:
			message1 = "\r    Laden erfolglos.";
			message2 = "\r  Datei nicht gefunden.";
			break;
		default:
			message1 = "\r       Load failed.";
			message2 = "\r     File not found.";
			break;
		}
	}

	if (getGameType() == GType_ELVIRA1) {
		printScroll();
		window->textColumn = 0;
		window->textRow = 0;
		window->textColumnOffset = 0;
		window->textLength = 0;		// Difference
	} else {
		windowPutChar(window, 12);
	}

	for (; *message1; message1++)
		windowPutChar(window, *message1);
	for (; *message2; message2++)
		windowPutChar(window, *message2);

	waitWindow(window);
}

uint16 readItemID(Common::SeekableReadStream *f) {
	uint32 val = f->readUint32BE();
	if (val == 0xFFFFFFFF)
		return 0;
	return val + 1;
}

void writeItemID(Common::WriteStream *f, uint16 val) {
	if (val == 0)
		f->writeUint32BE(0xFFFFFFFF);
	else
		f->writeUint32BE(val - 1);
}

bool AGOSEngine::loadGame(const char *filename, bool restartMode) {
	char ident[100];
	Common::SeekableReadStream *f = NULL;
	uint num, item_index, i;

	_lockWord |= 0x100;

	if (restartMode) {
		// Load restart state
		Common::File *file = new Common::File();
		file->open(filename, Common::File::kFileReadMode);
		f = file;
	} else {
		f = _saveFileMan->openForLoading(filename);
	}

	if (f == NULL) {
		_lockWord &= ~0x100;
		return false;
	}

	if (!restartMode) {
		f->read(ident, 8);
	}

	num = f->readUint32BE();

	if (f->readUint32BE() != 0xFFFFFFFF || num != _itemArrayInited - 1) {
		delete f;
		_lockWord &= ~0x100;
		return false;
	}

	f->readUint32BE();
	f->readUint32BE();
	_noParentNotify = true;

	// add all timers
	killAllTimers();
	for (num = f->readUint32BE(); num; num--) {
		uint32 timeout = f->readUint32BE();
		uint16 subroutine_id = f->readUint16BE();
		addTimeEvent(timeout, subroutine_id);
	}

	item_index = 1;
	for (num = _itemArrayInited - 1; num; num--) {
		Item *item = _itemArrayPtr[item_index++], *parent_item;

		parent_item = derefItem(readItemID(f));
		setItemParent(item, parent_item);

		item->state = f->readUint16BE();
		item->classFlags = f->readUint16BE();

		SubObject *o = (SubObject *)findChildOfType(item, 2);
		if (o) {
			o->objectSize = f->readUint16BE();
			o->objectWeight = f->readUint16BE();
		}

		SubPlayer *p = (SubPlayer *)findChildOfType(item, 3);
		if (p) {
			p->score = f->readUint32BE();
			p->level = f->readUint16BE();
			p->size = f->readUint16BE();
			p->weight = f->readUint16BE();
			p->strength = f->readUint16BE();
		}

		SubUserFlag *u = (SubUserFlag *) findChildOfType(item, 9);
		if (u) {
			for (i = 0; i != 8; i++) {
				u->userFlags[i] = f->readUint16BE();
			}
			u->userItems[0] = readItemID(f);
		}
	}

	// read the variables
	for (i = 0; i != _numVars; i++) {
		writeVariable(i, f->readUint16BE());
	}

	if (f->ioFailed()) {
		error("load failed");
	}

	delete f;

	_noParentNotify = false;

	_lockWord &= ~0x100;

	return true;
}

bool AGOSEngine::saveGame(uint slot, const char *caption) {
	Common::OutSaveFile *f;
	uint item_index, num_item, i;
	TimeEvent *te;
	uint32 curTime = 0;
	uint32 gsc = _gameStoppedClock;

	_lockWord |= 0x100;

	f = _saveFileMan->openForSaving(genSaveName(slot));
	if (f == NULL) {
		warning("saveGame: Failed to save slot %d", slot);
		_lockWord &= ~0x100;
		return false;
	}

	f->write(caption, 8);

	f->writeUint32BE(_itemArrayInited - 1);
	f->writeUint32BE(0xFFFFFFFF);
	f->writeUint32BE(0);
	f->writeUint32BE(0);

	i = 0;
	for (te = _firstTimeStruct; te; te = te->next)
		i++;
	f->writeUint32BE(i);

	for (te = _firstTimeStruct; te; te = te->next) {
		f->writeUint32BE(te->time - curTime + gsc);
		f->writeUint16BE(te->subroutine_id);
	}

	item_index = 1;
	for (num_item = _itemArrayInited - 1; num_item; num_item--) {
		Item *item = _itemArrayPtr[item_index++];

		writeItemID(f, item->parent);

		f->writeUint16BE(item->state);
		f->writeUint16BE(item->classFlags);

		SubObject *o = (SubObject *)findChildOfType(item, 2);
		if (o) {
			f->writeUint16BE(o->objectSize);
			f->writeUint16BE(o->objectWeight);
		}

		SubPlayer *p = (SubPlayer *)findChildOfType(item, 3);
		if (p) {
			f->writeUint32BE(p->score);
			f->writeUint16BE(p->level);
			f->writeUint16BE(p->size);
			f->writeUint16BE(p->weight);
			f->writeUint16BE(p->strength);
		}

		SubUserFlag *u = (SubUserFlag *) findChildOfType(item, 9);
		if (u) {
			for (i = 0; i != 8; i++) {
				f->writeUint16BE(u->userFlags[i]);
			}
			writeItemID(f, u->userItems[0]);
		}
	}

	// write the variables
	for (i = 0; i != _numVars; i++) {
		f->writeUint16BE(readVariable(i));
	}

	f->finalize();
	bool result = !f->ioFailed();

	delete f;
	_lockWord &= ~0x100;

	return result;
}

bool AGOSEngine_Elvira2::loadGame(const char *filename, bool restartMode) {
	char ident[100];
	Common::SeekableReadStream *f = NULL;
	uint num, item_index, i, j;

	_lockWord |= 0x100;

	if (restartMode) {
		// Load restart state
		Common::File *file = new Common::File();
		file->open(filename, Common::File::kFileReadMode);
		f = file;
	} else {
		f = _saveFileMan->openForLoading(filename);
	}

	if (f == NULL) {
		warning("loadGame: Failed to load %s", filename);
		_lockWord &= ~0x100;
		return false;
	}

	if (getGameType() == GType_PP) {
		// No caption
	} else if (getGameType() == GType_FF) {
		f->read(ident, 100);
	} else if (getGameType() == GType_SIMON1 || getGameType() == GType_SIMON2) {
		f->read(ident, 18);
	} else if (!restartMode) {
		f->read(ident, 8);
	}

	num = f->readUint32BE();

	if (f->readUint32BE() != 0xFFFFFFFF || num != _itemArrayInited - 1) {
		delete f;
		_lockWord &= ~0x100;
		return false;
	}

	f->readUint32BE();
	f->readUint32BE();
	_noParentNotify = true;

	// add all timers
	killAllTimers();
	for (num = f->readUint32BE(); num; num--) {
		uint32 timeout = f->readUint32BE();
		uint16 subroutine_id = f->readUint16BE();
		addTimeEvent(timeout, subroutine_id);
	}

	if (getGameType() == GType_WW) {
		// TODO Load room state data
		for (uint s = 0; s <= _numRoomStates; s++) {
			f->readUint16BE();
		}
		f->readUint16BE();
	}

	item_index = 1;
	for (num = _itemArrayInited - 1; num; num--) {
		Item *item = _itemArrayPtr[item_index++], *parent_item;

		if (getGameType() == GType_ELVIRA2) {
			parent_item = derefItem(readItemID(f));
			setItemParent(item, parent_item);
		} else {
			uint parent = f->readUint16BE();
			uint next = f->readUint16BE();

			parent_item = derefItem(parent);
			setItemParent(item, parent_item);

			if (parent_item == NULL) {
				item->parent = parent;
				item->next = next;
			}
		}

		item->state = f->readUint16BE();
		item->classFlags = f->readUint16BE();

		SubRoom *r = (SubRoom *)findChildOfType(item, 1);
		if (r) {
			r->roomExitStates = f->readUint16BE();
		}

		SubSuperRoom *sr = (SubSuperRoom *)findChildOfType(item, 4);
		if (sr) {
			uint16 n = sr->roomX * sr->roomY * sr->roomZ;
 			for (i = j = 0; i != n; i++)
				sr->roomExitStates[j++] = f->readUint16BE();
		}

		SubObject *o = (SubObject *)findChildOfType(item, 2);
		if (o) {
			o->objectFlags = f->readUint32BE();
			i = o->objectFlags & 1;

			for (j = 1; j < 16; j++) {
				if (o->objectFlags & (1 << j)) {
					o->objectFlagValue[i++] = f->readUint16BE();
				}
			}
		}

		SubUserFlag *u = (SubUserFlag *) findChildOfType(item, 9);
		if (u) {
			for (i = 0; i != 4; i++) {
				u->userFlags[i] = f->readUint16BE();
			}
		}
	}

	// read the variables
	for (i = 0; i != _numVars; i++) {
		writeVariable(i, f->readUint16BE());
	}

	// read the items in item store
	for (i = 0; i != _numItemStore; i++) {
		if (getGameType() == GType_ELVIRA2 && getPlatform() == Common::kPlatformPC) {
			_itemStore[i] = derefItem(readItemID(f));
		} else {
			_itemStore[i] = derefItem(f->readUint16BE());
		}
	}

	// Read the bits in array 1
	for (i = 0; i != _numBitArray1; i++)
		_bitArray[i] = f->readUint16BE();

	// Read the bits in array 2
	for (i = 0; i != _numBitArray2; i++)
		_bitArrayTwo[i] = f->readUint16BE();

	// Read the bits in array 3
	for (i = 0; i != _numBitArray3; i++)
		_bitArrayThree[i] = f->readUint16BE();

	if (getGameType() == GType_ELVIRA2 || getGameType() == GType_WW) {
		_superRoomNumber = f->readUint16BE();
	}

	if (f->ioFailed()) {
		error("load failed");
	}

	delete f;

	_noParentNotify = false;

	_lockWord &= ~0x100;

	return true;
}

bool AGOSEngine_Elvira2::saveGame(uint slot, const char *caption) {
	Common::OutSaveFile *f;
	uint item_index, num_item, i, j;
	TimeEvent *te;
	uint32 curTime = 0;
	uint32 gsc = _gameStoppedClock;

	_lockWord |= 0x100;

	f = _saveFileMan->openForSaving(genSaveName(slot));
	if (f == NULL) {
		warning("saveGame: Failed to save slot %d", slot);
		_lockWord &= ~0x100;
		return false;
	}

	if (getGameType() == GType_PP) {
		// No caption
	} else if (getGameType() == GType_FF) {
		f->write(caption, 100);
		curTime = time(NULL);
	} else if (getGameType() == GType_SIMON1 || getGameType() == GType_SIMON2) {
		f->write(caption, 18);
	} else {
		f->write(caption, 8);
	}

	f->writeUint32BE(_itemArrayInited - 1);
	f->writeUint32BE(0xFFFFFFFF);
	f->writeUint32BE(0);
	f->writeUint32BE(0);

	i = 0;
	for (te = _firstTimeStruct; te; te = te->next)
		i++;
	f->writeUint32BE(i);

	if (getGameType() == GType_FF && _clockStopped)
		gsc += ((uint32)time(NULL) - _clockStopped);
	for (te = _firstTimeStruct; te; te = te->next) {
		f->writeUint32BE(te->time - curTime + gsc);
		f->writeUint16BE(te->subroutine_id);
	}

	if (getGameType() == GType_WW) {
		// TODO Save room state data
		for (uint s = 0; s <= _numRoomStates; s++) {
			f->writeUint16BE(0);
		}
		f->writeUint16BE(_currentRoom);
	}

	item_index = 1;
	for (num_item = _itemArrayInited - 1; num_item; num_item--) {
		Item *item = _itemArrayPtr[item_index++];

		if (getGameType() == GType_ELVIRA2) {
			writeItemID(f, item->parent);
		} else {
			f->writeUint16BE(item->parent);
			f->writeUint16BE(item->next);
		}

		f->writeUint16BE(item->state);
		f->writeUint16BE(item->classFlags);

		SubRoom *r = (SubRoom *)findChildOfType(item, 1);
		if (r) {
			f->writeUint16BE(r->roomExitStates);
		}

		SubSuperRoom *sr = (SubSuperRoom *)findChildOfType(item, 4);
		if (sr) {
			uint16 n = sr->roomX * sr->roomY * sr->roomZ;
 			for (i = j = 0; i != n; i++)
				f->writeUint16BE(sr->roomExitStates[j++]);
		}

		SubObject *o = (SubObject *)findChildOfType(item, 2);
		if (o) {
			f->writeUint32BE(o->objectFlags);
			i = o->objectFlags & 1;

			for (j = 1; j < 16; j++) {
				if (o->objectFlags & (1 << j)) {
					f->writeUint16BE(o->objectFlagValue[i++]);
				}
			}
		}

		SubUserFlag *u = (SubUserFlag *)findChildOfType(item, 9);
		if (u) {
			for (i = 0; i != 4; i++) {
				f->writeUint16BE(u->userFlags[i]);
			}
		}
	}

	// write the variables
	for (i = 0; i != _numVars; i++) {
		f->writeUint16BE(readVariable(i));
	}

	// write the items in item store
	for (i = 0; i != _numItemStore; i++) {
		if (getGameType() == GType_ELVIRA2 && getPlatform() == Common::kPlatformPC) {
			writeItemID(f, itemPtrToID(_itemStore[i]));
		} else {
			f->writeUint16BE(itemPtrToID(_itemStore[i]));
		}
	}

	// Write the bits in array 1
	for (i = 0; i != _numBitArray1; i++)
		f->writeUint16BE(_bitArray[i]);

	// Write the bits in array 2
	for (i = 0; i != _numBitArray2; i++)
		f->writeUint16BE(_bitArrayTwo[i]);

	// Write the bits in array 3
	for (i = 0; i != _numBitArray3; i++)
		f->writeUint16BE(_bitArrayThree[i]);

	if (getGameType() == GType_ELVIRA2 || getGameType() == GType_WW) {
		f->writeUint16BE(_superRoomNumber);
	}

	f->finalize();
	bool result = !f->ioFailed();

	delete f;
	_lockWord &= ~0x100;

	return result;
}

} // End of namespace AGOS
