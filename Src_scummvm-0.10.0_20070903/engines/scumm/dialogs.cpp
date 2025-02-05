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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/engines/scumm/dialogs.cpp $
 * $Id: dialogs.cpp 27068 2007-06-03 17:32:42Z fingolfin $
 */

#include "common/stdafx.h"

#include "common/config-manager.h"
#include "common/savefile.h"
#include "common/system.h"
#include "common/events.h"

#include "graphics/scaler.h"

#ifdef __DS__
#include "scummhelp.h"
#endif

#include "gui/about.h"
#include "gui/chooser.h"
#include "gui/eval.h"
#include "gui/newgui.h"
#include "gui/ListWidget.h"

#include "scumm/dialogs.h"
#include "scumm/sound.h"
#include "scumm/scumm.h"
#include "scumm/imuse/imuse.h"
#include "scumm/imuse_digi/dimuse.h"
#include "scumm/player_v2.h"
#include "scumm/verbs.h"
#include "sound/mididrv.h"
#include "sound/mixer.h"

#ifndef DISABLE_HELP
#include "scumm/help.h"
#endif

#ifdef SMALL_SCREEN_DEVICE
#include "gui/KeysDialog.h"
#endif

using GUI::CommandSender;
using GUI::StaticTextWidget;
using GUI::kButtonWidth;
using GUI::kButtonHeight;
using GUI::kBigButtonWidth;
using GUI::kBigButtonHeight;
using GUI::kCloseCmd;
using GUI::kTextAlignCenter;
using GUI::kTextAlignLeft;
using GUI::WIDGET_ENABLED;

typedef GUI::OptionsDialog GUI_OptionsDialog;
typedef GUI::ChooserDialog GUI_ChooserDialog;
typedef GUI::Dialog GUI_Dialog;

