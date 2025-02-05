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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/engines/gob/palanim.cpp $
 * $Id: palanim.cpp 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

#include "gob/gob.h"
#include "gob/palanim.h"
#include "gob/global.h"
#include "gob/util.h"
#include "gob/video.h"

namespace Gob {

PalAnim::PalAnim(GobEngine *vm) : _vm(vm) {
	_fadeValue = 1;
	for (int i = 0; i < 256; i++) {
		_toFadeRed[i] = 0;
		_toFadeGreen[i] = 0;
		_toFadeBlue[i] = 0;
	}

	_palArray[0] = _vm->_global->_redPalette;
	_palArray[1] = _vm->_global->_greenPalette;
	_palArray[2] = _vm->_global->_bluePalette;
	_fadeArray[0] = _toFadeRed;
	_fadeArray[1] = _toFadeGreen;
	_fadeArray[2] = _toFadeBlue;
}

char PalAnim::fadeColor(int16 from, int16 to) {
	if ((from - _fadeValue) > to)
		return from - _fadeValue;
	else if ((from + _fadeValue) < to)
		return from + _fadeValue;
	else return to;
}

bool PalAnim::fadeStepColor(int color) {
	bool stop = true;
	char colors[3];

	for (int i = 0; i < 16; i++) {
		colors[0] = _palArray[0][i];
		colors[1] = _palArray[1][i];
		colors[2] = _palArray[2][i];
		colors[color] = fadeColor(_palArray[color][i], _fadeArray[color][i]);

		_vm->_video->setPalElem(i, colors[0], colors[1], colors[2],
				-1, _vm->_global->_videoMode);

		if (_palArray[color][i] != _fadeArray[color][i])
			stop = false;
	}

	return stop;
}

bool PalAnim::fadeStep(int16 oper) {
	bool stop = true;
	byte newRed;
	byte newGreen;
	byte newBlue;

	if (_vm->_global->_colorCount != 256)
		error("PalAnim::fadeStep(): Only 256 color mode is supported!");

	if (oper == 0) {
		if (_vm->_global->_setAllPalette) {
			if (_vm->_global->_inVM != 0)
				error("PalAnim::fadeStep(): _vm->_global->_inVM != 0 not supported.");

			for (int i = 0; i < 256; i++) {
				newRed = fadeColor(_vm->_global->_redPalette[i], _toFadeRed[i]);
				newGreen = fadeColor(_vm->_global->_greenPalette[i], _toFadeGreen[i]);
				newBlue = fadeColor(_vm->_global->_bluePalette[i], _toFadeBlue[i]);

				if ((_vm->_global->_redPalette[i] != newRed) ||
				    (_vm->_global->_greenPalette[i] != newGreen) ||
				    (_vm->_global->_bluePalette[i] != newBlue)) {

					_vm->_video->setPalElem(i, newRed, newGreen, newBlue, 0, 0x13);

					_vm->_global->_redPalette[i] = newRed;
					_vm->_global->_greenPalette[i] = newGreen;
					_vm->_global->_bluePalette[i] = newBlue;
					stop = false;
				}
			}
		} else {
			for (int i = 0; i < 16; i++) {

				_vm->_video->setPalElem(i,
						fadeColor(_vm->_global->_redPalette[i], _toFadeRed[i]),
						fadeColor(_vm->_global->_greenPalette[i], _toFadeGreen[i]),
						fadeColor(_vm->_global->_bluePalette[i], _toFadeBlue[i]),
						-1, _vm->_global->_videoMode);

				if ((_vm->_global->_redPalette[i] != _toFadeRed[i]) ||
				    (_vm->_global->_greenPalette[i] != _toFadeGreen[i]) ||
				    (_vm->_global->_bluePalette[i] != _toFadeBlue[i]))
					stop = false;
			}
		}
	} else if ((oper > 0) && (oper < 4))
		stop = fadeStepColor(oper - 1);

	return stop;
}

void PalAnim::fade(Video::PalDesc *palDesc, int16 fadeV, int16 allColors) {
	bool stop;
	int16 i;

	if (_vm->_quitRequested)
		return;

	_fadeValue = (fadeV < 0) ? -fadeV : 2;

	if (_vm->_global->_colorCount < 256) {
		if (palDesc)
			_vm->_video->setFullPalette(palDesc);
		return;
	}

	if (!_vm->_global->_setAllPalette) {
		if (!palDesc) {
			for (i = 0; i < 16; i++) {
				_toFadeRed[i] = 0;
				_toFadeGreen[i] = 0;
				_toFadeBlue[i] = 0;
			}
		} else {
			for (i = 0; i < 16; i++) {
				_toFadeRed[i] = palDesc->vgaPal[i].red;
				_toFadeGreen[i] = palDesc->vgaPal[i].green;
				_toFadeBlue[i] = palDesc->vgaPal[i].blue;
			}
		}
	} else {
		if (_vm->_global->_inVM != 0)
			error("PalAnim::fade(): _vm->_global->_inVM != 0 is not supported");

		if (!palDesc) {
			for (i = 0; i < 256; i++) {
				_toFadeRed[i] = 0;
				_toFadeGreen[i] = 0;
				_toFadeBlue[i] = 0;
			}
		} else {
			for (i = 0; i < 256; i++) {
				_toFadeRed[i] = palDesc->vgaPal[i].red;
				_toFadeGreen[i] = palDesc->vgaPal[i].green;
				_toFadeBlue[i] = palDesc->vgaPal[i].blue;
			}
		}
	}

	if (allColors == 0) {
		do {
			stop = fadeStep(0);
			_vm->_video->waitRetrace();

			if (fadeV > 0)
				_vm->_util->delay(fadeV);
		} while (!stop);

		if (palDesc)
			_vm->_video->setFullPalette(palDesc);
		else
			_vm->_util->clearPalette();
	}

	if (allColors == 1) {
		do {
			_vm->_video->waitRetrace();
			stop = fadeStep(1);
		} while (!stop);

		do {
			_vm->_video->waitRetrace();
			stop = fadeStep(2);
		} while (!stop);

		do {
			_vm->_video->waitRetrace();
			stop = fadeStep(3);
		} while (!stop);

		if (palDesc)
			_vm->_video->setFullPalette(palDesc);
		else
			_vm->_util->clearPalette();
	}
}

} // End of namespace Gob
