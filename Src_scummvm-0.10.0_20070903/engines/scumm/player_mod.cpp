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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/engines/scumm/player_mod.cpp $
 * $Id: player_mod.cpp 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

#include "common/stdafx.h"
#include "scumm/player_mod.h"
#include "sound/mixer.h"
#include "sound/rate.h"

namespace Scumm {

Player_MOD::Player_MOD(Audio::Mixer *mixer) {
	int i;
	_mixer = mixer;
	_samplerate = _mixer->getOutputRate();
	_mixamt = 0;
	_mixpos = 0;

	for (i = 0; i < MOD_MAXCHANS; i++) {
		_channels[i].id = 0;
		_channels[i].vol = 0;
		_channels[i].freq = 0;
		_channels[i].converter = NULL;
		_channels[i].input = NULL;
	}

	_playproc = NULL;
	_playparam = NULL;

	_mixer->playInputStream(Audio::Mixer::kPlainSoundType, &_soundHandle, this, -1, Audio::Mixer::kMaxChannelVolume, 0, false, true);
}

Player_MOD::~Player_MOD() {
	_mixer->stopHandle(_soundHandle);
	for (int i = 0; i < MOD_MAXCHANS; i++) {
		if (!_channels[i].id)
			continue;
		delete _channels[i].converter;
		delete _channels[i].input;
	}
}

void Player_MOD::setMusicVolume(int vol) {
	_maxvol = vol;
}

void Player_MOD::setUpdateProc(ModUpdateProc *proc, void *param, int freq) {
	_playproc = proc;
	_playparam = param;
	_mixamt = _samplerate / freq;
}
void Player_MOD::clearUpdateProc() {
	_playproc = NULL;
	_playparam = NULL;
	_mixamt = 0;
}

void Player_MOD::startChannel(int id, void *data, int size, int rate, uint8 vol, int loopStart, int loopEnd, int8 pan) {
	int i;
	if (id == 0)
		error("player_mod - attempted to start channel id 0");

	for (i = 0; i < MOD_MAXCHANS; i++) {
		if (!_channels[i].id)
			break;
	}
	if (i == MOD_MAXCHANS) {
		warning("player_mod - too many music channels playing (%i max)",MOD_MAXCHANS);
		return;
	}
	_channels[i].id = id;
	_channels[i].vol = vol;
	_channels[i].pan = pan;
	_channels[i].freq = rate;
	_channels[i].input = Audio::makeLinearInputStream((const byte*)data, size, rate, Audio::Mixer::FLAG_AUTOFREE | (loopStart != loopEnd ? Audio::Mixer::FLAG_LOOP : 0), loopStart, loopEnd);
	_channels[i].converter = Audio::makeRateConverter(rate, _mixer->getOutputRate(), false, false);
}

void Player_MOD::stopChannel(int id) {
	if (id == 0)
		error("player_mod - attempted to stop channel id 0");
	for (int i = 0; i < MOD_MAXCHANS; i++) {
		if (_channels[i].id == id) {
			delete _channels[i].converter;
			_channels[i].converter = NULL;
			delete _channels[i].input;
			_channels[i].input = NULL;
			_channels[i].id = 0;
			_channels[i].vol = 0;
			_channels[i].freq = 0;
		}
	}
}
void Player_MOD::setChannelVol(int id, uint8 vol) {
	if (id == 0)
		error("player_mod - attempted to set volume for channel id 0");
	for (int i = 0; i < MOD_MAXCHANS; i++) {
		if (_channels[i].id == id) {
			_channels[i].vol = vol;
			break;
		}
	}
}

void Player_MOD::setChannelPan(int id, int8 pan) {
	if (id == 0)
		error("player_mod - attempted to set pan for channel id 0");
	for (int i = 0; i < MOD_MAXCHANS; i++) {
		if (_channels[i].id == id) {
			_channels[i].pan = pan;
			break;
		}
	}
}

void Player_MOD::setChannelFreq(int id, int freq) {
	if (id == 0)
		error("player_mod - attempted to set frequency for channel id 0");
	for (int i = 0; i < MOD_MAXCHANS; i++) {
		if (_channels[i].id == id) {
			_channels[i].freq = freq;
			delete _channels[i].converter;
			_channels[i].converter = Audio::makeRateConverter(freq, _mixer->getOutputRate(), false, false);
			break;
		}
	}
}

void Player_MOD::do_mix(int16 *data, uint len) {
	int i;
	int dpos = 0;
	uint dlen = 0;
	memset(data, 0, 2 * len * sizeof(int16));
	while (len) {
		if (_playproc) {
			dlen = _mixamt - _mixpos;
			if (!_mixpos)
				_playproc(_playparam);
			if (dlen <= len) {
				_mixpos = 0;
				len -= dlen;
			} else {
				_mixpos = _mixamt - len;
				dlen = len;
				len = 0;
			}
		} else {
			dlen = len;
			len = 0;
		}
		for (i = 0; i < MOD_MAXCHANS; i++)
			if (_channels[i].id) {
				Audio::st_volume_t vol_l = (127 - _channels[i].pan) * _channels[i].vol / 127;
				Audio::st_volume_t vol_r = (127 + _channels[i].pan) * _channels[i].vol / 127;
				_channels[i].converter->flow(*_channels[i].input, &data[dpos*2], dlen, vol_l, vol_r);
			}
		dpos += dlen;
	}
}

} // End of namespace Scumm