namespace Scumm {

struct ResString {
	int num;
	char string[80];
};

#ifdef PALMOS_68K
static const ResString *string_map_table_v7;
static const ResString *string_map_table_v6;
static const ResString *string_map_table_v5;
#else
static const ResString string_map_table_v8[] = {
	{0, "/BT_100/Please insert disk %d. Press ENTER"},
	{0, "/BT__003/Unable to Find %s, (%s %d) Press Button."},
	{0, "/BT__004/Error reading disk %c, (%c%d) Press Button."},
	{0, "/BT__002/Game Paused.  Press SPACE to Continue."},
	{0, "/BT__005/Are you sure you want to restart?  (Y/N)"}, //BOOT.004
	{0, "/BT__006/Are you sure you want to quit?  (Y/N)"}, //BOOT.005
	{0, "/BT__008/Save"},
	{0, "/BT__009/Load"},
	{0, "/BT__010/Play"},
	{0, "/BT__011/Cancel"},
	{0, "/BT__012/Quit"},
	{0, "/BT__013/OK"},
	{0, ""},
	{0, "/BT__014/You must enter a name"},
	{0, "/BT__015/The game was NOT saved (disk full?)"},
	{0, "/BT__016/The game was NOT loaded"},
	{0, "/BT__017/Saving '%s'"},
	{0, "/BT__018/Loading '%s'"},
	{0, "/BT__019/Name your SAVE game"},
	{0, "/BT__020/Select a game to LOAD"}
};

static const ResString string_map_table_v7[] = {
	{96, "game name and version"}, //that's how it's supposed to be
	{83, "Unable to Find %s, (%c%d) Press Button."},
	{84, "Error reading disk %c, (%c%d) Press Button."},
	{85, "/BOOT.003/Game Paused.  Press SPACE to Continue."},
	{86, "/BOOT.004/Are you sure you want to restart?  (Y/N)"},
	{87, "/BOOT.005/Are you sure you want to quit?  (Y/N)"},
	{70, "/BOOT.008/Save"},
	{71, "/BOOT.009/Load"},
	{72, "/BOOT.010/Play"},
	{73, "/BOOT.011/Cancel"}, 
	{74, "/BOOT.012/Quit"},
	{75, "/BOOT.013/OK"},
	{0, ""},
	{78, "/BOOT.014/You must enter a name"},
	{81, "/BOOT.015/The game was NOT saved (disk full?)"},
	{82, "/BOOT.016/The game was NOT loaded"},
	{79, "/BOOT.017/Saving '%s'"},
	{80, "/BOOT.018/Loading '%s'"},
	{76, "/BOOT.019/Name your SAVE game"},
	{77, "/BOOT.020/Select a game to LOAD"}

	/* This is the complete string map for v7
	{68, "/BOOT.007/c:\\dig"},
	{69, "/BOOT.021/The Dig"},
	{70, "/BOOT.008/Save"},
	{71, "/BOOT.009/Load"},
	{72, "/BOOT.010/Play"},
	{73, "/BOOT.011/Cancel"},
	{74, "/BOOT.012/Quit"},
	{75, "/BOOT.013/OK"},
	{76, "/BOOT.019/Name your SAVE game"},
	{77, "/BOOT.020/Select a game to LOAD"},
	{78, "/BOOT.014/You must enter a name"},
	{79, "/BOOT.017/Saving '%s'"},
	{80, "/BOOT.018/Loading '%s'"},
	{81, "/BOOT.015/The game was NOT saved (disk full?)"},
	{82, "/BOOT.016/The game was NOT loaded"},
	{83, "Unable to Find %s, (%c%d) Press Button."},
	{84, "Error reading disk %c, (%c%d) Press Button."},
	{85, "/BOOT.003/Game Paused.  Press SPACE to Continue."},
	{86, "/BOOT.004/Are you sure you want to restart?  (Y/N)"},
	{87, "/BOOT.005/Are you sure you want to quit?  (Y/N)"},
	{90, "/BOOT.022/Music"},
	{91, "/BOOT.023/Voice"},
	{92, "/BOOT.024/Sfx"},
	{93, "/BOOT.025/disabled"},
	{94, "/BOOT.026/Text speed"},
	{95, "/BOOT.027/Display Text"},
	{96, "The Dig v1.0"},
	{138, "/BOOT.028/Spooled Music"),
	{139, "/BOOT.029/Do you want to replace this saved game?  (Y/N)"},
	{141, "Voice Only"},
	{142, "Voice and Text"},
	{143, "Text Display Only"}, */

};

static const ResString string_map_table_v6[] = {
	{90, "Insert Disk %c and Press Button to Continue."},
	{91, "Unable to Find %s, (%c%d) Press Button."},
	{92, "Error reading disk %c, (%c%d) Press Button."},
	{93, "Game Paused.  Press SPACE to Continue."},
	{94, "Are you sure you want to restart?  (Y/N)"},
	{95, "Are you sure you want to quit?  (Y/N)"},
	{96, "Save"},
	{97, "Load"},
	{98, "Play"},
	{99, "Cancel"},
	{100, "Quit"},
	{101, "OK"},
	{102, "Insert save/load game disk"},
	{103, "You must enter a name"},
	{104, "The game was NOT saved (disk full?)"},
	{105, "The game was NOT loaded"},
	{106, "Saving '%s'"},
	{107, "Loading '%s'"},
	{108, "Name your SAVE game"},
	{109, "Select a game to LOAD"},
	{117, "How may I serve you?"}
};

static const ResString string_map_table_v345[] = {
	{1, "Insert Disk %c and Press Button to Continue."},
	{2, "Unable to Find %s, (%c%d) Press Button."},
	{3, "Error reading disk %c, (%c%d) Press Button."},
	{4, "Game Paused.  Press SPACE to Continue."},
	{5, "Are you sure you want to restart?  (Y/N)"},
	{6, "Are you sure you want to quit?  (Y/N)"},

	// Added in SCUMM4
	{7, "Save"},
	{8, "Load"},
	{9, "Play"},
	{10, "Cancel"},
	{11, "Quit"},
	{12, "OK"},
	{13, "Insert save/load game disk"},
	{14, "You must enter a name"},
	{15, "The game was NOT saved (disk full?)"},
	{16, "The game was NOT loaded"},
	{17, "Saving '%s'"},
	{18, "Loading '%s'"},
	{19, "Name your SAVE game"},
	{20, "Select a game to LOAD"},
	{28, "Game title"}
};
#endif

#pragma mark -

ScummDialog::ScummDialog(String name)
	: GUI::Dialog(name) {
_drawingHints |= GUI::THEME_HINT_SPECIAL_COLOR;
}

#pragma mark -

Common::StringList generateSavegameList(ScummEngine *scumm, bool saveMode);

enum {
	kSaveCmd = 'SAVE',
	kLoadCmd = 'LOAD',
	kPlayCmd = 'PLAY',
	kOptionsCmd = 'OPTN',
	kHelpCmd = 'HELP',
	kAboutCmd = 'ABOU',
	kQuitCmd = 'QUIT',
	kChooseCmd = 'CHOS'
};

SaveLoadChooser::SaveLoadChooser(const String &title, const String &buttonLabel, bool saveMode, ScummEngine *engine)
	: Dialog("scummsaveload"), _saveMode(saveMode), _list(0), _chooseButton(0), _gfxWidget(0), _vm(engine) {

	_drawingHints |= GUI::THEME_HINT_SPECIAL_COLOR;

	new StaticTextWidget(this, "scummsaveload_title", title);

	// Add choice list
	_list = new GUI::ListWidget(this, "scummsaveload_list");
	_list->setEditable(saveMode);
	_list->setNumberingMode(saveMode ? GUI::kListNumberingOne : GUI::kListNumberingZero);

	_container = new GUI::ContainerWidget(this, 0, 0, 10, 10);
	_container->setHints(GUI::THEME_HINT_USE_SHADOW);

	_gfxWidget = new GUI::GraphicsWidget(this, 0, 0, 10, 10);
	
	_date = new StaticTextWidget(this, 0, 0, 10, 10, "No date saved", kTextAlignCenter);
	_time = new StaticTextWidget(this, 0, 0, 10, 10, "No time saved", kTextAlignCenter);
	_playtime = new StaticTextWidget(this, 0, 0, 10, 10, "No playtime saved", kTextAlignCenter);

	// Buttons
	new GUI::ButtonWidget(this, "scummsaveload_cancel", "Cancel", kCloseCmd, 0);
	_chooseButton = new GUI::ButtonWidget(this, "scummsaveload_choose", buttonLabel, kChooseCmd, 0);
	_chooseButton->setEnabled(false);
}

SaveLoadChooser::~SaveLoadChooser() {
}

const Common::String &SaveLoadChooser::getResultString() const {
	return _list->getSelectedString();
}

void SaveLoadChooser::setList(const StringList& list) {
	_list->setList(list);
}

int SaveLoadChooser::runModal() {
	if (_gfxWidget)
		_gfxWidget->setGfx(0);
	int ret = Dialog::runModal();
	return ret;
}

void SaveLoadChooser::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	int selItem = _list->getSelected();
	switch (cmd) {
	case GUI::kListItemActivatedCmd:
	case GUI::kListItemDoubleClickedCmd:
		if (selItem >= 0) {
			if (_saveMode || !getResultString().empty()) {
				_list->endEditMode();
				setResult(selItem);
				close();
			}
		}
		break;
	case kChooseCmd:
		_list->endEditMode();
		setResult(selItem);
		close();
		break;
	case GUI::kListSelectionChangedCmd: {
		if (_gfxWidget) {
			updateInfos();
		}

		if (_saveMode) {
			_list->startEditMode();
		}
		// Disable button if nothing is selected, or (in load mode) if an empty
		// list item is selected. We allow choosing an empty item in save mode
		// because we then just assign a default name.
		_chooseButton->setEnabled(selItem >= 0 && (_saveMode || !getResultString().empty()));
		_chooseButton->draw();
	} break;
	case kCloseCmd:
		setResult(-1);
	default:
		Dialog::handleCommand(sender, cmd, data);
	}
}

