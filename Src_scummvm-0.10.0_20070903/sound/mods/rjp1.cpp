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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/sound/mods/rjp1.cpp $
 * $Id: rjp1.cpp 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

#include "common/stdafx.h"
#include "common/endian.h"

#include "sound/mods/paula.h"
#include "sound/mods/rjp1.h"
#include "sound/audiostream.h"

namespace Audio {

struct Rjp1Channel {
	const int8 *waveData;
	const int8 *modulatePeriodData;
	const int8 *modulateVolumeData;
	const int8 *envelopeData;
	uint16 volumeScale;
	int16 volume;
	uint16 modulatePeriodBase;
	uint32 modulatePeriodLimit;
	uint32 modulatePeriodIndex;
	uint16 modulateVolumeBase;
	uint32 modulateVolumeLimit;
	uint32 modulateVolumeIndex;
	uint8 freqStep;
	uint32 freqInc;
	uint32 freqInit;
	const uint8 *noteData;
	const uint8 *sequenceOffsets;
	const uint8 *sequenceData;
	uint8 loopSeqCount;
	uint8 loopSeqCur;
	uint8 loopSeq2Count;
	uint8 loopSeq2Cur;
	bool active;
	int16 modulatePeriodInit;
	int16 modulatePeriodNext;
	bool setupNewNote;
	int8 envelopeMode;
	int8 envelopeScale;
	int8 envelopeEnd1;
	int8 envelopeEnd2;
	int8 envelopeStart;
	int8 envelopeVolume;
	uint8 currentInstrument;
	const int8 *data;
	uint16 pos;
	uint16 len;
	uint16 repeatPos;
	uint16 repeatLen;
};

class Rjp1 : public Paula {
public:

	struct Vars {
		int8 *instData;
		uint8 *songData[7];
		uint8 activeChannelsMask;
		uint8 currentChannel;
		int subsongsCount;
		int instrumentsCount;
	};

	Rjp1(int rate, bool stereo);
	virtual ~Rjp1();

	bool load(Common::SeekableReadStream *songData, Common::SeekableReadStream *instrumentsData);
	void unload();

	void startSong(int song);

protected:

	void startSequence(uint8 channelNum, uint8 seqNum);
	void turnOffChannel(Rjp1Channel *channel);
	void playChannel(Rjp1Channel *channel);
	void turnOnChannel(Rjp1Channel *channel);
	void playSongSequence(Rjp1Channel *channel);
	void modulateVolume(Rjp1Channel *channel);
	void modulatePeriod(Rjp1Channel *channel);
	void setupNote(Rjp1Channel *channel, int16 freq);
	void setupInstrument(Rjp1Channel *channel, uint8 num);
	void setRelease(Rjp1Channel *channel);
	void modulateVolumeEnvelope(Rjp1Channel *channel);
	void setSustain(Rjp1Channel *channel);
	void setDecay(Rjp1Channel *channel);
	void modulateVolumeWaveform(Rjp1Channel *channel);
	void setVolume(Rjp1Channel *channel);

	void stopPaulaChannel(uint8 channel);
	void setupPaulaChannel(uint8 channel, const int8 *waveData, uint16 offset, uint16 len, uint16 repeatPos, uint16 repeatLen);
	void setupPaulaChannelPeriod(uint8 channel, int16 period);
	void setPaulaChannelVolume(uint8 channel, uint8 volume);

	virtual void interrupt();

	Vars _vars;
	Rjp1Channel _channelsTable[4];

