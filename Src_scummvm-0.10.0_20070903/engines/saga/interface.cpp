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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/engines/saga/interface.cpp $
 * $Id: interface.cpp 27107 2007-06-05 13:19:47Z thebluegr $
 *
 */

// Game interface module
#include "saga/saga.h"

#include "saga/gfx.h"
#include "saga/actor.h"
#include "saga/console.h"
#include "saga/displayinfo.h"
#include "saga/events.h"
#include "saga/font.h"
#include "saga/objectmap.h"
#include "saga/isomap.h"
#include "saga/itedata.h"
#include "saga/music.h"
#include "saga/puzzle.h"
#include "saga/render.h"
#include "saga/scene.h"
#include "saga/script.h"
#include "saga/sound.h"
#include "saga/sprite.h"
#include "saga/rscfile.h"
#include "saga/sagaresnames.h"

#include "saga/interface.h"

#include "common/config-manager.h"
#include "common/system.h"
#include "common/timer.h"

namespace Saga {

static int verbTypeToTextStringsIdLUT[2][kVerbTypeIdsMax] = {
	{-1,
	kTextPickUp,
	kTextLookAt,
	kTextWalkTo,
	kTextTalkTo,
	kTextOpen,
	kTextClose,
	kTextGive,
	kTextUse,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1},
	{-1, 
	kVerbIHNMWalk,
	kVerbIHNMLookAt,
	kVerbIHNMTake, 
	kVerbIHNMUse, 
	kVerbIHNMTalkTo,
	kVerbIHNMSwallow,
	kVerbIHNMGive, 
	kVerbIHNMPush}
};

Interface::Interface(SagaEngine *vm) : _vm(vm) {
	byte *resource;
	size_t resourceLength;
	int i;

	// Load interface module resource file context
	_interfaceContext = _vm->_resource->getContext(GAME_RESOURCEFILE);
	if (_interfaceContext == NULL) {
		error("Interface::Interface() resource context not found");
	}

	_mainPanel.buttons = _vm->getDisplayInfo().mainPanelButtons;
	_mainPanel.buttonsCount = _vm->getDisplayInfo().mainPanelButtonsCount;

	for (i = 0; i < kVerbTypeIdsMax; i++) {
		_verbTypeToPanelButton[i] = NULL;
	}

	for (i = 0; i < _mainPanel.buttonsCount; i++) {
		if (_mainPanel.buttons[i].type == kPanelButtonVerb) {
			_verbTypeToPanelButton[_mainPanel.buttons[i].id] = &_mainPanel.buttons[i];
		}
	}

	_vm->_resource->loadResource(_interfaceContext, _vm->getResourceDescription()->mainPanelResourceId, resource, resourceLength);
	_vm->decodeBGImage(resource, resourceLength, &_mainPanel.image,
		&_mainPanel.imageLength, &_mainPanel.imageWidth, &_mainPanel.imageHeight);

	free(resource);

	_conversePanel.buttons = _vm->getDisplayInfo().conversePanelButtons;
	_conversePanel.buttonsCount = _vm->getDisplayInfo().conversePanelButtonsCount;

	_vm->_resource->loadResource(_interfaceContext, _vm->getResourceDescription()->conversePanelResourceId, resource, resourceLength);
	_vm->decodeBGImage(resource, resourceLength, &_conversePanel.image,
		&_conversePanel.imageLength, &_conversePanel.imageWidth, &_conversePanel.imageHeight);
	free(resource);

	_optionPanel.buttons = _vm->getDisplayInfo().optionPanelButtons;
	_optionPanel.buttonsCount = _vm->getDisplayInfo().optionPanelButtonsCount;

	_vm->_resource->loadResource(_interfaceContext, _vm->getResourceDescription()->optionPanelResourceId, resource, resourceLength);
	_vm->decodeBGImage(resource, resourceLength, &_optionPanel.image,
		&_optionPanel.imageLength, &_optionPanel.imageWidth, &_optionPanel.imageHeight);
	free(resource);

	_vm->_sprite->loadList(_vm->getResourceDescription()->mainPanelSpritesResourceId, _mainPanel.sprites);

	if (_vm->getGameType() == GType_ITE) {
		_vm->_sprite->loadList(_vm->getResourceDescription()->defaultPortraitsResourceId, _defPortraits);
	}

	setPortraitBgColor(0, 0, 0);

	_mainPanel.x = _vm->getDisplayInfo().mainPanelXOffset;
	_mainPanel.y = _vm->getDisplayInfo().mainPanelYOffset;
	_mainPanel.currentButton = NULL;
	_inventoryUpButton = _mainPanel.getButton(_vm->getDisplayInfo().inventoryUpButtonIndex);
	_inventoryDownButton = _mainPanel.getButton(_vm->getDisplayInfo().inventoryDownButtonIndex);

	_conversePanel.x = _vm->getDisplayInfo().conversePanelXOffset;
	_conversePanel.y = _vm->getDisplayInfo().conversePanelYOffset;
	_conversePanel.currentButton = NULL;
	_converseUpButton = _conversePanel.getButton(_vm->getDisplayInfo().converseUpButtonIndex);
	_converseDownButton = _conversePanel.getButton(_vm->getDisplayInfo().converseDownButtonIndex);

	_leftPortrait = 0;
	_rightPortrait = 0;

	_optionPanel.x = _vm->getDisplayInfo().optionPanelXOffset;
	_optionPanel.y = _vm->getDisplayInfo().optionPanelYOffset;
	_optionPanel.currentButton = NULL;
	_optionSaveFileSlider = _optionPanel.getButton(_vm->getDisplayInfo().optionSaveFileSliderIndex);
	_optionSaveFilePanel = _optionPanel.getButton(_vm->getDisplayInfo().optionSaveFilePanelIndex);

	_quitPanel.x = _vm->getDisplayInfo().quitPanelXOffset;
	_quitPanel.y = _vm->getDisplayInfo().quitPanelYOffset;
	_quitPanel.imageWidth = _vm->getDisplayInfo().quitPanelWidth;
	_quitPanel.imageHeight = _vm->getDisplayInfo().quitPanelHeight;
	_quitPanel.buttons = _vm->getDisplayInfo().quitPanelButtons;
	_quitPanel.buttonsCount = _vm->getDisplayInfo().quitPanelButtonsCount;
	_quitPanel.currentButton = NULL;

	_loadPanel.x = _vm->getDisplayInfo().loadPanelXOffset;
	_loadPanel.y = _vm->getDisplayInfo().loadPanelYOffset;
	_loadPanel.imageWidth = _vm->getDisplayInfo().loadPanelWidth;
	_loadPanel.imageHeight = _vm->getDisplayInfo().loadPanelHeight;
	_loadPanel.buttons = _vm->getDisplayInfo().loadPanelButtons;
	_loadPanel.buttonsCount = _vm->getDisplayInfo().loadPanelButtonsCount;
	_loadPanel.currentButton = NULL;

	_savePanel.x = _vm->getDisplayInfo().savePanelXOffset;
	_savePanel.y = _vm->getDisplayInfo().savePanelYOffset;
	_savePanel.imageWidth = _vm->getDisplayInfo().savePanelWidth;
	_savePanel.imageHeight = _vm->getDisplayInfo().savePanelHeight;
	_savePanel.buttons = _vm->getDisplayInfo().savePanelButtons;
	_savePanel.buttonsCount = _vm->getDisplayInfo().savePanelButtonsCount;
	_saveEdit = _savePanel.getButton(_vm->getDisplayInfo().saveEditIndex);
	_savePanel.currentButton = NULL;

	_protectPanel.x = _vm->getDisplayInfo().protectPanelXOffset;
	_protectPanel.y = _vm->getDisplayInfo().protectPanelYOffset;
	_protectPanel.imageWidth = _vm->getDisplayInfo().protectPanelWidth;
	_protectPanel.imageHeight = _vm->getDisplayInfo().protectPanelHeight;
	_protectPanel.buttons = _vm->getDisplayInfo().protectPanelButtons;
	_protectPanel.buttonsCount = _vm->getDisplayInfo().protectPanelButtonsCount;
	_protectEdit = _protectPanel.getButton(_vm->getDisplayInfo().protectEditIndex);
	_protectPanel.currentButton = NULL;

	_active = true;
	_panelMode = _lockedMode = kPanelNull;
	_savedMode = -1;
	_bossMode = -1;
	_fadeMode = kNoFade;
	_inMainMode = false;
	*_statusText = 0;
	_statusOnceColor = -1;

	_inventoryCount = 0;
	_inventoryPos = 0;
	_inventoryStart = 0;
	_inventoryEnd = 0;
	_inventoryBox = 0;
	_inventorySize = ITE_INVENTORY_SIZE;
	_saveReminderState = 0;

	_optionSaveFileTop = 0;
	_optionSaveFileTitleNumber = 0;

	_inventory = (uint16 *)calloc(_inventorySize, sizeof(uint16));
	if (_inventory == NULL) {
		error("Interface::Interface(): not enough memory");
	}

	_textInput = false;
	_statusTextInput = false;
	_statusTextInputState = kStatusTextInputFirstRun;

	_disableAbortSpeeches = false;
}

Interface::~Interface(void) {
	free(_inventory);

	_mainPanel.sprites.freeMem();
	_defPortraits.freeMem();
	_scenePortraits.freeMem();
}

int Interface::activate() {
	if (!_active) {
		_active = true;
		_vm->_script->_skipSpeeches = false;
		_vm->_actor->_protagonist->_targetObject = ID_NOTHING;
		unlockMode();
		if (_panelMode == kPanelMain){
			_saveReminderState = 1;
		}
		draw();
	}
	_vm->_gfx->showCursor(true);

	return SUCCESS;
}

int Interface::deactivate() {
	if (_active) {
		_active = false;
		lockMode();
		setMode(kPanelNull);
	}
	_vm->_gfx->showCursor(false);

	return SUCCESS;
}

void Interface::rememberMode() {
	assert (_savedMode == -1);

	_savedMode = _panelMode;
}

void Interface::restoreMode() {
	assert (_savedMode != -1);

	_panelMode = _savedMode;
	_savedMode = -1;

	draw();
}

void Interface::setMode(int mode) {
	debug(1, "Interface::setMode %i", mode);

	if (mode == kPanelMain) {
		_inMainMode = true;
		_saveReminderState = 1; //TODO: blinking timeout
	} else {
		if (mode == kPanelConverse) {
			_inMainMode = false;
		}
		_saveReminderState = 0;
	}

	_panelMode = mode;

	switch (_panelMode) {
	case kPanelMain:
		if (_vm->getGameType() == GType_IHNM)
			warning("FIXME: Implement IHNM differences from ExecuteInventoryPanel");

		_mainPanel.currentButton = NULL;
		break;
	case kPanelConverse:
		_conversePanel.currentButton = NULL;
		converseDisplayText();
		break;
	case kPanelOption:
		_optionPanel.currentButton = NULL;
		_vm->fillSaveList();
		calcOptionSaveSlider();
		if (_optionSaveFileTitleNumber >= _vm->getDisplayInfo().optionSaveFileVisible) {
			_optionSaveFileTitleNumber = _vm->getDisplayInfo().optionSaveFileVisible - 1;
		}
		break;
	case kPanelLoad:
		_loadPanel.currentButton = NULL;
		break;
	case kPanelQuit:
		_quitPanel.currentButton = NULL;
		break;
	case kPanelSave:
		_savePanel.currentButton = NULL;
		_textInputMaxWidth = _saveEdit->width - 10;
		_textInput = true;
		_textInputStringLength = strlen(_textInputString);
		_textInputPos = _textInputStringLength + 1;
		break;
	case kPanelMap:
		mapPanelShow();
		break;
	case kPanelSceneSubstitute:
		_vm->_render->setFlag(RF_DEMO_SUBST);
		_vm->_gfx->getCurrentPal(_mapSavedPal);
		break;
	case kPanelChapterSelection:
		break;
	case kPanelBoss:
		_vm->_render->setFlag(RF_DEMO_SUBST);
		break;
	case kPanelProtect:
		_protectPanel.currentButton = NULL;
		_textInputMaxWidth = _protectEdit->width - 10;
		_textInput = true;
		_textInputString[0] = 0;
		_textInputStringLength = 0;
		_textInputPos = _textInputStringLength + 1;
		break;
	}

	draw();
}

bool Interface::processAscii(uint16 ascii) {
	// TODO: Checking for Esc and Enter below is a bit hackish, and
	// and probably only works with the English version. Maybe we should
	// add a flag to the button so it can indicate if it's the default or
	// cancel button?

	int i;
	PanelButton *panelButton;
	if (_statusTextInput) {
		processStatusTextInput(ascii);
		return true;
	}

	switch (_panelMode) {
	case kPanelNull:
		if (ascii == 27) { // Esc
			if (_vm->_scene->isInIntro()) {
				_vm->_scene->skipScene();
			} else {
				if (!_disableAbortSpeeches)
					_vm->_actor->abortAllSpeeches();
			}
			return true;
		}
		break;
	case kPanelCutaway:
		if (ascii == 27) { // Esc
			if (!_disableAbortSpeeches)
				_vm->_actor->abortAllSpeeches();
			_vm->_scene->cutawaySkip();
			return true;
		}
		break;
	case kPanelVideo:
		if (ascii == 27) { // Esc
			if (_vm->_scene->isInIntro()) {
				_vm->_scene->skipScene();
			} else {
				if (!_disableAbortSpeeches)
					_vm->_actor->abortAllSpeeches();
			}
			_vm->_scene->cutawaySkip();
		}
		break;
	case kPanelOption:
		// TODO: check input dialog keys
		if (ascii == 27 || ascii == 13) { // Esc or Enter
			ascii = 'c'; //continue
		}

		for (i = 0; i < _optionPanel.buttonsCount; i++) {
			panelButton = &_optionPanel.buttons[i];
			if (panelButton->type == kPanelButtonOption) {
				if (panelButton->ascii == ascii) {
					setOption(panelButton);
					return true;
				}
			}
		}
		break;
	case kPanelSave:
		if (_textInput && processTextInput(ascii)) {
			return true;
		}

		if (ascii == 27) { // Esc
			ascii = 'c'; // cancel
		} else if (ascii == 13) { // Enter
			ascii = 's'; // save
		}

		for (i = 0; i < _savePanel.buttonsCount; i++) {
			panelButton = &_savePanel.buttons[i];
			if (panelButton->type == kPanelButtonSave) {
				if (panelButton->ascii == ascii) {
					setSave(panelButton);
					return true;
				}
			}
		}
		break;
	case kPanelQuit:
		if (ascii == 27) { // Esc
			ascii = 'c'; // cancel
		} else if (ascii == 13) { // Enter
			ascii = 'q'; // quit
		}

		for (i = 0; i < _quitPanel.buttonsCount; i++) {
			panelButton = &_quitPanel.buttons[i];
			if (panelButton->type == kPanelButtonQuit) {
				if (panelButton->ascii == ascii) {
					setQuit(panelButton);
					return true;
				}
			}
		}
		break;
	case kPanelLoad:
		for (i = 0; i < _loadPanel.buttonsCount; i++) {
			panelButton = &_loadPanel.buttons[i];
			if (panelButton->type == kPanelButtonLoad) {
				if (panelButton->ascii == ascii) {
					setLoad(panelButton);
					return true;
				}
			}
		}
		break;
	case kPanelMain:
		for (i = 0; i < _mainPanel.buttonsCount; i++) {
			panelButton = &_mainPanel.buttons[i];
			if (panelButton->ascii == ascii) {
				if (panelButton->type == kPanelButtonVerb) {
					_vm->_script->setVerb(panelButton->id);
				}
				if (panelButton->type == kPanelButtonArrow) {
					inventoryChangePos(panelButton->id);
				}
				return true;
			}
		}
		if (ascii == 15) // ctrl-o
		{
			if (_saveReminderState > 0) {
				setMode(kPanelOption);
				return true;
			}
		}
		break;
	case kPanelConverse:
		switch (ascii) {
		case 'x':
			setMode(kPanelMain);
			if (_vm->_puzzle->isActive())
				_vm->_puzzle->exitPuzzle();
			break;

		case 'u':
			converseChangePos(-1);
			break;

		case 'd':
			converseChangePos(1);
			break;

		case '1':
		case '2':
		case '3':
		case '4':
			converseSetPos(ascii);
			break;
		}
		break;
	case kPanelMap:
		mapPanelClean();
		break;
	case kPanelSceneSubstitute:
		if (ascii == 13) {
			_vm->_render->clearFlag(RF_DEMO_SUBST);
			_vm->_gfx->setPalette(_mapSavedPal);
			setMode(kPanelMain);
			_vm->_script->setNoPendingVerb();
		} else if (ascii == 'q' || ascii == 'Q') {
			_vm->shutDown();
		}
		break;
	case kPanelBoss:
		_vm->_render->clearFlag(RF_DEMO_SUBST);
		keyBossExit();
		break;
	case kPanelProtect:
		if (_textInput && processTextInput(ascii)) {
			return true;
		}

		if (ascii == 27 || ascii == 13) { // Esc or Enter
			_vm->_script->wakeUpThreads(kWaitTypeRequest);
			_vm->_interface->setMode(kPanelMain);
			
			_protectHash = 0;

			for (char *p = _textInputString; *p; p++)
				_protectHash = (_protectHash << 1) + toupper(*p);
		}
		break;
	}
	return false;
}

void Interface::setStatusText(const char *text, int statusColor) {

	// Disable the status text in IHNM when the chapter is 8
	if (_vm->getGameType() == GType_IHNM && _vm->_scene->currentChapterNumber() == 8)
		return;

	assert(text != NULL);
	assert(strlen(text) < STATUS_TEXT_LEN);

	if (_vm->_render->getFlags() & (RF_PLACARD | RF_MAP))
		return;

	strncpy(_statusText, text, STATUS_TEXT_LEN);
	_statusOnceColor = statusColor;
	drawStatusBar();
}

void Interface::loadScenePortraits(int resourceId) {
	_scenePortraits.freeMem();

	_vm->_sprite->loadList(resourceId, _scenePortraits);
}

void Interface::drawVerbPanel(Surface *backBuffer, PanelButton* panelButton) {
	PanelButton * rightButtonVerbPanelButton;
	PanelButton * currentVerbPanelButton;
	KnownColor textColor;
	int spriteNumber;
	Point point;

	rightButtonVerbPanelButton = getPanelButtonByVerbType(_vm->_script->getRightButtonVerb());
	currentVerbPanelButton = getPanelButtonByVerbType(_vm->_script->getCurrentVerb());

	if (panelButton->state) {
		textColor = kKnownColorVerbTextActive;
	} else if (panelButton == rightButtonVerbPanelButton) {
		textColor = kKnownColorVerbTextActive;
	} else {
		textColor = kKnownColorVerbText;
	}

	if (panelButton == currentVerbPanelButton) {
		spriteNumber = panelButton->downSpriteNumber;
	} else {
		spriteNumber = panelButton->upSpriteNumber;
	}
	point.x = _mainPanel.x + panelButton->xOffset;
	point.y = _mainPanel.y + panelButton->yOffset;

	_vm->_sprite->draw(backBuffer, _vm->getDisplayClip(), _mainPanel.sprites, spriteNumber, point, 256);

	drawVerbPanelText(backBuffer, panelButton, textColor, kKnownColorVerbTextShadow);
}

void Interface::draw() {
	Surface *backBuffer;
	int i;

	Point leftPortraitPoint;
	Point rightPortraitPoint;
	Rect rect;

	backBuffer = _vm->_gfx->getBackBuffer();

	if (_vm->_scene->isInIntro() || _fadeMode == kFadeOut)
		return;

	drawStatusBar();

	if (_panelMode == kPanelMain || _panelMode == kPanelMap) {
		_mainPanel.getRect(rect);
		backBuffer->blit(rect, _mainPanel.image);

		for (i = 0; i < kVerbTypeIdsMax; i++) {
			if (_verbTypeToPanelButton[i] != NULL) {
				drawVerbPanel(backBuffer, _verbTypeToPanelButton[i]);
			}
		}
	} else if (_panelMode == kPanelConverse) {
		_conversePanel.getRect(rect);
		backBuffer->blit(rect, _conversePanel.image);
		converseDisplayTextLines(backBuffer);
	}

	if (_vm->getGameType() == GType_IHNM) {
		if (_vm->_spiritualBarometer > 255)
			_vm->_gfx->setPaletteColor(kIHNMColorPortrait, 0xff, 0xff, 0xff);
		else
			_vm->_gfx->setPaletteColor(kIHNMColorPortrait,
				_vm->_spiritualBarometer * _portraitBgColor.red / 256,
				_vm->_spiritualBarometer * _portraitBgColor.green / 256,
				_vm->_spiritualBarometer * _portraitBgColor.blue / 256);
	}

	if (_panelMode == kPanelMain || _panelMode == kPanelConverse ||
		_lockedMode == kPanelMain || _lockedMode == kPanelConverse) {
		leftPortraitPoint.x = _mainPanel.x + _vm->getDisplayInfo().leftPortraitXOffset;
		leftPortraitPoint.y = _mainPanel.y + _vm->getDisplayInfo().leftPortraitYOffset;
		_vm->_sprite->draw(backBuffer, _vm->getDisplayClip(), _defPortraits, _leftPortrait, leftPortraitPoint, 256);
	}

	if (!_inMainMode && _vm->getDisplayInfo().rightPortraitXOffset >= 0) { //FIXME: should we change !_inMainMode to _panelMode == kPanelConverse ?
		rightPortraitPoint.x = _mainPanel.x + _vm->getDisplayInfo().rightPortraitXOffset;
		rightPortraitPoint.y = _mainPanel.y + _vm->getDisplayInfo().rightPortraitYOffset;

		// This looks like hack - particularly since it's only done for
		// the right-side portrait - and perhaps it is! But as far as I
		// can tell this is what the original engine does. And it keeps
		// ITE from crashing when entering the Elk King's court.

		if (_rightPortrait >= _scenePortraits.spriteCount)
			_rightPortrait = 0;

		_vm->_sprite->draw(backBuffer, _vm->getDisplayClip(), _scenePortraits, _rightPortrait, rightPortraitPoint, 256);
	}

	drawInventory(backBuffer);
}

void Interface::calcOptionSaveSlider() {
	int totalFiles = _vm->getSaveFilesCount();
	int visibleFiles = _vm->getDisplayInfo().optionSaveFileVisible;
	if (_optionSaveFileSlider == NULL) return; //TODO:REMOVE
	int height = _optionSaveFileSlider->height;
	int sliderHeight;
	int pos;

	if (totalFiles < visibleFiles) {
		totalFiles = visibleFiles;
	}

	sliderHeight = visibleFiles * height / totalFiles;
	if (sliderHeight < 7) {
		sliderHeight = 7;
	}

	if (totalFiles - visibleFiles <= 0) {
		pos = 0;
	} else {
		pos = _optionSaveFileTop * (height - sliderHeight) / (totalFiles - visibleFiles);
	}
	_optionPanel.calcPanelButtonRect(_optionSaveFileSlider, _optionSaveRectTop);
	_optionSaveRectBottom = _optionSaveRectSlider = _optionSaveRectTop;

	_optionSaveRectTop.bottom = _optionSaveRectTop.top + pos;
	_optionSaveRectTop.top++;
	_optionSaveRectTop.right--;

	_optionSaveRectSlider.top = _optionSaveRectTop.bottom;
	_optionSaveRectSlider.bottom = _optionSaveRectSlider.top + sliderHeight;

	_optionSaveRectBottom.top = _optionSaveRectSlider.bottom;
	_optionSaveRectBottom.right--;
}

void Interface::drawPanelText(Surface *ds, InterfacePanel *panel, PanelButton *panelButton) {
	const char *text;
	int textWidth;
	Rect rect;
	Point textPoint;

	// Button differs for CD version
	if (panelButton->id == kTextReadingSpeed && _vm->getFeatures() & GF_CD_FX)
		return;
	if (panelButton->id == kTextShowDialog && !(_vm->getFeatures() & GF_CD_FX))
		return;

	text = _vm->getTextString(panelButton->id);
	panel->calcPanelButtonRect(panelButton, rect);
	if (panelButton->xOffset < 0) {
		if (_vm->getGameType() == GType_ITE)
			textWidth = _vm->_font->getStringWidth(kKnownFontMedium, text, 0, kFontNormal);
		else
			textWidth = _vm->_font->getStringWidth(kKnownFontVerb, text, 0, kFontNormal);
		rect.left += 2 + (panel->imageWidth - 1 - textWidth) / 2;
	}

	textPoint.x = rect.left;
	textPoint.y = rect.top + 1;

	if (_vm->getGameType() == GType_ITE)
		_vm->_font->textDraw(kKnownFontMedium, ds, text, textPoint, _vm->KnownColor2ColorId(kKnownColorVerbText), _vm->KnownColor2ColorId(kKnownColorVerbTextShadow), kFontShadow);
	else
		_vm->_font->textDraw(kKnownFontVerb, ds, text, textPoint, _vm->KnownColor2ColorId(kKnownColorVerbText), _vm->KnownColor2ColorId(kKnownColorVerbTextShadow), kFontShadow);
}

void Interface::drawOption() {
	const char *text;
	Surface *backBuffer;
	int i;
	int fontHeight;
	uint j, idx;
	int fgColor;
	int bgColor;
	Rect rect;
	Rect rect2;
	PanelButton *panelButton;
	Point textPoint;
	Point point;
	
	backBuffer = _vm->_gfx->getBackBuffer();

	_optionPanel.getRect(rect);
	backBuffer->blit(rect, _optionPanel.image);

	for (i = 0; i < _optionPanel.buttonsCount; i++) {
		panelButton = &_optionPanel.buttons[i];

		// TODO: This probably works for the button background, but the resources are still not loeaded
		// (_optionPanel.sprites)
		/*
		point.x = _optionPanel.x + panelButton->xOffset;
		point.y = _optionPanel.y + panelButton->yOffset;
		_vm->_sprite->draw(backBuffer, _vm->getDisplayClip(), _optionPanel.sprites, i, point, 256);
		*/

		if (panelButton->type == kPanelButtonOption) {
			drawPanelButtonText(backBuffer, &_optionPanel, panelButton);
		}
		if (panelButton->type == kPanelButtonOptionText) {
			drawPanelText(backBuffer, &_optionPanel, panelButton);
		}
	}

	if (_optionSaveRectTop.height() > 0) {
		if (_vm->getGameType() == GType_ITE) {
			backBuffer->drawRect(_optionSaveRectTop, kITEColorDarkGrey);
		} else {
			// TODO: Draw the button graphic properly for IHNM
		}
	}

	// FIXME: The _optionSaveFileSlider checks exist for IHNM, where 
	// _optionSaveFileSlider is not initialized correctly yet
	if (_optionSaveFileSlider == NULL) return; //TODO:REMOVE

	drawButtonBox(backBuffer, _optionSaveRectSlider, kSlider, _optionSaveFileSlider->state > 0);

	if (_optionSaveRectBottom.height() > 0) {
		backBuffer->drawRect(_optionSaveRectBottom, kITEColorDarkGrey);
	}

	_optionPanel.calcPanelButtonRect(_optionSaveFilePanel, rect);
	rect.top++;
	rect2 = rect;
	fontHeight = _vm->_font->getHeight(kKnownFontSmall);
	for (j = 0; j < _vm->getDisplayInfo().optionSaveFileVisible; j++) {
		bgColor = kITEColorDarkGrey0C;
		fgColor = kITEColorBrightWhite;

		idx = j + _optionSaveFileTop;
		if (idx == _optionSaveFileTitleNumber) {
			SWAP(bgColor, fgColor);
		}
		if (idx < _vm->getSaveFilesCount()) {
			rect2.top = rect.top + j * (fontHeight + 1);
			rect2.bottom = rect2.top + fontHeight;
			backBuffer->fillRect(rect2, bgColor);
			text = _vm->getSaveFile(idx)->name;
			textPoint.x = rect.left + 1;
			textPoint.y = rect2.top;
			_vm->_font->textDraw(kKnownFontSmall, backBuffer, text, textPoint, fgColor, 0, kFontNormal);
		}
	}

}

void Interface::drawQuit() {
	Surface *backBuffer;
	Rect rect;
	int i;
	PanelButton *panelButton;

	backBuffer = _vm->_gfx->getBackBuffer();

	_quitPanel.getRect(rect);
	drawButtonBox(backBuffer, rect, kButton, false);
	for (i = 0; i < _quitPanel.buttonsCount; i++) {
		panelButton = &_quitPanel.buttons[i];
		if (panelButton->type == kPanelButtonQuit) {
			drawPanelButtonText(backBuffer, &_quitPanel, panelButton);
		}
		if (panelButton->type == kPanelButtonQuitText) {
			drawPanelText(backBuffer, &_quitPanel, panelButton);
		}
	}
}

void Interface::handleQuitUpdate(const Point& mousePoint) {
	bool releasedButton;

	_quitPanel.currentButton = quitHitTest(mousePoint);
	releasedButton = (_quitPanel.currentButton != NULL) && (_quitPanel.currentButton->state > 0) && (!_vm->mouseButtonPressed());

	if (!_vm->mouseButtonPressed()) {
		_quitPanel.zeroAllButtonState();
	}

	if (releasedButton) {
		setQuit(_quitPanel.currentButton);
	}
}

void Interface::handleQuitClick(const Point& mousePoint) {
	_quitPanel.currentButton = quitHitTest(mousePoint);

	_quitPanel.zeroAllButtonState();

	if (_quitPanel.currentButton == NULL) {
		return;
	}

	_quitPanel.currentButton->state = 1;
}

void Interface::setQuit(PanelButton *panelButton) {
	_quitPanel.currentButton = NULL;
	switch (panelButton->id) {
		case kTextCancel:
			setMode(kPanelOption);
			break;
		case kTextQuit:
			_vm->shutDown();
			break;
	}
}

void Interface::drawLoad() {
	Surface *backBuffer;
	Rect rect;
	int i;
	PanelButton *panelButton;

	backBuffer = _vm->_gfx->getBackBuffer();

	_loadPanel.getRect(rect);
	drawButtonBox(backBuffer, rect, kButton, false);
	for (i = 0; i < _loadPanel.buttonsCount; i++) {
		panelButton = &_loadPanel.buttons[i];
		if (panelButton->type == kPanelButtonLoad) {
			drawPanelButtonText(backBuffer, &_loadPanel, panelButton);
		}
		if (panelButton->type == kPanelButtonLoadText) {
			drawPanelText(backBuffer, &_loadPanel, panelButton);
		}
	}
}

void Interface::handleLoadUpdate(const Point& mousePoint) {
	bool releasedButton;

	_loadPanel.currentButton = loadHitTest(mousePoint);
	releasedButton = (_loadPanel.currentButton != NULL) && (_loadPanel.currentButton->state > 0) && (!_vm->mouseButtonPressed());

	if (!_vm->mouseButtonPressed()) {
		_loadPanel.zeroAllButtonState();
	}

	if (releasedButton) {
		setLoad(_loadPanel.currentButton);
	}
}

void Interface::handleLoadClick(const Point& mousePoint) {
	_loadPanel.currentButton = loadHitTest(mousePoint);

	_loadPanel.zeroAllButtonState();

	if (_loadPanel.currentButton == NULL) {
		return;
	}

	_loadPanel.currentButton->state = 1;
}

void Interface::setLoad(PanelButton *panelButton) {
	_loadPanel.currentButton = NULL;
	switch (panelButton->id) {
		case kTextOK:
			setMode(kPanelMain);
			break;
	}
}

void Interface::processStatusTextInput(uint16 ascii) {

	switch (ascii) {
	case 27: // esc
		_statusTextInputState = kStatusTextInputAborted;
		_statusTextInput = false;
		_vm->_script->wakeUpThreads(kWaitTypeStatusTextInput);
		break;
	case 13: // return
		_statusTextInputState = kStatusTextInputEntered;
		_statusTextInput = false;
		_vm->_script->wakeUpThreads(kWaitTypeStatusTextInput);
		break;
	case 8: // backspace
		if (_statusTextInputPos == 0) {
			break;
		}
		_statusTextInputPos--;
		_statusTextInputString[_statusTextInputPos] = 0;
	default:
		if (_statusTextInputPos >= STATUS_TEXT_INPUT_MAX) {
			break;
		}
		if (((ascii >= 'a') && (ascii <='z')) ||
			((ascii >= '0') && (ascii <='9')) ||
			((ascii >= 'A') && (ascii <='Z')) ||
			(ascii == ' ')) {
			_statusTextInputString[_statusTextInputPos++] = ascii;
			_statusTextInputString[_statusTextInputPos] = 0;
		}
	}
	setStatusText(_statusTextInputString);
}

bool Interface::processTextInput(uint16 ascii) {
	char ch[2];
	char tempString[SAVE_TITLE_SIZE];
	uint tempWidth;
	memset(tempString, 0, SAVE_TITLE_SIZE);
	ch[1] = 0;

	switch (ascii) {
	case 13:
		return false;
	case 27: // esc
		_textInput = false;
		break;
	case 8: // backspace
		if (_textInputPos <= 1) {
			break;
		}
		_textInputPos--;
	case 127: // del
		if (_textInputPos <= _textInputStringLength) {
			if (_textInputPos != 1) {
				strncpy(tempString, _textInputString, _textInputPos - 1);
			}
			if (_textInputPos != _textInputStringLength) {
				strncat(tempString, &_textInputString[_textInputPos], _textInputStringLength - _textInputPos);
			}
			strcpy(_textInputString, tempString);
			_textInputStringLength = strlen(_textInputString);
		}
		break;
	case 276: // left
		if (_textInputPos > 1) {
			_textInputPos--;
		}
		break;
	case 275: // right
		if (_textInputPos <= _textInputStringLength) {
			_textInputPos++;
		}
		break;
	default:
		if (((ascii >= 'a') && (ascii <='z')) ||
			((ascii >= '0') && (ascii <='9')) ||
			((ascii >= 'A') && (ascii <='Z')) ||
			(ascii == ' ')) {
			if (_textInputStringLength < SAVE_TITLE_SIZE - 1) {
				ch[0] = ascii;
				tempWidth = _vm->_font->getStringWidth(kKnownFontSmall, ch, 0, kFontNormal);
				tempWidth += _vm->_font->getStringWidth(kKnownFontSmall, _textInputString, 0, kFontNormal);
				if (tempWidth > _textInputMaxWidth) {
									break;
				}
				if (_textInputPos != 1) {
					strncpy(tempString, _textInputString, _textInputPos - 1);
					strcat(tempString, ch);
				}
				if ((_textInputStringLength == 0) || (_textInputPos == 1)) {
					strcpy(tempString, ch);
				}
				if ((_textInputStringLength != 0) && (_textInputPos != _textInputStringLength)) {
					strncat(tempString, &_textInputString[_textInputPos - 1], _textInputStringLength - _textInputPos + 1);
				}

				strcpy(_textInputString, tempString);
				_textInputStringLength = strlen(_textInputString);
				_textInputPos++;
			}
		}
		break;
	}
	return true;
}

void Interface::drawTextInput(Surface *ds, InterfacePanel *panel, PanelButton *panelButton) {
	Point textPoint;
	Rect rect;
	char ch[2];
	int fgColor;
	uint i;

	ch[1] = 0;
	panel->calcPanelButtonRect(panelButton, rect);
	drawButtonBox(ds, rect, kEdit, _textInput);
	rect.left += 4;
	rect.top += 4;
	rect.setHeight(_vm->_font->getHeight(kKnownFontSmall));

	i = 0;
	while ((ch[0] = _textInputString[i++]) != 0) {
		rect.setWidth(_vm->_font->getStringWidth(kKnownFontSmall, ch, 0, kFontNormal));
		if ((i == _textInputPos) && _textInput) {
			fgColor = kITEColorBlack;
			ds->fillRect(rect, kITEColorWhite);
		} else {
			fgColor = kITEColorWhite;
		}
		textPoint.x = rect.left;
		textPoint.y = rect.top + 1;

		_vm->_font->textDraw(kKnownFontSmall, ds, ch, textPoint, fgColor, 0, kFontNormal);
		rect.left += rect.width();
	}
	if (_textInput && (_textInputPos >= i)) {
		ch[0] = ' ';
		rect.setWidth(_vm->_font->getStringWidth(kKnownFontSmall, ch, 0, kFontNormal));
		ds->fillRect(rect, kITEColorWhite);
	}
}

void Interface::drawSave() {
	Surface *backBuffer;
	Rect rect;
	int i;
	PanelButton *panelButton;

	backBuffer = _vm->_gfx->getBackBuffer();

	_savePanel.getRect(rect);
	drawButtonBox(backBuffer, rect, kButton, false);
	for (i = 0; i < _savePanel.buttonsCount; i++) {
		panelButton = &_savePanel.buttons[i];
		if (panelButton->type == kPanelButtonSave) {
			drawPanelButtonText(backBuffer, &_savePanel, panelButton);
		}
		if (panelButton->type == kPanelButtonSaveText) {
			drawPanelText(backBuffer, &_savePanel, panelButton);
		}
	}

	drawTextInput(backBuffer, &_savePanel, _saveEdit);
}

void Interface::drawProtect() {
	Surface *backBuffer;
	Rect rect;
	int i;
	PanelButton *panelButton;

	backBuffer = _vm->_gfx->getBackBuffer();

	_protectPanel.getRect(rect);
	drawButtonBox(backBuffer, rect, kButton, false);

	for (i = 0; i < _protectPanel.buttonsCount; i++) {
		panelButton = &_protectPanel.buttons[i];
		if (panelButton->type == kPanelButtonProtectText) {
			drawPanelText(backBuffer, &_protectPanel, panelButton);
		}
	}
	drawTextInput(backBuffer, &_protectPanel, _protectEdit);
}

void Interface::handleSaveUpdate(const Point& mousePoint) {
	bool releasedButton;

	_savePanel.currentButton = saveHitTest(mousePoint);

	validateSaveButtons();

	releasedButton = (_savePanel.currentButton != NULL) &&
		(_savePanel.currentButton->state > 0) && (!_vm->mouseButtonPressed());

	if (!_vm->mouseButtonPressed()) {
		_savePanel.zeroAllButtonState();
	}

	if (releasedButton) {
		setSave(_savePanel.currentButton);
	}
}

void Interface::handleSaveClick(const Point& mousePoint) {
	_savePanel.currentButton = saveHitTest(mousePoint);

	validateSaveButtons();

	_savePanel.zeroAllButtonState();

	if (_savePanel.currentButton == NULL) {
		_textInput = false;
		return;
	}

	_savePanel.currentButton->state = 1;
	if (_savePanel.currentButton == _saveEdit) {
		_textInput = true;
	}
}

void Interface::setSave(PanelButton *panelButton) {
	_savePanel.currentButton = NULL;
	uint titleNumber;
	char *fileName;
	switch (panelButton->id) {
		case kTextSave:
			if (_textInputStringLength == 0 ) {
				break;
			}
			if (!_vm->isSaveListFull() && (_optionSaveFileTitleNumber == 0)) {
				if (_vm->locateSaveFile(_textInputString, titleNumber)) {
					fileName = _vm->calcSaveFileName(_vm->getSaveFile(titleNumber)->slotNumber);
					_vm->save(fileName, _textInputString);
					_optionSaveFileTitleNumber = titleNumber;
				} else {
					fileName = _vm->calcSaveFileName(_vm->getNewSaveSlotNumber());
					_vm->save(fileName, _textInputString);
					_vm->fillSaveList();
					calcOptionSaveSlider();
				}
			} else {
				fileName = _vm->calcSaveFileName(_vm->getSaveFile(_optionSaveFileTitleNumber)->slotNumber);
				_vm->save(fileName, _textInputString);
			}
			_textInput = false;
			setMode(kPanelOption);
			break;
		case kTextCancel:
			_textInput = false;
			setMode(kPanelOption);
			break;
	}
}

void Interface::handleOptionUpdate(const Point& mousePoint) {
	int16 mouseY;
	Rect rect;
	int totalFiles = _vm->getSaveFilesCount();
	int visibleFiles = _vm->getDisplayInfo().optionSaveFileVisible;
	bool releasedButton;

	if (_vm->mouseButtonPressed()) {
		if (_optionSaveFileSlider != NULL) //TODO:REMOVE
		if (_optionSaveFileSlider->state > 0) {
			_optionPanel.calcPanelButtonRect(_optionSaveFileSlider, rect);

			mouseY = mousePoint.y - rect.top -_optionSaveFileMouseOff;

			if (totalFiles - visibleFiles <= 0) {
				_optionSaveFileTop = 0;
			} else {
				_optionSaveFileTop = mouseY * (totalFiles - visibleFiles) /
					(_optionSaveFileSlider->height - _optionSaveRectSlider.height());
			}

			_optionSaveFileTop = clamp(0, _optionSaveFileTop, _vm->getSaveFilesCount() - _vm->getDisplayInfo().optionSaveFileVisible);
			calcOptionSaveSlider();
		}
	}

	_optionPanel.currentButton = optionHitTest(mousePoint);

	validateOptionButtons();

	releasedButton = (_optionPanel.currentButton != NULL) && (_optionPanel.currentButton->state > 0) && (!_vm->mouseButtonPressed());

	if (!_vm->mouseButtonPressed()) {
		_optionPanel.zeroAllButtonState();
	}

	if (releasedButton) {
		setOption(_optionPanel.currentButton);
	}
}


void Interface::handleOptionClick(const Point& mousePoint) {
	Rect rect;
	_optionPanel.currentButton = optionHitTest(mousePoint);

	validateOptionButtons();

	_optionPanel.zeroAllButtonState();

	if (_optionPanel.currentButton == NULL) {
		return;
	}

	if (_optionPanel.currentButton == _optionSaveFileSlider) {
		if ((_optionSaveRectTop.height() > 0) && (mousePoint.y < _optionSaveRectTop.bottom)) {
			_optionSaveFileTop -= _vm->getDisplayInfo().optionSaveFileVisible;
		} else {
			if ((_optionSaveRectBottom.height() > 0) && (mousePoint.y >= _optionSaveRectBottom.top)) {
				_optionSaveFileTop += _vm->getDisplayInfo().optionSaveFileVisible;
			} else {
				if (_vm->getDisplayInfo().optionSaveFileVisible < _vm->getSaveFilesCount()) {
					_optionSaveFileMouseOff = mousePoint.y - _optionSaveRectSlider.top;
					_optionPanel.currentButton->state = 1;
				}
			}
		}

		_optionSaveFileTop = clamp(0, _optionSaveFileTop, _vm->getSaveFilesCount() - _vm->getDisplayInfo().optionSaveFileVisible);
		calcOptionSaveSlider();
	} else {
		if (_optionPanel.currentButton == _optionSaveFilePanel) {
			_optionPanel.calcPanelButtonRect(_optionSaveFilePanel, rect);
			_optionSaveFileTitleNumber = (mousePoint.y - rect.top) / (_vm->_font->getHeight(kKnownFontSmall) + 1);

			if (_optionSaveFileTitleNumber >= _vm->getDisplayInfo().optionSaveFileVisible) {
				_optionSaveFileTitleNumber = _vm->getDisplayInfo().optionSaveFileVisible - 1;
			}
			_optionSaveFileTitleNumber += _optionSaveFileTop;
			if (_optionSaveFileTitleNumber >= _vm->getSaveFilesCount()) {
				_optionSaveFileTitleNumber = _vm->getSaveFilesCount() - 1;
			}
		} else {
			_optionPanel.currentButton->state = 1;
		}
	}
}

void Interface::handleChapterSelectionUpdate(const Point& mousePoint) {
	uint16 objectId;

	// FIXME: Original handled more object types here.

	objectId = _vm->_actor->hitTest(mousePoint, true);

	if (objectId != _vm->_script->_pointerObject) {
		_vm->_script->_pointerObject = objectId;
	}
}

void Interface::handleChapterSelectionClick(const Point& mousePoint) {
	int obj = _vm->_script->_pointerObject;

	_vm->_actor->abortSpeech();

	if (obj) {
		int script = 0;
		HitZone *hitZone;
		ActorData *a;
		ObjectData *o;
		Event event;

		switch (objectTypeId(obj)) {
		case kGameObjectHitZone:
			hitZone = _vm->_scene->_actionMap->getHitZone(objectIdToIndex(obj));
			if (hitZone->getFlags() & kHitZoneExit)
				script = hitZone->getScriptNumber();
			break;

		case kGameObjectActor:
			a = _vm->_actor->getActor(obj);
			script = a->_scriptEntrypointNumber;
			break;

		case kGameObjectObject:
			o = _vm->_actor->getObj(obj);
			script = o->_scriptEntrypointNumber;
			break;
		}

		if (script > 0) {
			event.type = kEvTOneshot;
			event.code = kScriptEvent;
			event.op = kEventExecNonBlocking;
			event.time = 0;
			event.param = _vm->_scene->getScriptModuleNumber();
			event.param2 = script;
			event.param3 = _vm->_script->getVerbType(kVerbUse);		// Action
			event.param4 = obj;	// Object
			event.param5 = 0;	// With Object
			event.param6 = obj;		// Actor

			_vm->_events->queue(&event);
		}
	}
}

void Interface::setOption(PanelButton *panelButton) {
	char * fileName;
	_optionPanel.currentButton = NULL;
	switch (panelButton->id) {
	case kTextContinuePlaying:
		ConfMan.flushToDisk();
		setMode(kPanelMain);
		break;
	case kTextQuitGame:
		setMode(kPanelQuit);
		break;
	case kTextLoad:
		if (_vm->getSaveFilesCount() > 0) {
			if (_vm->isSaveListFull() || (_optionSaveFileTitleNumber > 0)) {
				fileName = _vm->calcSaveFileName(_vm->getSaveFile(_optionSaveFileTitleNumber)->slotNumber);
				setMode(kPanelMain);
				_vm->load(fileName);
			}
		}
		break;
	case kTextSave:
		if (!_vm->isSaveListFull() && (_optionSaveFileTitleNumber == 0)) {
			_textInputString[0] = 0;
		} else {
			strcpy(_textInputString, _vm->getSaveFile(_optionSaveFileTitleNumber)->name);
		}
		setMode(kPanelSave);
		break;
	case kTextReadingSpeed:
		if (_vm->getFeatures() & GF_CD_FX) {
			_vm->_subtitlesEnabled = !_vm->_subtitlesEnabled;
			ConfMan.setBool("subtitles", _vm->_subtitlesEnabled);
		} else {
			_vm->_readingSpeed = (_vm->_readingSpeed + 1) % 4;
			_vm->setTalkspeed(_vm->_readingSpeed);
		}
		break;
	case kTextMusic:
		_vm->_musicVolume = (_vm->_musicVolume + 1) % 11;
		_vm->_music->setVolume(_vm->_musicVolume == 10 ? -1 : _vm->_musicVolume * 25, 1);
		ConfMan.setInt("music_volume", _vm->_musicVolume * 25);
		break;
	case kTextSound:
		_vm->_soundVolume = (_vm->_soundVolume + 1) % 11;
		_vm->_sound->setVolume(_vm->_soundVolume == 10 ? 255 : _vm->_soundVolume * 25);
		ConfMan.setInt("sfx_volume", _vm->_soundVolume * 25);
		break;
	}
}

void Interface::update(const Point& mousePoint, int updateFlag) {

	if (!_active && _panelMode == kPanelNull && (updateFlag & UPDATE_MOUSECLICK))
		_vm->_actor->abortSpeech();

	if (_vm->_scene->isInIntro() || _fadeMode == kFadeOut || !_active) {
		return;
	}

	if (_statusTextInput) {
		return;
	}

	switch (_panelMode) {
	case kPanelMain:
		if (updateFlag & UPDATE_MOUSEMOVE) {
			bool lastWasPlayfield = _lastMousePoint.y < _vm->_scene->getHeight();
			if (mousePoint.y < _vm->_scene->getHeight()) {
				if (!lastWasPlayfield) {
					handleMainUpdate(mousePoint);
				}
				_vm->_script->whichObject(mousePoint);
			} else {
				if (lastWasPlayfield) {
					_vm->_script->setNonPlayfieldVerb();
				}
				handleMainUpdate(mousePoint);
			}

		} else {

			if (updateFlag & UPDATE_MOUSECLICK) {
				if (mousePoint.y < _vm->_scene->getHeight()) {
					_vm->_script->playfieldClick(mousePoint, (updateFlag & UPDATE_LEFTBUTTONCLICK) != 0);
				} else {
					handleMainClick(mousePoint);
				}
			}
		}
		break;

	case kPanelConverse:
		if (updateFlag & UPDATE_MOUSEMOVE) {
			handleConverseUpdate(mousePoint);
		} else {
			if (updateFlag & UPDATE_MOUSECLICK) {
				handleConverseClick(mousePoint);
			}
			if (updateFlag & UPDATE_WHEELUP) {
				converseChangePos(-1);
			}
			if (updateFlag & UPDATE_WHEELDOWN) {
				converseChangePos(1);
			}

			if (_vm->_puzzle->isActive()) {
				_vm->_puzzle->handleClick(mousePoint);
			}
		}
		break;

	case kPanelOption:
		if (updateFlag & UPDATE_MOUSEMOVE) {
			handleOptionUpdate(mousePoint);
		} else {
			if (updateFlag & UPDATE_MOUSECLICK) {
				handleOptionClick(mousePoint);
			}
			if (updateFlag & UPDATE_WHEELUP) {
				if (_optionSaveFileTop)
					_optionSaveFileTop--;
				calcOptionSaveSlider();
			}
			if (updateFlag & UPDATE_WHEELDOWN) {
				if (_optionSaveFileTop < _vm->getSaveFilesCount() - _vm->getDisplayInfo().optionSaveFileVisible)
					_optionSaveFileTop++;
				calcOptionSaveSlider();
			}
		}
		break;

	case kPanelQuit:
		if (updateFlag & UPDATE_MOUSEMOVE) {
			handleQuitUpdate(mousePoint);
		} else {
			if (updateFlag & UPDATE_MOUSECLICK) {
				handleQuitClick(mousePoint);
			}
		}
		break;

	case kPanelLoad:
		if (updateFlag & UPDATE_MOUSEMOVE) {

			handleLoadUpdate(mousePoint);

		} else {
			if (updateFlag & UPDATE_MOUSECLICK) {
				handleLoadClick(mousePoint);
			}
		}
		break;

	case kPanelSave:
		if (updateFlag & UPDATE_MOUSEMOVE) {

			handleSaveUpdate(mousePoint);

		} else {
			if (updateFlag & UPDATE_MOUSECLICK) {
				handleSaveClick(mousePoint);
			}
		}
		break;

	case kPanelMap:
		if (updateFlag & UPDATE_MOUSECLICK)
			mapPanelClean();
		break;

	case kPanelSceneSubstitute:
		if (updateFlag & UPDATE_MOUSECLICK) {
			_vm->_render->clearFlag(RF_DEMO_SUBST);
			_vm->_gfx->setPalette(_mapSavedPal);
			setMode(kPanelMain);
			_vm->_script->setNoPendingVerb();
		}
		break;

	case kPanelChapterSelection:
		// TODO: panel has silent button
		if (updateFlag & UPDATE_MOUSEMOVE) {
			handleChapterSelectionUpdate(mousePoint);
		} else {
			if (updateFlag & UPDATE_MOUSECLICK)
				handleChapterSelectionClick(mousePoint);
		}
		break;

	case kPanelProtect:
		// No mouse interaction
		break;

	}

	_lastMousePoint = mousePoint;
}

void Interface::drawStatusBar() {
	Surface *backBuffer;
	Rect rect;
	Point textPoint;
	int stringWidth;
	int color;

	// Disable the status text in IHNM when the chapter is 8
	if (_vm->getGameType() == GType_IHNM && _vm->_scene->currentChapterNumber() == 8)
		return;

	backBuffer = _vm->_gfx->getBackBuffer();

	// Erase background of status bar
	rect.left = _vm->getDisplayInfo().statusXOffset;
	rect.top = _vm->getDisplayInfo().statusYOffset;
	rect.right = rect.left + _vm->getDisplayWidth();
	rect.bottom = rect.top + _vm->getDisplayInfo().statusHeight;

	backBuffer->drawRect(rect, _vm->getDisplayInfo().statusBGColor);

	stringWidth = _vm->_font->getStringWidth(kKnownFontSmall, _statusText, 0, kFontNormal);

	if (_statusOnceColor == -1)
		color = _vm->getDisplayInfo().statusTextColor;
	else
		color = _statusOnceColor;

	textPoint.x = _vm->getDisplayInfo().statusXOffset + (_vm->getDisplayInfo().statusWidth - stringWidth) / 2;
	textPoint.y = _vm->getDisplayInfo().statusYOffset + _vm->getDisplayInfo().statusTextY;
	_vm->_font->textDraw(kKnownFontSmall, backBuffer, _statusText, textPoint, color, 0, kFontNormal);

	if (_saveReminderState > 0) {
		rect.left = _vm->getDisplayInfo().saveReminderXOffset;
		rect.top = _vm->getDisplayInfo().saveReminderYOffset;

		rect.right = rect.left + _vm->getDisplayInfo().saveReminderWidth;
		rect.bottom = rect.top + _vm->getDisplayInfo().saveReminderHeight;
		_vm->_sprite->draw(backBuffer, _vm->getDisplayClip(), _vm->_sprite->_saveReminderSprites,
			_saveReminderState == 1 ? _vm->getDisplayInfo().saveReminderFirstSpriteNumber : _vm->getDisplayInfo().saveReminderSecondSpriteNumber,
			rect, 256);

	}
}

void Interface::handleMainClick(const Point& mousePoint) {

	PanelButton *panelButton;

	panelButton = verbHitTest(mousePoint);
	if (panelButton) {
		_vm->_script->setVerb(panelButton->id);
		return;
	}

	panelButton = _mainPanel.hitTest(mousePoint, kPanelAllButtons);

	if (panelButton != NULL) {
		if (panelButton->type == kPanelButtonArrow) {
			panelButton->state = 1;
			converseChangePos(panelButton->id);
		}

		if (panelButton->type == kPanelButtonInventory) {
			if (_vm->_script->_pointerObject != ID_NOTHING) {
				_vm->_script->hitObject(_vm->leftMouseButtonPressed());
			}
			if (_vm->_script->_pendingVerb) {
				_vm->_actor->_protagonist->_currentAction = kActionWait;
				_vm->_script->doVerb();
			}
		}
	} else {
		if (_saveReminderState > 0) {
			Rect rect;
			rect.left = _vm->getDisplayInfo().saveReminderXOffset;
			rect.top = _vm->getDisplayInfo().saveReminderYOffset;

			rect.right = rect.left + _vm->getDisplayInfo().saveReminderWidth;
			rect.bottom = rect.top + _vm->getDisplayInfo().saveReminderHeight;
			if (rect.contains(mousePoint)) {
				setMode(kPanelOption);
			}
		}
	}
}

void Interface::handleMainUpdate(const Point& mousePoint) {
	PanelButton *panelButton;

	panelButton = verbHitTest(mousePoint);
	if (_mainPanel.currentButton != panelButton) {
		if (_mainPanel.currentButton) {
			if (_mainPanel.currentButton->type == kPanelButtonVerb) {
				setVerbState(_mainPanel.currentButton->id, 0);
			}
		}
		if (panelButton) {
			setVerbState(panelButton->id, 1);
		}
	}

	if (panelButton) {
		_mainPanel.currentButton = panelButton;
		return;
	}


	if (!_vm->mouseButtonPressed()) {			// remove pressed flag
		if (_inventoryUpButton) {
			_inventoryUpButton->state = 0;
			_inventoryDownButton->state = 0;
		}
	}

	panelButton = _mainPanel.hitTest(mousePoint, kPanelAllButtons);

	bool changed = false;

	if ((panelButton != NULL) && (panelButton->type == kPanelButtonArrow)) {
		if (panelButton->state == 1) {
			//TODO: insert timeout catchup
			inventoryChangePos(panelButton->id);
		}
		changed = true;
	} else {
		_vm->_script->whichObject(mousePoint);
	}

	changed = changed || (panelButton != _mainPanel.currentButton);
	_mainPanel.currentButton = panelButton;
	if (changed) {
		draw();
	}
}

//inventory stuff
void Interface::inventoryChangePos(int chg) {
	if ((chg < 0 && _inventoryStart + chg >= 0) ||
		(chg > 0 && _inventoryStart < _inventoryEnd)) {
			_inventoryStart += chg;
			draw();
		}
}

void Interface::inventorySetPos(int key) {
	_inventoryBox = key - '1';
	_inventoryPos = _inventoryStart + _inventoryBox;
	if (_inventoryPos >= _inventoryCount)
		_inventoryPos = -1;
}

void Interface::updateInventory(int pos) {
	int cols = _vm->getDisplayInfo().inventoryColumns;
	if (pos >= _inventoryCount) {
		pos = _inventoryCount - 1;
	}
	if (pos < 0) {
		pos = 0;
	}
	_inventoryStart = (pos - cols) / cols * cols;
	if (_inventoryStart < 0) {
		_inventoryStart = 0;
	}

	_inventoryEnd = (_inventoryCount - 1 - cols) / cols * cols;
	if (_inventoryEnd < 0) {
		_inventoryEnd = 0;
	}
}

void Interface::addToInventory(int objectId) {
	if (_inventoryCount >= _inventorySize) {
		return;
	}

	for (int i = _inventoryCount; i > 0; i--) {
		_inventory[i] = _inventory[i - 1];
	}

	_inventory[0] = objectId;
	_inventoryCount++;

	_inventoryPos = 0;
	updateInventory(0);
	draw();
}

void Interface::removeFromInventory(int objectId) {
	int j = inventoryItemPosition(objectId);
	if (j == -1) {
		return;
	}

	int i;

	for (i = j; i < _inventoryCount - 1; i++) {
		_inventory[i] = _inventory[i + 1];
	}

	--_inventoryCount;
	_inventory[_inventoryCount] = 0;
	updateInventory(j);
	draw();
}

void Interface::clearInventory() {
	for (int i = 0; i < _inventoryCount; i++)
		_inventory[i] = 0;

	_inventoryCount = 0;
	updateInventory(0);
}

int Interface::inventoryItemPosition(int objectId) {
	for (int i = 0; i < _inventoryCount; i++)
		if (_inventory[i] == objectId)
			return i;

	return -1;
}

void Interface::drawInventory(Surface *backBuffer) {
	if (!isInMainMode())
		return;

	int i;
	Rect rect;
	int ci;
	ObjectData *obj;
	ci = _inventoryStart;
	if (_inventoryStart != 0) {
		drawPanelButtonArrow(backBuffer, &_mainPanel, _inventoryUpButton);
	}
	if (_inventoryStart != _inventoryEnd) {
		drawPanelButtonArrow(backBuffer, &_mainPanel, _inventoryDownButton);
	}

	for (i = 0; i < _mainPanel.buttonsCount; i++) {
		if (_mainPanel.buttons[i].type != kPanelButtonInventory) {
			continue;
		}
		_mainPanel.calcPanelButtonRect(&_mainPanel.buttons[i], rect);

		// TODO: Different colour for IHNM, probably.
		backBuffer->drawRect(rect, kITEColorDarkGrey);

		if (ci < _inventoryCount) {
			obj = _vm->_actor->getObj(_inventory[ci]);
			_vm->_sprite->draw(backBuffer, _vm->getDisplayClip(), _vm->_sprite->_inventorySprites, obj->_spriteListResourceId, rect, 256);
		}

		ci++;
	}
}

void Interface::setVerbState(int verb, int state) {
	PanelButton * panelButton = getPanelButtonByVerbType(verb);
	if (panelButton == NULL) return;
	if (state == 2) {
		state = (_mainPanel.currentButton == panelButton) ? 1 : 0;
	}
	panelButton->state = state;
	draw();
}

void Interface::drawButtonBox(Surface *ds, const Rect& rect, ButtonKind kind, bool down) {
	byte cornerColor;
	byte frameColor;
	byte fillColor;
	byte solidColor;
	byte odl, our, idl, iur;

	switch (kind ) {
		case kSlider:
			cornerColor = 0x8b;
			frameColor = kITEColorBlack;
			fillColor = kITEColorLightBlue96;
			odl = kITEColorDarkBlue8a;
			our = kITEColorLightBlue92;
			idl = 0x89;
			iur = 0x94;
			solidColor = down ? kITEColorLightBlue94 : kITEColorLightBlue96;
			break;
		case kEdit:
			cornerColor = kITEColorLightBlue96;
			frameColor = kITEColorLightBlue96;
			fillColor = kITEColorLightBlue96;
			our = kITEColorDarkBlue8a;
			odl = kITEColorLightBlue94;
			iur = 0x97;
			idl = 0x95;
			if (down) {
				solidColor = kITEColorBlue;
			} else {
				solidColor = kITEColorDarkGrey0C;
			}
			break;
		default:
			cornerColor = 0x8b;
			frameColor = kITEColorBlack;
			solidColor = fillColor = kITEColorLightBlue96;
			odl = kITEColorDarkBlue8a;
			our = kITEColorLightBlue94;
			idl = 0x97;
			iur = 0x95;
			if (down) {
				SWAP(odl, our);
				SWAP(idl, iur);
			}
			break;
	}

	int x = rect.left;
	int y = rect.top;
	int w = rect.width();
	int h = rect.height();
	int xe = rect.right - 1;
	int ye = rect.bottom - 1;

	((byte *)ds->getBasePtr(x, y))[0] = cornerColor;
	((byte *)ds->getBasePtr(x, ye))[0] = cornerColor;
	((byte *)ds->getBasePtr(xe, y))[0] = cornerColor;
	((byte *)ds->getBasePtr(xe, ye))[0] = cornerColor;
	ds->hLine(x + 1, y, x + 1 + w - 2, frameColor);
	ds->hLine(x + 1, ye, x + 1 + w - 2, frameColor);
	ds->vLine(x, y + 1, y + 1 + h - 2, frameColor);
	ds->vLine(xe, y + 1, y + 1 + h - 2, frameColor);

	x++;
	y++;
	xe--;
	ye--;
	w -= 2;
	h -= 2;
	ds->vLine(x, y, y + h - 1, odl);
	ds->hLine(x, ye, x + w - 1, odl);
	ds->vLine(xe, y, y + h - 1, our);
	ds->hLine(x + 1, y, x + 1 + w - 2, our);

	x++;
	y++;
	xe--;
	ye--;
	w -= 2;
	h -= 2;
	((byte *)ds->getBasePtr(x, y))[0] = fillColor;
	((byte *)ds->getBasePtr(xe, ye))[0] = fillColor;
	ds->vLine(x, y + 1, y + 1 + h - 2, idl);
	ds->hLine(x + 1, ye, x + 1 + w - 2, idl);
	ds->vLine(xe, y, y + h - 1, iur);
	ds->hLine(x + 1, y, x + 1 + w - 2, iur);

	x++; y++;
	w -= 2; h -= 2;

	Common::Rect fill(x, y, x + w, y + h);
	ds->fillRect(fill, solidColor);
}

static const int readingSpeeds[] = { kTextClick, kTextSlow, kTextMid, kTextFast };

void Interface::drawPanelButtonText(Surface *ds, InterfacePanel *panel, PanelButton *panelButton) {
	const char *text;
	int textId;
	int textWidth;
	int textHeight;
	Point point;
	KnownColor textColor;
	Rect rect;

	textId = panelButton->id;
	switch (panelButton->id) {
	case kTextReadingSpeed:
		if (_vm->getFeatures() & GF_CD_FX) {
			if (_vm->_subtitlesEnabled)
				textId = kTextOn;
			else
				textId = kTextOff;
		} else {
			textId = readingSpeeds[_vm->_readingSpeed];
		}
		break;
	case kTextMusic:
		if (_vm->_musicVolume)
			textId = kText10Percent + _vm->_musicVolume - 1;
		else
			textId = kTextOff;
		break;
	case kTextSound:
		if (_vm->_soundVolume)
			textId = kText10Percent + _vm->_soundVolume - 1;
		else
			textId = kTextOff;
		break;
	}
	text = _vm->getTextString(textId);

	if (_vm->getGameType() == GType_ITE) {
		textWidth = _vm->_font->getStringWidth(kKnownFontMedium, text, 0, kFontNormal);
		textHeight = _vm->_font->getHeight(kKnownFontMedium);
	} else {
		textWidth = _vm->_font->getStringWidth(kKnownFontVerb, text, 0, kFontNormal);
		textHeight = _vm->_font->getHeight(kKnownFontVerb);
	}

	point.x = panel->x + panelButton->xOffset + (panelButton->width / 2) - (textWidth / 2);
	point.y = panel->y + panelButton->yOffset + (panelButton->height / 2) - (textHeight / 2);

	if (panelButton == panel->currentButton) {
		textColor = kKnownColorVerbTextActive;
	} else {
		textColor = kKnownColorVerbText;
	}

	panel->calcPanelButtonRect(panelButton, rect);
	drawButtonBox(ds, rect, kButton, panelButton->state > 0);

	if (_vm->getGameType() == GType_ITE)
		_vm->_font->textDraw(kKnownFontMedium, ds, text, point,
			_vm->KnownColor2ColorId(textColor), _vm->KnownColor2ColorId(kKnownColorVerbTextShadow), kFontShadow);
	else
		_vm->_font->textDraw(kKnownFontVerb, ds, text, point,
			_vm->KnownColor2ColorId(textColor), _vm->KnownColor2ColorId(kKnownColorVerbTextShadow), kFontShadow);
}

void Interface::drawPanelButtonArrow(Surface *ds, InterfacePanel *panel, PanelButton *panelButton) {
	Point point;
	int spriteNumber;

	if (panel->currentButton == panelButton) {
		if (panelButton->state != 0) {
			spriteNumber = panelButton->downSpriteNumber;
		} else {
			spriteNumber = panelButton->overSpriteNumber;
		}
	} else {
		spriteNumber = panelButton->upSpriteNumber;
	}

	point.x = panel->x + panelButton->xOffset;
	point.y = panel->y + panelButton->yOffset;

	if (_vm->getGameType() == GType_ITE)
		_vm->_sprite->draw(ds, _vm->getDisplayClip(), _vm->_sprite->_mainSprites, spriteNumber, point, 256);
	else
		_vm->_sprite->draw(ds, _vm->getDisplayClip(), _vm->_sprite->_arrowSprites, spriteNumber, point, 256);
}

void Interface::drawVerbPanelText(Surface *ds, PanelButton *panelButton, KnownColor textKnownColor, KnownColor textShadowKnownColor) {
	const char *text;
	int textWidth;
	Point point;
	int textId;

	if (_vm->getGameType() == GType_ITE) {
		textId = verbTypeToTextStringsIdLUT[0][panelButton->id];
		text = _vm->getTextString(textId);
	} else {	
		textId = verbTypeToTextStringsIdLUT[1][panelButton->id];
		text = _vm->_script->_mainStrings.getString(textId + 1);
		textShadowKnownColor = kKnownColorTransparent;
	}

	textWidth = _vm->_font->getStringWidth(kKnownFontVerb, text, 0, kFontNormal);

	if (_vm->getGameType() == GType_ITE) {
		point.x = _mainPanel.x + panelButton->xOffset + 1 + (panelButton->width - 1 - textWidth) / 2;
		point.y = _mainPanel.y + panelButton->yOffset + 1;
	} else {
		point.x = _mainPanel.x + panelButton->xOffset + 1 + (panelButton->width - textWidth) / 2;
		point.y = _mainPanel.y + panelButton->yOffset + 12;
	}

	_vm->_font->textDraw(kKnownFontVerb, ds, text, point, _vm->KnownColor2ColorId(textKnownColor),_vm->KnownColor2ColorId(textShadowKnownColor), (textShadowKnownColor != kKnownColorTransparent) ? kFontShadow : kFontNormal);
}


// Converse stuff
void Interface::converseInit(void) {
	for (int i = 0; i < CONVERSE_MAX_TEXTS; i++)
		_converseText[i].text = NULL;
	converseClear();
}

void Interface::converseClear(void) {
	for (int i = 0; i < CONVERSE_MAX_TEXTS; i++) {
		if (_converseText[i].text != NULL) {
			free(_converseText[i].text);
			_converseText[i].text = NULL;
		}
		_converseText[i].stringNum = -1;
		_converseText[i].replyId = 0;
		_converseText[i].replyFlags = 0;
		_converseText[i].replyBit = 0;
	}

	_converseTextCount = 0;
	_converseStrCount = 0;
	_converseStartPos = 0;
	_converseEndPos = 0;
	_conversePos = -1;
}

bool Interface::converseAddText(const char *text, int replyId, byte replyFlags, int replyBit) {
	int count = 0;	// count how many pieces of text per string
	int i;
	int len;
	byte c;

	assert(strlen(text) < CONVERSE_MAX_WORK_STRING);

	strncpy(_converseWorkString, text, CONVERSE_MAX_WORK_STRING);

	while (1) {
		len = strlen(_converseWorkString);

		for (i = len; i >= 0; i--) {
			c = _converseWorkString[i];

			if ((c == ' ' || c == '\0') && (_vm->_font->getStringWidth(kKnownFontSmall, _converseWorkString, i, kFontNormal) <= _vm->getDisplayInfo().converseMaxTextWidth)) {
				break;
			}
		}
		if (i < 0) {
			return true;
		}

		if (_converseTextCount == CONVERSE_MAX_TEXTS) {
			return true;
		}

		_converseText[_converseTextCount].text = (char *)malloc(i + 1);
		strncpy(_converseText[_converseTextCount].text, _converseWorkString, i);

		_converseText[_converseTextCount].text[i] = 0;
		_converseText[_converseTextCount].textNum = count;
		_converseText[_converseTextCount].stringNum = _converseStrCount;
		_converseText[_converseTextCount].replyId = replyId;
		_converseText[_converseTextCount].replyFlags = replyFlags;
		_converseText[_converseTextCount].replyBit = replyBit;

		_converseTextCount++;
		count++;

		if (len == i)
			break;

		strncpy(_converseWorkString, &_converseWorkString[i + 1], len - i);
	}

	_converseStrCount++;

	return false;
}

void Interface::converseDisplayText() {
	int end;

	_converseStartPos = 0;

	end = _converseTextCount - _vm->getDisplayInfo().converseTextLines;

	if (end < 0)
		end = 0;

	_converseEndPos = end;
	draw();
}


void Interface::converseSetTextLines(int row) {
	int pos = row + _converseStartPos;
	if (pos >= _converseTextCount)
		pos = -1;
	if (pos != _conversePos) {
		_conversePos = pos;
		draw();
	}
}

void Interface::converseDisplayTextLines(Surface *ds) {
	int relPos;
	byte foregnd;
	byte backgnd;
	byte bulletForegnd;
	byte bulletBackgnd;
	const char *str;
	char bullet[2] = {
		(char)0xb7, 0
	};
	Rect rect(8, _vm->getDisplayInfo().converseTextLines * _vm->getDisplayInfo().converseTextHeight);
	Point textPoint;

	assert(_conversePanel.buttonsCount >= 6);

	bulletForegnd = kITEColorGreen;
	bulletBackgnd = kITEColorBlack;

	rect.moveTo(_conversePanel.x + _conversePanel.buttons[0].xOffset,
		_conversePanel.y + _conversePanel.buttons[0].yOffset);

	ds->drawRect(rect, kITEColorDarkGrey); //fill bullet place

	for (int i = 0; i < _vm->getDisplayInfo().converseTextLines; i++) {
		relPos = _converseStartPos + i;

		if (_converseTextCount <= relPos) {
			break;
		}

		if (_conversePos >= 0 && _converseText[_conversePos].stringNum == _converseText[relPos].stringNum) {
			foregnd = kITEColorBrightWhite;
			backgnd = (!_vm->leftMouseButtonPressed()) ? kITEColorDarkGrey : kITEColorGrey;
		} else {
			foregnd = kITEColorBlue;
			backgnd = kITEColorDarkGrey;
		}

		_conversePanel.calcPanelButtonRect(&_conversePanel.buttons[i], rect);
		rect.left += 8;
		ds->drawRect(rect, backgnd);

		str = _converseText[relPos].text;

		if (_converseText[relPos].textNum == 0) { // first entry
			textPoint.x = rect.left - 6;
			textPoint.y = rect.top;

			_vm->_font->textDraw(kKnownFontSmall, ds, bullet, textPoint, bulletForegnd, bulletBackgnd, (FontEffectFlags)(kFontShadow | kFontDontmap));
		}
		textPoint.x = rect.left + 1;
		textPoint.y = rect.top;
		_vm->_font->textDraw(kKnownFontSmall, ds, str, textPoint, foregnd, kITEColorBlack, kFontShadow);
	}

	if (_converseStartPos != 0) {
		drawPanelButtonArrow(ds, &_conversePanel, _converseUpButton);
	}

	if (_converseStartPos != _converseEndPos) {
		drawPanelButtonArrow(ds, &_conversePanel, _converseDownButton);
	}
}

void Interface::converseChangePos(int chg) {
	if ((chg < 0 && _converseStartPos + chg >= 0) ||
		(chg > 0 && _converseStartPos < _converseEndPos)) {
		_converseStartPos += chg;
		draw();
	}
}

void Interface::converseSetPos(int key) {
	Converse *ct;
	int selection = key - '1';

	if (selection >= _converseTextCount)
		return;

	converseSetTextLines(selection);

	ct = &_converseText[_conversePos];

	_vm->_script->finishDialog(ct->replyId, ct->replyFlags, ct->replyBit);

	if (_vm->_puzzle->isActive())
		_vm->_puzzle->handleReply(ct->replyId);

	_conversePos = -1;
}


void Interface::handleConverseUpdate(const Point& mousePoint) {
	bool changed;

	PanelButton *last = _conversePanel.currentButton;

	if (!_vm->mouseButtonPressed()) {			// remove pressed flag
		if (_converseUpButton) {
			_converseUpButton->state = 0;
			_converseDownButton->state = 0;
		}
	}

	_conversePanel.currentButton = converseHitTest(mousePoint);
	changed = last != _conversePanel.currentButton;


	if (_conversePanel.currentButton == NULL) {
		_conversePos = -1;
		if (changed) {
			draw();
		}
		return;
	}

	if (_conversePanel.currentButton->type == kPanelButtonConverseText) {
		converseSetTextLines(_conversePanel.currentButton->id);
	}

	if (_conversePanel.currentButton->type == kPanelButtonArrow) {
		if (_conversePanel.currentButton->state == 1) {
			//TODO: insert timeout catchup
			converseChangePos(_conversePanel.currentButton->id);
		}
		draw();
	}
}


void Interface::handleConverseClick(const Point& mousePoint) {
	_conversePanel.currentButton = converseHitTest(mousePoint);

	if (_conversePanel.currentButton == NULL) {
		return;
	}

	if (_conversePanel.currentButton->type == kPanelButtonConverseText) {
		converseSetPos(_conversePanel.currentButton->ascii);
	}

	if (_conversePanel.currentButton->type == kPanelButtonArrow) {
		_conversePanel.currentButton->state = 1;
		converseChangePos(_conversePanel.currentButton->id);
	}

}

void Interface::saveState(Common::OutSaveFile *out) {
	out->writeUint16LE(_inventoryCount);

	for (int i = 0; i < _inventoryCount; i++) {
		out->writeUint16LE(_inventory[i]);
	}
}

void Interface::loadState(Common::InSaveFile *in) {
	_inventoryCount = in->readUint16LE();

	for (int i = 0; i < _inventoryCount; i++) {
		_inventory[i] = in->readUint16LE();
	}

	updateInventory(0);
}

void Interface::mapPanelShow() {
	int i;
	byte *resource;
	size_t resourceLength, imageLength;
	Surface *backBuffer;
	Rect rect;
	byte *image;
	int imageWidth, imageHeight;
	const byte *pal;
	PalEntry cPal[PAL_ENTRIES];

	_vm->_gfx->showCursor(false);

	backBuffer = _vm->_gfx->getBackBuffer();

	rect.left = rect.top = 0;

	_vm->_resource->loadResource(_interfaceContext, 
			 _vm->_resource->convertResourceId(RID_ITE_TYCHO_MAP), resource, resourceLength);
	if (resourceLength == 0) {
		error("Interface::mapPanelShow() unable to load Tycho map resource");
	}

	_vm->_gfx->getCurrentPal(_mapSavedPal);

	for (i = 0; i < 6 ; i++) {
		_vm->_gfx->palToBlack(_mapSavedPal, 0.2 * i);
		_vm->_render->drawScene();
		_vm->_system->delayMillis(5);
	}

	_vm->_render->setFlag(RF_MAP);

	_vm->decodeBGImage(resource, resourceLength, &image, &imageLength, &imageWidth, &imageHeight);
	pal = _vm->getImagePal(resource, resourceLength);

	for (i = 0; i < PAL_ENTRIES; i++) {
		cPal[i].red = *pal++;
		cPal[i].green = *pal++;
		cPal[i].blue = *pal++;
	}

	rect.setWidth(imageWidth);
	rect.setHeight(imageHeight);

	backBuffer->blit(rect, image);

	// Evil Evil
	for (i = 0; i < 6 ; i++) {
		_vm->_gfx->blackToPal(cPal, 0.2 * i);
		_vm->_render->drawScene();
		_vm->_system->delayMillis(5);
	}

	free(resource);
	free(image);

	setSaveReminderState(false);

	_mapPanelCrossHairState = true;
}

void Interface::mapPanelClean() {
	PalEntry pal[PAL_ENTRIES];
	int i;

	_vm->_gfx->getCurrentPal(pal);

	for (i = 0; i < 6 ; i++) {
		_vm->_gfx->palToBlack(pal, 0.2 * i);
		_vm->_render->drawScene();
		_vm->_system->delayMillis(5);
	}

	_vm->_render->clearFlag(RF_MAP);
	setMode(kPanelMain);

	_vm->_gfx->showCursor(true);
	_vm->_render->drawScene();

	for (i = 0; i < 6 ; i++) {
		_vm->_gfx->blackToPal(_mapSavedPal, 0.2 * i);
		_vm->_render->drawScene();
		_vm->_system->delayMillis(5);
	}
}

void Interface::mapPanelDrawCrossHair() {
	Surface *backBuffer;

	backBuffer = _vm->_gfx->getBackBuffer();
	_mapPanelCrossHairState = !_mapPanelCrossHairState;

	Point mapPosition = _vm->_isoMap->getMapPosition();
	Rect screen(_vm->getDisplayWidth(), _vm->_scene->getHeight());

	if (screen.contains(mapPosition)) {
		_vm->_sprite->draw(backBuffer, _vm->getDisplayClip(), _vm->_sprite->_mainSprites,
						   _mapPanelCrossHairState? RID_ITE_SPR_XHAIR1 : RID_ITE_SPR_XHAIR2,
						   mapPosition, 256);
	}
}

void Interface::keyBoss() {
	if (_vm->getGameType() != GType_IHNM)
		return;

	if (_bossMode != -1 || _fadeMode != kNoFade)
		return;

	_vm->_sound->pauseVoice();
	_vm->_sound->pauseSound();
	_vm->_music->pause();

	int i;
	byte *resource;
	size_t resourceLength, imageLength;
	Surface *backBuffer;
	Rect rect;
	byte *image;
	int imageWidth, imageHeight;
	const byte *pal;
	PalEntry cPal[PAL_ENTRIES];

	_vm->_gfx->showCursor(false);

	backBuffer = _vm->_gfx->getBackBuffer();

	rect.left = rect.top = 0;

	_vm->_resource->loadResource(_interfaceContext, RID_IHNM_BOSS_SCREEN, resource, resourceLength);
	if (resourceLength == 0) {
		error("Interface::bossKey() unable to load Boss image resource");
	}

	_bossMode = _panelMode;
	setMode(kPanelBoss);

	_vm->decodeBGImage(resource, resourceLength, &image, &imageLength, &imageWidth, &imageHeight);
	rect.setWidth(imageWidth);
	rect.setHeight(imageHeight);

	_vm->_gfx->getCurrentPal(_mapSavedPal);
	pal = _vm->getImagePal(resource, resourceLength);

	for (i = 0; i < PAL_ENTRIES; i++) {
		cPal[i].red = *pal++;
		cPal[i].green = *pal++;
		cPal[i].blue = *pal++;
	}

	backBuffer->blit(rect, image);

	_vm->_gfx->setPalette(cPal);

	free(resource);
	free(image);
}


void Interface::keyBossExit() {
	PalEntry pal[PAL_ENTRIES];

	_vm->_sound->resumeVoice();
	_vm->_sound->resumeSound();
	_vm->_music->resume();

	_vm->_gfx->getCurrentPal(pal);

	_vm->_gfx->palToBlack(pal, 1);
	setMode(_bossMode);

	_vm->_render->drawScene();

	_vm->_gfx->blackToPal(_mapSavedPal, 1);

	_vm->_gfx->showCursor(true);

	_bossMode = -1;
}


} // End of namespace Saga