void SaveLoadChooser::reflowLayout() {
	if (g_gui.evaluator()->getVar("scummsaveload_extinfo.visible") == 1) {
		int thumbX = g_gui.evaluator()->getVar("scummsaveload_thumbnail.x");
		int thumbY = g_gui.evaluator()->getVar("scummsaveload_thumbnail.y");
		int hPad = g_gui.evaluator()->getVar("scummsaveload_thumbnail.hPad");
		int vPad = g_gui.evaluator()->getVar("scummsaveload_thumbnail.vPad");
		int thumbH = ((g_system->getHeight() % 200 && g_system->getHeight() != 350) ? kThumbnailHeight2 : kThumbnailHeight1);

		_container->resize(thumbX - hPad, thumbY - vPad, kThumbnailWidth + hPad * 2, thumbH + vPad * 2 + kLineHeight * 4);
	
		// Add the thumbnail display
		_gfxWidget->resize(thumbX, thumbY, kThumbnailWidth, thumbH);
	
		int height = thumbY + thumbH + kLineHeight;

		_date->resize(thumbX, height, kThumbnailWidth, kLineHeight);
	
		height += kLineHeight;

		_time->resize(thumbX, height, kThumbnailWidth, kLineHeight);
	
		height += kLineHeight;

		_playtime->resize(thumbX, height, kThumbnailWidth, kLineHeight);
	
		_container->clearFlags(GUI::WIDGET_INVISIBLE);
		_gfxWidget->clearFlags(GUI::WIDGET_INVISIBLE);
		_date->clearFlags(GUI::WIDGET_INVISIBLE);
		_time->clearFlags(GUI::WIDGET_INVISIBLE);
		_playtime->clearFlags(GUI::WIDGET_INVISIBLE);

		_fillR = g_gui.evaluator()->getVar("scummsaveload_thumbnail.fillR");
		_fillG = g_gui.evaluator()->getVar("scummsaveload_thumbnail.fillG");
		_fillB = g_gui.evaluator()->getVar("scummsaveload_thumbnail.fillB");
		updateInfos();
	} else {
		_container->setFlags(GUI::WIDGET_INVISIBLE);
		_gfxWidget->setFlags(GUI::WIDGET_INVISIBLE);
		_date->setFlags(GUI::WIDGET_INVISIBLE);
		_time->setFlags(GUI::WIDGET_INVISIBLE);
		_playtime->setFlags(GUI::WIDGET_INVISIBLE);
	}

	Dialog::reflowLayout();
}