	static const int16 _periodsTable[];
	static const int _periodsCount;
};

Rjp1::Rjp1(int rate, bool stereo)
	: Paula(stereo, rate, rate / 50) {
	memset(&_vars, 0, sizeof(_vars));
	memset(_channelsTable, 0, sizeof(_channelsTable));
}

Rjp1::~Rjp1() {
	unload();
}

bool Rjp1::load(Common::SeekableReadStream *songData, Common::SeekableReadStream *instrumentsData) {
	if (songData->readUint32BE() == MKID_BE('RJP1') && songData->readUint32BE() == MKID_BE('SMOD')) {
		for (int i = 0; i < 7; ++i) {
			uint32 size = songData->readUint32BE();
			_vars.songData[i] = (uint8 *)malloc(size);
			if (!_vars.songData[i])
				return false;

			songData->read(_vars.songData[i], size);
			switch (i) {
			case 0:
				_vars.instrumentsCount = size / 32;
				break;
			case 1:
				break;
			case 2:
				// sequence index to offsets, 1 per channel
				_vars.subsongsCount = size / 4;
				break;
			case 3:
			case 4:
				// sequence offsets
				break;
			case 5:
			case 6:
				// sequence data
				break;
			}
		}

		if (instrumentsData->readUint32BE() == MKID_BE('RJP1')) {
			uint32 size = instrumentsData->size() - 4;
			_vars.instData = (int8 *)malloc(size);
			if (!_vars.instData)
				return false;

			instrumentsData->read(_vars.instData, size);

		}
	}

	debug(5, "Rjp1::load() _instrumentsCount = %d _subsongsCount = %d", _vars.instrumentsCount, _vars.subsongsCount);
	return true;
}

void Rjp1::unload() {
	for (int i = 0; i < 7; ++i) {
		free(_vars.songData[i]);
	}
	free(_vars.instData);
	memset(&_vars, 0, sizeof(_vars));
	memset(_channelsTable, 0, sizeof(_channelsTable));
}

void Rjp1::startSong(int song) {
	if (song == 0 || song >= _vars.subsongsCount) {
		warning("Invalid subsong number %d, defaulting to 1", song);
		song = 1;
	}
	const uint8 *p = _vars.songData[2] + (song & 0x3F) * 4;
	for (int i = 0; i < 4; ++i) {
		uint8 seq = *p++;
		if (seq) {
			startSequence(i, seq);
		}
	}
	// "start" Paula audiostream
	_playing = true;
	_end = false;
}

void Rjp1::startSequence(uint8 channelNum, uint8 seqNum) {
	Rjp1Channel *channel = &_channelsTable[channelNum];
	_vars.activeChannelsMask |= 1 << channelNum;
	if (seqNum != 0) {
		const uint8 *p = READ_BE_UINT32(_vars.songData[3] + seqNum * 4) + _vars.songData[5];
		uint8 seq = *p++;
		channel->sequenceOffsets = p;
		channel->sequenceData = READ_BE_UINT32(_vars.songData[4] + seq * 4) + _vars.songData[6];
		channel->loopSeqCount = 6;
		channel->loopSeqCur = channel->loopSeq2Cur = 1;
		channel->active = true;
	} else {
		channel->active = false;
		turnOffChannel(channel);
	}
}

void Rjp1::turnOffChannel(Rjp1Channel *channel) {
	stopPaulaChannel(channel - _channelsTable);
}

void Rjp1::playChannel(Rjp1Channel *channel) {
	if (channel->active) {
		turnOnChannel(channel);
		if (channel->sequenceData) {
			playSongSequence(channel);
		}
		modulateVolume(channel);
		modulatePeriod(channel);
	}
}

void Rjp1::turnOnChannel(Rjp1Channel *channel) {
	if (channel->setupNewNote) {
		channel->setupNewNote = false;
		setupPaulaChannel(channel - _channelsTable, channel->data, channel->pos, channel->len, channel->repeatPos, channel->repeatLen);
	}
}

void Rjp1::playSongSequence(Rjp1Channel *channel) {
	const uint8 *p = channel->sequenceData;
	--channel->loopSeqCur;
	if (channel->loopSeqCur == 0) {
		--channel->loopSeq2Cur;
		if (channel->loopSeq2Cur == 0) {
			bool loop = true;
			do {
				uint8 code = *p++;
				if (code & 0x80) {
					const uint8 *offs;
					switch (code & 7) {
					case 0:
						offs = channel->sequenceOffsets;
						channel->loopSeq2Count = 1;
						while (1) {
							code = *offs++;
							if (code != 0) {
								channel->sequenceOffsets = offs;
								p = READ_BE_UINT32(_vars.songData[4] + code * 4) + _vars.songData[6];
								break;
							} else {
								code = offs[0];
								if (code == 0) {
									p = 0;
									channel->active = false;
									_vars.activeChannelsMask &= ~(1 << _vars.currentChannel);
									loop = false;
									break;
								} else if (code & 0x80) {
									code = offs[1];
									offs = READ_BE_UINT32(_vars.songData[3] + code * 4) + _vars.songData[5];
								} else {
									offs -= code;
								}
							}
						}
						break;
					case 1:
						setRelease(channel);
						loop = false;
						break;
					case 2:
						channel->loopSeqCount = *p++;
						break;
					case 3:
						channel->loopSeq2Count = *p++;
						break;
					case 4:
						code = *p++;
						if (code != 0) {
							setupInstrument(channel, code);
						}
						break;
					case 5:
						channel->volumeScale = *p++;
						break;
					case 6:
						channel->freqStep = *p++;
						channel->freqInc = READ_BE_UINT32(p); p += 4;
						channel->freqInit = 0;
						break;
					case 7:
						loop = false;
						break;
					}
				} else {
					code >>= 1;
					if (code < _periodsCount) {
						setupNote(channel, _periodsTable[code]);
					}
					loop = false;
				}
			} while (loop);
			channel->sequenceData = p;
			channel->loopSeq2Cur = channel->loopSeq2Count;
		}
		channel->loopSeqCur = channel->loopSeqCount;
	}
}

void Rjp1::modulateVolume(Rjp1Channel *channel) {
	modulateVolumeEnvelope(channel);
	modulateVolumeWaveform(channel);
	setVolume(channel);
}

void Rjp1::modulatePeriod(Rjp1Channel *channel) {
	if (channel->modulatePeriodData) {
		uint32 per = channel->modulatePeriodIndex;
		int period = (channel->modulatePeriodData[per] * channel->modulatePeriodInit) / 128;
		period = -period;
		if (period < 0) {
			period /= 2;
		}
		channel->modulatePeriodNext = period + channel->modulatePeriodInit;
		++per;
		if (per == channel->modulatePeriodLimit) {
			per = channel->modulatePeriodBase * 2;
		}
		channel->modulatePeriodIndex = per;
	}
	if (channel->freqStep != 0) {
		channel->freqInit += channel->freqInc;
		--channel->freqStep;
	}
	setupPaulaChannelPeriod(channel - _channelsTable, channel->freqInit + channel->modulatePeriodNext);
}

void Rjp1::setupNote(Rjp1Channel *channel, int16 period) {
	const uint8 *note = channel->noteData;
	if (note) {
		channel->modulatePeriodInit = channel->modulatePeriodNext = period;
		channel->freqInit = 0;
		const int8 *e = (const int8 *)_vars.songData[1] + READ_BE_UINT16(note + 12);
		channel->envelopeData = e;
		channel->envelopeStart = e[1];
		channel->envelopeScale = e[1] - e[0];
		channel->envelopeEnd2 = e[2];
		channel->envelopeEnd1 = e[2];
		channel->envelopeMode = 4;
		channel->data = channel->waveData;
		channel->pos = READ_BE_UINT16(note + 16);
		channel->len = READ_BE_UINT16(note + 18);
		channel->setupNewNote = true;
	}
}

void Rjp1::setupInstrument(Rjp1Channel *channel, uint8 num) {
	if (channel->currentInstrument != num) {
		channel->currentInstrument = num;
		const uint8 *p = _vars.songData[0] + num * 32;
		channel->noteData = p;
		channel->repeatPos = READ_BE_UINT16(p + 20);
		channel->repeatLen = READ_BE_UINT16(p + 22);
		channel->volumeScale = READ_BE_UINT16(p + 14);
		channel->modulatePeriodBase = READ_BE_UINT16(p + 24);
		channel->modulatePeriodIndex = 0;
		channel->modulatePeriodLimit = READ_BE_UINT16(p + 26) * 2;
		channel->modulateVolumeBase = READ_BE_UINT16(p + 28);
		channel->modulateVolumeIndex = 0;
		channel->modulateVolumeLimit = READ_BE_UINT16(p + 30) * 2;
		channel->waveData = _vars.instData + READ_BE_UINT32(p);
		uint32 off = READ_BE_UINT32(p + 4);
		if (off) {
			channel->modulatePeriodData = _vars.instData + off;
		}
		off = READ_BE_UINT32(p + 8);
		if (off) {
			channel->modulateVolumeData = _vars.instData + off;
		}
	}
}

void Rjp1::setRelease(Rjp1Channel *channel) {
	const int8 *e = channel->envelopeData;
	if (e) {
		channel->envelopeStart = 0;
		channel->envelopeScale = -channel->envelopeVolume;
		channel->envelopeEnd2 = e[5];
		channel->envelopeEnd1 = e[5];
		channel->envelopeMode = -1;
	}
}

void Rjp1::modulateVolumeEnvelope(Rjp1Channel *channel) {
	if (channel->envelopeMode) {
		int16 es = channel->envelopeScale;
		if (es) {
			int8 m = channel->envelopeEnd1;
			if (m == 0) {
				es = 0;
			} else {
				es *= m;
				m = channel->envelopeEnd2;
				if (m == 0) {
					es = 0;
				} else {
					es /= m;
				}
			}
		}
		channel->envelopeVolume = channel->envelopeStart - es;
		--channel->envelopeEnd1;
		if (channel->envelopeEnd1 == -1) {
			switch (channel->envelopeMode) {
			case 0:
				break;
			case 2:
				setSustain(channel);
				break;
			case 4:
				setDecay(channel);
				break;
			case -1:
				setSustain(channel);
				break;
			default:
				error("Unhandled envelope mode %d", channel->envelopeMode);
				break;
			}
			return;
		}
	}
	channel->volume = channel->envelopeVolume;
}

void Rjp1::setSustain(Rjp1Channel *channel) {
	channel->envelopeMode = 0;
}

void Rjp1::setDecay(Rjp1Channel *channel) {
	const int8 *e = channel->envelopeData;
	if (e) {
		channel->envelopeStart = e[3];
		channel->envelopeScale = e[3] - e[1];
		channel->envelopeEnd2 = e[4];
		channel->envelopeEnd1 = e[4];
		channel->envelopeMode = 2;
	}
}

void Rjp1::modulateVolumeWaveform(Rjp1Channel *channel) {
	if (channel->modulateVolumeData) {
		uint32 i = channel->modulateVolumeIndex;
		channel->volume += channel->modulateVolumeData[i] * channel->volume / 128;
		++i;
		if (i == channel->modulateVolumeLimit) {
			i = channel->modulateVolumeBase * 2;
		}
		channel->modulateVolumeIndex = i;
	}
}

void Rjp1::setVolume(Rjp1Channel *channel) {
	channel->volume = (channel->volume * channel->volumeScale) / 64;
	channel->volume = CLIP<int16>(channel->volume, 0, 64);
	setPaulaChannelVolume(channel - _channelsTable, channel->volume);
}

void Rjp1::stopPaulaChannel(uint8 channel) {
	clearVoice(channel);
}

void Rjp1::setupPaulaChannel(uint8 channel, const int8 *waveData, uint16 offset, uint16 len, uint16 repeatPos, uint16 repeatLen) {
	if (waveData) {
		Channel *ch = &_voice[channel];
		ch->data = waveData;
		ch->dataRepeat = waveData + repeatPos * 2;
		ch->length = len * 2;
		ch->lengthRepeat = repeatLen * 2;
		ch->offset = offset * 2;
	}
}

void Rjp1::setupPaulaChannelPeriod(uint8 channel, int16 period) {
	_voice[channel].period = period;
}

void Rjp1::setPaulaChannelVolume(uint8 channel, uint8 volume) {
	_voice[channel].volume = volume;
}

void Rjp1::interrupt() {
	for (int i = 0; i < 4; ++i) {
		_vars.currentChannel = i;
		playChannel(&_channelsTable[i]);
	}
}

const int16 Rjp1::_periodsTable[] = {
	0x01C5, 0x01E0, 0x01FC, 0x021A, 0x023A, 0x025C, 0x0280, 0x02A6, 0x02D0,
	0x02FA, 0x0328, 0x0358, 0x00E2, 0x00F0, 0x00FE, 0x010D, 0x011D, 0x012E,
	0x0140, 0x0153, 0x0168, 0x017D, 0x0194, 0x01AC, 0x0071, 0x0078, 0x007F,
	0x0087, 0x008F, 0x0097, 0x00A0, 0x00AA, 0x00B4, 0x00BE, 0x00CA, 0x00D6
};

const int Rjp1::_periodsCount = ARRAYSIZE(_periodsTable);

AudioStream *makeRjp1Stream(Common::SeekableReadStream *songData, Common::SeekableReadStream *instrumentsData, int song, int rate, bool stereo) {
	Rjp1 *stream = new Rjp1(rate, stereo);
	if (stream->load(songData, instrumentsData)) {
		stream->startSong(song);
		return stream;
	}
	delete stream;
	return 0;
}

} // End of namespace Audio