void SaveLoadChooser::updateInfos() {
	int selItem = _list->getSelected();
	Graphics::Surface *thumb;
	thumb = _vm->loadThumbnailFromSlot(_saveMode ? selItem + 1 : selItem);

	if (thumb) {
		_gfxWidget->setGfx(thumb);
		_gfxWidget->useAlpha(256);
		thumb->free();
	} else {
		_gfxWidget->setGfx(-1, -1, _fillR, _fillG, _fillB);
	}

	delete thumb;
	_gfxWidget->draw();

	InfoStuff infos;
	memset(&infos, 0, sizeof(InfoStuff));
	char buffer[32];
	if (_vm->loadInfosFromSlot(_saveMode ? selItem + 1 : selItem, &infos)) {
		snprintf(buffer, 32, "Date: %.2d.%.2d.%.4d",
			(infos.date >> 24) & 0xFF, (infos.date >> 16) & 0xFF,
			infos.date & 0xFFFF);
		_date->setLabel(buffer);
		_date->draw();

		snprintf(buffer, 32, "Time: %.2d:%.2d",
			(infos.time >> 8) & 0xFF, infos.time & 0xFF);
		_time->setLabel(buffer);
		_time->draw();

		int minutes = infos.playtime / 60;
		int hours = minutes / 60;
		minutes %= 60;

		snprintf(buffer, 32, "Playtime: %.2d:%.2d",
			hours & 0xFF, minutes & 0xFF);
		_playtime->setLabel(buffer);
		_playtime->draw();
	} else {
		snprintf(buffer, 32, "No date saved");
		_date->setLabel(buffer);
		_date->draw();

		snprintf(buffer, 32, "No time saved");
		_time->setLabel(buffer);
		_time->draw();

		snprintf(buffer, 32, "No playtime saved");
		_playtime->setLabel(buffer);
		_playtime->draw();
	}
}

#pragma mark -

Common::StringList generateSavegameList(ScummEngine *scumm, bool saveMode) {
	// Get savegame names
	Common::StringList l;
	char name[32];
	uint i = saveMode ? 1 : 0;
	bool avail_saves[81];

	scumm->listSavegames(avail_saves, ARRAYSIZE(avail_saves));
	for (; i < ARRAYSIZE(avail_saves); i++) {
		if (avail_saves[i])
			scumm->getSavegameName(i, name);
		else
			name[0] = 0;
		l.push_back(name);
	}

	return l;
}

MainMenuDialog::MainMenuDialog(ScummEngine *scumm)
	: ScummDialog("scummmain"), _vm(scumm) {

	new GUI::ButtonWidget(this, "scummmain_resume", "Resume", kPlayCmd, 'P');

	new GUI::ButtonWidget(this, "scummmain_load", "Load", kLoadCmd, 'L');
	new GUI::ButtonWidget(this, "scummmain_save", "Save", kSaveCmd, 'S');

	new GUI::ButtonWidget(this, "scummmain_options", "Options", kOptionsCmd, 'O');
#ifndef DISABLE_HELP
	new GUI::ButtonWidget(this, "scummmain_help", "Help", kHelpCmd, 'H');
#endif
	new GUI::ButtonWidget(this, "scummmain_about", "About", kAboutCmd, 'A');

	new GUI::ButtonWidget(this, "scummmain_quit", "Quit", kQuitCmd, 'Q');

	//
	// Create the sub dialog(s)
	//
	_aboutDialog = new GUI::AboutDialog();
	_optionsDialog = new ConfigDialog();
#ifndef DISABLE_HELP
	_helpDialog = new HelpDialog(scumm->_game);
#endif
	_saveDialog = new SaveLoadChooser("Save game:", "Save", true, scumm);
	_loadDialog = new SaveLoadChooser("Load game:", "Load", false, scumm);
}

MainMenuDialog::~MainMenuDialog() {
	delete _aboutDialog;
	delete _optionsDialog;
#ifndef DISABLE_HELP
	delete _helpDialog;
#endif
	delete _saveDialog;
	delete _loadDialog;
}

void MainMenuDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
	case kSaveCmd:
		save();
		break;
	case kLoadCmd:
		load();
		break;
	case kPlayCmd:
		close();
		break;
	case kOptionsCmd:
		_optionsDialog->runModal();
		break;
	case kAboutCmd:
		_aboutDialog->runModal();
		break;
#ifndef DISABLE_HELP
	case kHelpCmd:
		_helpDialog->runModal();
		break;
#endif
	case kQuitCmd:
		_vm->_quit = true;
		close();
		break;
	default:
		ScummDialog::handleCommand(sender, cmd, data);
	}
}

void MainMenuDialog::save() {
	int idx;
	_saveDialog->setList(generateSavegameList(_vm, true));
	idx = _saveDialog->runModal();
	if (idx >= 0) {
		String result(_saveDialog->getResultString());
		char buffer[20];
		const char *str;
		if (result.empty()) {
			// If the user was lazy and entered no save name, come up with a default name.
			sprintf(buffer, "Save %d", idx + 1);
			str = buffer;
		} else
			str = result.c_str();
		_vm->requestSave(idx + 1, str);
		close();
	}
}

void MainMenuDialog::load() {
	int idx;
	_loadDialog->setList(generateSavegameList(_vm, false));
	idx = _loadDialog->runModal();
	if (idx >= 0) {
		_vm->requestLoad(idx);
		close();
	}
}

#pragma mark -

enum {
	kOKCmd = 'ok  '
};

enum {
	kKeysCmd = 'KEYS'
};

// FIXME: We use the empty string as domain name here. This tells the
// ConfigManager to use the 'default' domain for all its actions. We do that
// to get as close as possible to editing the 'active' settings.
//
// However, that requires bad & evil hacks in the ConfigManager code,
// and even then still doesn't work quite correctly.
// For example, if the transient domain contains 'false' for the 'fullscreen'
// flag, but the user used a hotkey to switch to windowed mode, then the dialog
// will display the wrong value anyway.
//
// Proposed solution consisting of multiple steps:
// 1) Add special code to the open() code that reads out everything stored
//    in the transient domain that is controlled by this dialog, and updates
//    the dialog accordingly.
// 2) Even more code is added to query the backend for current settings, like
//    the fullscreen mode flag etc., and also updates the dialog accordingly.
// 3) The domain being edited is set to the active game domain.
// 4) If the dialog is closed with the "OK" button, then we remove everything
//    stored in the transient domain (or at least everything corresponding to
//    switches in this dialog.
//    If OTOH the dialog is closed with "Cancel" we do no such thing.
//
// These changes will achieve two things at once: Allow us to get rid of using
//  "" as value for the domain, and in fact provide a somewhat better user 
// experience at the same time.
ConfigDialog::ConfigDialog()
	: GUI::OptionsDialog("", "scummconfig") {

	//
	// Sound controllers
	//

	addVolumeControls(this, "scummconfig_");

	//
	// Some misc options
	//

	// SCUMM has a talkspeed range of 0-9
	addSubtitleControls(this, "scummconfig_", 9);

	//
	// Add the buttons
	//

	new GUI::ButtonWidget(this, "scummconfig_ok", "OK", GUI::OptionsDialog::kOKCmd, 'O');
	new GUI::ButtonWidget(this, "scummconfig_cancel", "Cancel", kCloseCmd, 'C');
#ifdef SMALL_SCREEN_DEVICE
	new GUI::ButtonWidget(this, "scummconfig_keys", "Keys", kKeysCmd, 'K');
#endif

#ifdef SMALL_SCREEN_DEVICE
	//
	// Create the sub dialog(s)
	//

	_keysDialog = new GUI::KeysDialog();
#endif
}

ConfigDialog::~ConfigDialog() {
#ifdef SMALL_SCREEN_DEVICE
	delete _keysDialog;
#endif
}

void ConfigDialog::open() {
	GUI_OptionsDialog::open();
}

void ConfigDialog::close() {
	GUI_OptionsDialog::close();
}

void ConfigDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
	case kKeysCmd:
#ifdef SMALL_SCREEN_DEVICE
		_keysDialog->runModal();
#endif
		break;
	default:
		GUI_OptionsDialog::handleCommand (sender, cmd, data);
	}
}

#ifndef DISABLE_HELP

#pragma mark -

enum {
	kNextCmd = 'NEXT',
	kPrevCmd = 'PREV'
};

HelpDialog::HelpDialog(const GameSettings &game)
	: ScummDialog("scummhelp"), _game(game) {
	_title = new StaticTextWidget(this, "scummhelp_title", "");

	_page = 1;

	_numPages = ScummHelp::numPages(_game.id);

	_prevButton = new GUI::ButtonWidget(this, "scummhelp_prev", "Previous", kPrevCmd, 'P');
	_nextButton = new GUI::ButtonWidget(this, "scummhelp_next", "Next", kNextCmd, 'N');
	new GUI::ButtonWidget(this, "scummhelp_close", "Close", kCloseCmd, 'C');
	_prevButton->clearFlags(WIDGET_ENABLED);

	// Dummy entries
	for (int i = 0; i < HELP_NUM_LINES; i++) {
		_key[i] = new StaticTextWidget(this, 0, 0, 10, 10, "", kTextAlignLeft);
		_dsc[i] = new StaticTextWidget(this, 0, 0, 10, 10, "", kTextAlignLeft);
	}

}

void HelpDialog::reflowLayout() {
	ScummDialog::reflowLayout();

	_drawingHints &= ~GUI::THEME_HINT_SPECIAL_COLOR;

	int lineHeight = g_gui.getFontHeight();

	int keyX = g_gui.evaluator()->getVar("scummhelp_key.x");
	int keyYoff = g_gui.evaluator()->getVar("scummhelp_key.yoffset");
	int keyW = g_gui.evaluator()->getVar("scummhelp_key.w");
	int keyH = g_gui.evaluator()->getVar("scummhelp_key.h");
	int dscX = g_gui.evaluator()->getVar("scummhelp_dsc.x");
	int dscYoff = g_gui.evaluator()->getVar("scummhelp_dsc.yoffset");
	int dscW = g_gui.evaluator()->getVar("scummhelp_dsc.w");
	int dscH = g_gui.evaluator()->getVar("scummhelp_dsc.h");

	for (int i = 0; i < HELP_NUM_LINES; i++) {
		_key[i]->resize(keyX, keyYoff + lineHeight * (i + 2), keyW, keyH);
		_dsc[i]->resize(dscX, dscYoff + lineHeight * (i + 2), dscW, dscH);
	}

	displayKeyBindings();
}

void HelpDialog::displayKeyBindings() {

	String titleStr, *keyStr, *dscStr;

#ifndef __DS__
	ScummHelp::updateStrings(_game.id, _game.version, _game.platform, _page, titleStr, keyStr, dscStr);
#else
	// DS version has a different help screen
	DS::updateStrings(_game.id, _game.version, _game.platform, _page, titleStr, keyStr, dscStr);
#endif

	_title->setLabel(titleStr);
	for (int i = 0; i < HELP_NUM_LINES; i++) {
		_key[i]->setLabel(keyStr[i]);
		_dsc[i]->setLabel(dscStr[i]);
	}

	delete [] keyStr;
	delete [] dscStr;
}

void HelpDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {

	switch (cmd) {
	case kNextCmd:
		_page++;
		if (_page >= _numPages) {
			_nextButton->clearFlags(WIDGET_ENABLED);
		}
		if (_page >= 2) {
			_prevButton->setFlags(WIDGET_ENABLED);
		}
		displayKeyBindings();
		draw();
		break;
	case kPrevCmd:
		_page--;
		if (_page <= _numPages) {
			_nextButton->setFlags(WIDGET_ENABLED);
		}
		if (_page <= 1) {
			_prevButton->clearFlags(WIDGET_ENABLED);
		}
		displayKeyBindings();
		draw();
		break;
	default:
		ScummDialog::handleCommand(sender, cmd, data);
	}
}

#endif

#pragma mark -

InfoDialog::InfoDialog(ScummEngine *scumm, int res)
: ScummDialog("scummDummyDialog"), _vm(scumm) { // dummy x and w

	_message = queryResString(res);

	// Width and height are dummy
	_text = new StaticTextWidget(this, 4, 4, 10, 10, _message, kTextAlignCenter);
}

InfoDialog::InfoDialog(ScummEngine *scumm, const String& message)
: ScummDialog("scummDummyDialog"), _vm(scumm) { // dummy x and w

	_message = message;

	// Width and height are dummy
	_text = new StaticTextWidget(this, 4, 4, 10, 10, _message, kTextAlignCenter);
}

void InfoDialog::setInfoText(const String& message) {
	_message = message;
	_text->setLabel(_message);
	//reflowLayout(); // FIXME: Should we call this here? Depends on the usage patterns, I guess...
}

void InfoDialog::reflowLayout() {
	const int screenW = g_system->getOverlayWidth();
	const int screenH = g_system->getOverlayHeight();

	int width = g_gui.getStringWidth(_message) + 16;
	int height = g_gui.getFontHeight() + 8;

	_w = width;
	_h = height;
	_x = (screenW - width) / 2;
	_y = (screenH - height) / 2;

	_text->setSize(_w - 8, _h);
}

const Common::String InfoDialog::queryResString(int stringno) {
	byte buf[256];
	const byte *result;

	if (stringno == 0)
		return String();

	if (_vm->_game.version == 8)
		result = (const byte *)string_map_table_v8[stringno - 1].string;
	else if (_vm->_game.version == 7)
		result = _vm->getStringAddressVar(string_map_table_v7[stringno - 1].num);
	else if (_vm->_game.version == 6)
		result = _vm->getStringAddressVar(string_map_table_v6[stringno - 1].num);
	else if (_vm->_game.version >= 3)
		result = _vm->getStringAddress(string_map_table_v345[stringno - 1].num);
	else
		return string_map_table_v345[stringno - 1].string;

	if (result && *result == '/') {
		_vm->translateText(result, buf);
		result = buf;
	}

	if (!result || *result == '\0') {	// Gracelessly degrade to english :)
		return string_map_table_v345[stringno - 1].string;
	}

	// Convert to a proper string (take care of FF codes)
	byte chr;
	String tmp;
	while ((chr = *result++)) {
		if (chr == 0xFF) {
			result += 3;
		} else if (chr != '@') {
			tmp += chr;
		}
	}
	return tmp;
}

#pragma mark -

PauseDialog::PauseDialog(ScummEngine *scumm, int res)
	: InfoDialog(scumm, res) {
}

void PauseDialog::handleKeyDown(uint16 ascii, int keycode, int modifiers) {
	if (ascii == ' ')  // Close pause dialog if space key is pressed
		close();
	else
		ScummDialog::handleKeyDown(ascii, keycode, modifiers);
}

ConfirmDialog::ConfirmDialog(ScummEngine *scumm, int res)
	: InfoDialog(scumm, res) {
}

void ConfirmDialog::handleKeyDown(uint16 ascii, int keycode, int modifiers) {
	if (tolower(ascii) == 'n') {
		setResult(0);
		close();
	} else if (tolower(ascii) == 'y') {
		setResult(1);
		close();
	} else
		ScummDialog::handleKeyDown(ascii, keycode, modifiers);
}

#pragma mark -

ValueDisplayDialog::ValueDisplayDialog(const Common::String& label, int minVal, int maxVal,
		int val, uint16 incKey, uint16 decKey)
	: GUI::Dialog("scummDummyDialog", false),
	_label(label), _min(minVal), _max(maxVal),
	_value(val), _incKey(incKey), _decKey(decKey) {
	assert(_min <= _value && _value <= _max);
}

void ValueDisplayDialog::drawDialog() {
	const int labelWidth = _w - 8 - _percentBarWidth;
	g_gui.theme()->drawDialogBackground(Common::Rect(_x, _y, _x+_w, _y+_h),
				GUI::THEME_HINT_SAVE_BACKGROUND | GUI::THEME_HINT_FIRST_DRAW);
	g_gui.theme()->drawText(Common::Rect(_x+4, _y+4, _x+labelWidth+4,
				_y+g_gui.theme()->getFontHeight()+4), _label);
	g_gui.theme()->drawSlider(Common::Rect(_x+4+labelWidth, _y+4, _x+_w-4, _y+_h-4),
				_percentBarWidth * (_value - _min) / (_max - _min));
}

void ValueDisplayDialog::handleTickle() {
	if (getMillis() > _timer) {
		close();
	}
}

void ValueDisplayDialog::reflowLayout() {
	const int screenW = g_system->getOverlayWidth();
	const int screenH = g_system->getOverlayHeight();

	if (g_gui.getWidgetSize() == GUI::kBigWidgetSize) {
		_percentBarWidth = kBigPercentBarWidth;
	} else {
		_percentBarWidth = kPercentBarWidth;
	}

	int width = g_gui.getStringWidth(_label) + 16 + _percentBarWidth;
	int height = g_gui.getFontHeight() + 4 * 2;

	_x = (screenW - width) / 2;
	_y = (screenH - height) / 2;
	_w = width;
	_h = height;
}

void ValueDisplayDialog::handleKeyDown(uint16 ascii, int keycode, int modifiers) {
	if (ascii == _incKey || ascii == _decKey) {
		if (ascii == _incKey && _value < _max)
			_value++;
		else if (ascii == _decKey && _value > _min)
			_value--;

		setResult(_value);
		_timer = getMillis() + kDisplayDelay;
		draw();
	} else {
		close();
	}
}

void ValueDisplayDialog::open() {
	GUI_Dialog::open();
	setResult(_value);
	_timer = getMillis() + kDisplayDelay;
}

SubtitleSettingsDialog::SubtitleSettingsDialog(ScummEngine *scumm, int value) 
	: InfoDialog(scumm, ""), _value(value) {

}

void SubtitleSettingsDialog::handleTickle() {
	InfoDialog::handleTickle();
	if (getMillis() > _timer)
		close();
}

void SubtitleSettingsDialog::handleKeyDown(uint16 ascii, int keycode, int modifiers) {
	if (keycode == 't' && modifiers == Common::KBD_CTRL) {
		cycleValue();

		reflowLayout();
		draw();
	} else {
		close();
	}
}

void SubtitleSettingsDialog::open() {
	cycleValue();
	InfoDialog::open();
}

void SubtitleSettingsDialog::cycleValue() {
	static const char* subtitleDesc[] = {
		"Speech Only",
		"Speech and Subtitles",
		"Subtitles Only"
	};
	
	_value = (_value + 1) % 3;

	setInfoText(subtitleDesc[_value]);

	setResult(_value);
	_timer = getMillis() + 1500;
}

Indy3IQPointsDialog::Indy3IQPointsDialog(ScummEngine *scumm, char* text)
	: InfoDialog(scumm, text) {
}

void Indy3IQPointsDialog::handleKeyDown(uint16 ascii, int keycode, int modifiers) {
	if (ascii == 'i') 
		close();
	else
		ScummDialog::handleKeyDown(ascii, keycode, modifiers);
}

} // End of namespace Scumm

#ifdef PALMOS_68K
#include "scumm_globals.h"

_GINIT(Dialogs)
_GSETPTR(Scumm::string_map_table_v7, GBVARS_STRINGMAPTABLEV7_INDEX, Scumm::ResString, GBVARS_SCUMM)
_GSETPTR(Scumm::string_map_table_v6, GBVARS_STRINGMAPTABLEV6_INDEX, Scumm::ResString, GBVARS_SCUMM)
_GSETPTR(Scumm::string_map_table_v5, GBVARS_STRINGMAPTABLEV5_INDEX, Scumm::ResString, GBVARS_SCUMM)
_GEND

_GRELEASE(Dialogs)
_GRELEASEPTR(GBVARS_STRINGMAPTABLEV7_INDEX, GBVARS_SCUMM)
_GRELEASEPTR(GBVARS_STRINGMAPTABLEV6_INDEX, GBVARS_SCUMM)
_GRELEASEPTR(GBVARS_STRINGMAPTABLEV5_INDEX, GBVARS_SCUMM)
_GEND

#endif
