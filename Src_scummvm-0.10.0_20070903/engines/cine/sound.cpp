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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/engines/cine/sound.cpp $
 * $Id: sound.cpp 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

#include "common/stdafx.h"
#include "common/endian.h"
#include "common/file.h"
#include "common/system.h"

#include "cine/cine.h"
#include "cine/sound.h"

#include "sound/audiostream.h"
#include "sound/fmopl.h"
#include "sound/mods/soundfx.h"

namespace Cine {

class PCSoundDriver {
public:
	typedef void (*UpdateCallback)(void *);

	virtual ~PCSoundDriver() {}

	virtual void setupChannel(int channel, const byte *data, int instrument, int volume) = 0;
	virtual void setChannelFrequency(int channel, int frequency) = 0;
	virtual void stopChannel(int channel) = 0;
	virtual void playSample(const byte *data, int size, int channel, int volume) = 0;
	virtual void stopAll() = 0;
	virtual const char *getInstrumentExtension() const { return ""; }

	void setUpdateCallback(UpdateCallback upCb, void *ref);
	void resetChannel(int channel);
	void findNote(int freq, int *note, int *oct) const;

protected:
	UpdateCallback _upCb;
	void *_upRef;

	static const int _noteTable[];
	static const int _noteTableCount;
};

const int PCSoundDriver::_noteTable[] = {
	0xEEE, 0xE17, 0xD4D, 0xC8C, 0xBD9, 0xB2F, 0xA8E, 0x9F7,
	0x967, 0x8E0, 0x861, 0x7E8, 0x777, 0x70B, 0x6A6, 0x647,
	0x5EC, 0x597, 0x547, 0x4FB, 0x4B3, 0x470, 0x430, 0x3F4,
	0x3BB, 0x385, 0x353, 0x323, 0x2F6, 0x2CB, 0x2A3, 0x27D,
	0x259, 0x238, 0x218, 0x1FA, 0x1DD, 0x1C2, 0x1A9, 0x191,
	0x17B, 0x165, 0x151, 0x13E, 0x12C, 0x11C, 0x10C, 0x0FD,
	0x0EE, 0x0E1, 0x0D4, 0x0C8, 0x0BD, 0x0B2, 0x0A8, 0x09F,
	0x096, 0x08E, 0x086, 0x07E, 0x077, 0x070, 0x06A, 0x064,
	0x05E, 0x059, 0x054, 0x04F, 0x04B, 0x047, 0x043, 0x03F,
	0x03B, 0x038, 0x035, 0x032, 0x02F, 0x02C, 0x02A, 0x027,
	0x025, 0x023, 0x021, 0x01F, 0x01D, 0x01C, 0x01A, 0x019,
	0x017, 0x016, 0x015, 0x013, 0x012, 0x011, 0x010, 0x00F
};

const int PCSoundDriver::_noteTableCount = ARRAYSIZE(_noteTable);

struct AdlibRegisterSoundInstrument {
	uint16 vibrato;
	uint16 attackDecay;
	uint16 sustainRelease;
	uint16 feedbackStrength;
	uint16 keyScaling;
	uint16 outputLevel;
	uint16 freqMod;
};

struct AdlibSoundInstrument {
	byte mode;
	byte channel;
	AdlibRegisterSoundInstrument regMod;
	AdlibRegisterSoundInstrument regCar;
	byte waveSelectMod;
	byte waveSelectCar;
	byte amDepth;
};

class AdlibSoundDriver : public PCSoundDriver, Audio::AudioStream {
public:
	AdlibSoundDriver(Audio::Mixer *mixer);
	virtual ~AdlibSoundDriver();

	// PCSoundDriver interface
	virtual void setupChannel(int channel, const byte *data, int instrument, int volume);
	virtual void stopChannel(int channel);
	virtual void stopAll();

	// AudioStream interface
	virtual int readBuffer(int16 *buffer, const int numSamples);
	virtual bool isStereo() const { return false; }
	virtual bool endOfData() const { return false; }
	virtual int getRate() const { return _sampleRate; }

	void initCard();
	void update(int16 *buf, int len);
	void setupInstrument(const byte *data, int channel);
	void loadRegisterInstrument(const byte *data, AdlibRegisterSoundInstrument *reg);
	virtual void loadInstrument(const byte *data, AdlibSoundInstrument *asi) = 0;

protected:
	FM_OPL *_opl;
	int _sampleRate;
	Audio::Mixer *_mixer;
	Audio::SoundHandle _soundHandle;

	byte _vibrato;
	int _channelsVolumeTable[4];
	AdlibSoundInstrument _instrumentsTable[4];

	static const int _freqTable[];
	static const int _freqTableCount;
	static const int _operatorsTable[];
	static const int _operatorsTableCount;
	static const int _voiceOperatorsTable[];
	static const int _voiceOperatorsTableCount;
};

const int AdlibSoundDriver::_freqTable[] = {
	0x157, 0x16C, 0x181, 0x198, 0x1B1, 0x1CB,
	0x1E6, 0x203, 0x222, 0x243, 0x266, 0x28A
};

const int AdlibSoundDriver::_freqTableCount = ARRAYSIZE(_freqTable);

const int AdlibSoundDriver::_operatorsTable[] = {
	0, 1, 2, 3, 4, 5, 8, 9, 10, 11, 12, 13,	16, 17, 18, 19, 20, 21
};

const int AdlibSoundDriver::_operatorsTableCount = ARRAYSIZE(_operatorsTable);

const int AdlibSoundDriver::_voiceOperatorsTable[] = {
	0, 3, 1, 4, 2, 5, 6, 9, 7, 10, 8, 11, 12, 15, 16, 16, 14, 14, 17, 17, 13, 13
};

const int AdlibSoundDriver::_voiceOperatorsTableCount = ARRAYSIZE(_voiceOperatorsTable);

// Future Wars Adlib driver
class AdlibSoundDriverINS : public AdlibSoundDriver {
public:
	AdlibSoundDriverINS(Audio::Mixer *mixer) : AdlibSoundDriver(mixer) {}
	virtual const char *getInstrumentExtension() const { return ".INS"; }
	virtual void loadInstrument(const byte *data, AdlibSoundInstrument *asi);
	virtual void setChannelFrequency(int channel, int frequency);
	virtual void playSample(const byte *data, int size, int channel, int volume);
};

// Operation Stealth Adlib driver
class AdlibSoundDriverADL : public AdlibSoundDriver {
public:
	AdlibSoundDriverADL(Audio::Mixer *mixer) : AdlibSoundDriver(mixer) {}
	virtual const char *getInstrumentExtension() const { return ".ADL"; }
	virtual void loadInstrument(const byte *data, AdlibSoundInstrument *asi);
	virtual void setChannelFrequency(int channel, int frequency);
	virtual void playSample(const byte *data, int size, int channel, int volume);
};

class PCSoundFxPlayer {
public:

	PCSoundFxPlayer(PCSoundDriver *driver);
	~PCSoundFxPlayer();

	bool load(const char *song);
	void play();
	void stop();
	void fadeOut();

	static void updateCallback(void *ref);

	enum {
		NUM_INSTRUMENTS = 15,
		NUM_CHANNELS = 4
	};

private:

	void update();
	void handleEvents();
	void handlePattern(int channel, const byte *patternData);
	void unload();

	bool _playing;
	int _currentPos;
	int _currentOrder;
	int _numOrders;
	int _eventsDelay;
	int _fadeOutCounter;
	int _updateTicksCounter;
	int _instrumentsChannelTable[NUM_CHANNELS];
	byte *_sfxData;
	byte *_instrumentsData[NUM_INSTRUMENTS];
	PCSoundDriver *_driver;
};


void PCSoundDriver::setUpdateCallback(UpdateCallback upCb, void *ref) {
	_upCb = upCb;
	_upRef = ref;
}

void PCSoundDriver::findNote(int freq, int *note, int *oct) const {
	*note = _noteTableCount - 1;
	for (int i = 0; i < _noteTableCount; ++i) {
		if (_noteTable[i] <= freq) {
			*note = i;
			break;
		}
	}
	*oct = *note / 12;
}

void PCSoundDriver::resetChannel(int channel) {
	stopChannel(channel);
	stopAll();
}

AdlibSoundDriver::AdlibSoundDriver(Audio::Mixer *mixer)
	: _mixer(mixer) {
	_sampleRate = _mixer->getOutputRate();
	_opl = makeAdlibOPL(_sampleRate);
	memset(_channelsVolumeTable, 0, sizeof(_channelsVolumeTable));
	memset(_instrumentsTable, 0, sizeof(_instrumentsTable));
	initCard();
	_mixer->playInputStream(Audio::Mixer::kPlainSoundType, &_soundHandle, this, -1, Audio::Mixer::kMaxChannelVolume, 0, false, true);
}

AdlibSoundDriver::~AdlibSoundDriver() {
	_mixer->stopHandle(_soundHandle);
}

void AdlibSoundDriver::setupChannel(int channel, const byte *data, int instrument, int volume) {
	assert(channel < 4);
	if (data) {
		if (volume > 80) {
			volume = 80;
		} else if (volume < 0) {
			volume = 0;
		}
		volume += volume / 4;
		if (volume > 127) {
			volume = 127;
		}
		_channelsVolumeTable[channel] = volume;
		setupInstrument(data, channel);
	}
}

void AdlibSoundDriver::stopChannel(int channel) {
	assert(channel < 4);
	AdlibSoundInstrument *ins = &_instrumentsTable[channel];
	if (ins->mode != 0 && ins->channel == 6) {
		channel = 6;
	}
	if (ins->mode == 0 || channel == 6) {
		OPLWriteReg(_opl, 0xB0 | channel, 0);
	}
	if (ins->mode != 0) {
		_vibrato &= ~(1 << (10 - ins->channel));
		OPLWriteReg(_opl, 0xBD, _vibrato);
	}
}

void AdlibSoundDriver::stopAll() {
	int i;
	for (i = 0; i < 18; ++i) {
		OPLWriteReg(_opl, 0x40 | _operatorsTable[i], 63);
	}
	for (i = 0; i < 9; ++i) {
		OPLWriteReg(_opl, 0xB0 | i, 0);
	}
	OPLWriteReg(_opl, 0xBD, 0);
}

int AdlibSoundDriver::readBuffer(int16 *buffer, const int numSamples) {
	update(buffer, numSamples);
	return numSamples;
}

void AdlibSoundDriver::initCard() {
	_vibrato = 0x20;
	OPLWriteReg(_opl, 0xBD, _vibrato);
	OPLWriteReg(_opl, 0x08, 0x40);

	int i;
	for (i = 0; i < 18; ++i) {
		OPLWriteReg(_opl, 0x40 | _operatorsTable[i], 0);
	}
	for (i = 0; i < 9; ++i) {
		OPLWriteReg(_opl, 0xB0 | i, 0);
	}
	for (i = 0; i < 9; ++i) {
		OPLWriteReg(_opl, 0xC0 | i, 0);
	}
	for (i = 0; i < 18; ++i) {
		OPLWriteReg(_opl, 0x60 | _operatorsTable[i], 0);
	}
	for (i = 0; i < 18; ++i) {
		OPLWriteReg(_opl, 0x80 | _operatorsTable[i], 0);
	}
	for (i = 0; i < 18; ++i) {
		OPLWriteReg(_opl, 0x20 | _operatorsTable[i], 0);
	}
	for (i = 0; i < 18; ++i) {
		OPLWriteReg(_opl, 0xE0 | _operatorsTable[i], 0);
	}

	OPLWriteReg(_opl, 1, 0x20);
	OPLWriteReg(_opl, 1, 0);
}

void AdlibSoundDriver::update(int16 *buf, int len) {
	static int samplesLeft = 0;
	while (len != 0) {
		int count = samplesLeft;
		if (count > len) {
			count = len;
		}
		samplesLeft -= count;
		len -= count;
		YM3812UpdateOne(_opl, buf, count);
		if (samplesLeft == 0) {
			if (_upCb) {
				(*_upCb)(_upRef);
			}
			samplesLeft = _sampleRate / 50;
		}
		buf += count;
	}
}

void AdlibSoundDriver::setupInstrument(const byte *data, int channel) {
	assert(channel < 4);
	AdlibSoundInstrument *ins = &_instrumentsTable[channel];
	loadInstrument(data, ins);

	int mod, car, tmp;
	const AdlibRegisterSoundInstrument *reg;

	if (ins->mode != 0)  {
		mod = _operatorsTable[_voiceOperatorsTable[2 * ins->channel + 0]];
		car = _operatorsTable[_voiceOperatorsTable[2 * ins->channel + 1]];
	} else {
		mod = _operatorsTable[_voiceOperatorsTable[2 * channel + 0]];
		car = _operatorsTable[_voiceOperatorsTable[2 * channel + 1]];
	}

	if (ins->mode == 0 || ins->channel == 6) {
		reg = &ins->regMod;
		OPLWriteReg(_opl, 0x20 | mod, reg->vibrato);
		if (reg->freqMod) {
			tmp = reg->outputLevel & 0x3F;
		} else {
			tmp = (63 - (reg->outputLevel & 0x3F)) * _channelsVolumeTable[channel];
			tmp = 63 - (2 * tmp + 127) / (2 * 127);
		}
		OPLWriteReg(_opl, 0x40 | mod, tmp | (reg->keyScaling << 6));
		OPLWriteReg(_opl, 0x60 | mod, reg->attackDecay);
		OPLWriteReg(_opl, 0x80 | mod, reg->sustainRelease);
		if (ins->mode != 0) {
			OPLWriteReg(_opl, 0xC0 | ins->channel, reg->feedbackStrength);
		} else {
			OPLWriteReg(_opl, 0xC0 | channel, reg->feedbackStrength);
		}
		OPLWriteReg(_opl, 0xE0 | mod, ins->waveSelectMod);
	}

	reg = &ins->regCar;
	OPLWriteReg(_opl, 0x20 | car, reg->vibrato);
	tmp = (63 - (reg->outputLevel & 0x3F)) * _channelsVolumeTable[channel];
	tmp = 63 - (2 * tmp + 127) / (2 * 127);
	OPLWriteReg(_opl, 0x40 | car, tmp | (reg->keyScaling << 6));
	OPLWriteReg(_opl, 0x60 | car, reg->attackDecay);
	OPLWriteReg(_opl, 0x80 | car, reg->sustainRelease);
	OPLWriteReg(_opl, 0xE0 | car, ins->waveSelectCar);
}

void AdlibSoundDriver::loadRegisterInstrument(const byte *data, AdlibRegisterSoundInstrument *reg) {
	reg->vibrato = 0;
	if (READ_LE_UINT16(data + 18)) { // amplitude vibrato
		reg->vibrato |= 0x80;
	}
	if (READ_LE_UINT16(data + 20)) { // frequency vibrato
		reg->vibrato |= 0x40;
	}
	if (READ_LE_UINT16(data + 10)) { // sustaining sound
		reg->vibrato |= 0x20;
	}
	if (READ_LE_UINT16(data + 22)) { // envelope scaling
		reg->vibrato |= 0x10;
	}
	reg->vibrato |= READ_LE_UINT16(data + 2) & 0xF; // frequency multiplier

	reg->attackDecay = READ_LE_UINT16(data + 6) << 4; // attack rate
	reg->attackDecay |= READ_LE_UINT16(data + 12) & 0xF; // decay rate

	reg->sustainRelease = READ_LE_UINT16(data + 8) << 4; // sustain level
	reg->sustainRelease |= READ_LE_UINT16(data + 14) & 0xF; // release rate

	reg->feedbackStrength = READ_LE_UINT16(data + 4) << 1; // feedback
	if (READ_LE_UINT16(data + 24) == 0) { // frequency modulation
		reg->feedbackStrength |= 1;
	}

	reg->keyScaling = READ_LE_UINT16(data);
	reg->outputLevel = READ_LE_UINT16(data + 16);
	reg->freqMod = READ_LE_UINT16(data + 24);
}

void AdlibSoundDriverINS::loadInstrument(const byte *data, AdlibSoundInstrument *asi) {
	asi->mode = *data++;
	asi->channel = *data++;
	loadRegisterInstrument(data, &asi->regMod); data += 26;
	loadRegisterInstrument(data, &asi->regCar); data += 26;
	asi->waveSelectMod = data[0] & 3; data += 2;
	asi->waveSelectCar = data[0] & 3; data += 2;
	asi->amDepth = data[0]; data += 2;
}

void AdlibSoundDriverINS::setChannelFrequency(int channel, int frequency) {
	assert(channel < 4);
	AdlibSoundInstrument *ins = &_instrumentsTable[channel];
	if (ins->mode != 0 && ins->channel == 6) {
		channel = 6;
	}
	if (ins->mode == 0 || ins->channel == 6) {
		int freq, note, oct;
		findNote(frequency, &note, &oct);
		if (channel == 6) {
			note %= 12;
		}
		freq = _freqTable[note % 12];
		OPLWriteReg(_opl, 0xA0 | channel, freq);
		freq = ((note / 12) << 2) | ((freq & 0x300) >> 8);
		if (ins->mode == 0) {
			freq |= 0x20;
		}
		OPLWriteReg(_opl, 0xB0 | channel, freq);
	}
	if (ins->mode != 0) {
		_vibrato |= 1 << (10 - ins->channel);
		OPLWriteReg(_opl, 0xBD, _vibrato);
	}
}

void AdlibSoundDriverINS::playSample(const byte *data, int size, int channel, int volume) {
	assert(channel < 4);
	_channelsVolumeTable[channel] = 127;
	resetChannel(channel);
	setupInstrument(data + 257, channel);
	AdlibSoundInstrument *ins = &_instrumentsTable[channel];
	if (ins->mode != 0 && ins->channel == 6) {
		channel = 6;
	}
	if (ins->mode == 0 || channel == 6) {
		uint16 note = 12;
		int freq = _freqTable[note % 12];
		OPLWriteReg(_opl, 0xA0 | channel, freq);
		freq = ((note / 12) << 2) | ((freq & 0x300) >> 8);
		if (ins->mode == 0) {
			freq |= 0x20;
		}
		OPLWriteReg(_opl, 0xB0 | channel, freq);
	}
	if (ins->mode != 0) {
		_vibrato |= 1 << (10 - ins->channel);
		OPLWriteReg(_opl, 0xBD, _vibrato);
	}
}

void AdlibSoundDriverADL::loadInstrument(const byte *data, AdlibSoundInstrument *asi) {
	asi->mode = *data++;
	asi->channel = *data++;
	asi->waveSelectMod = *data++ & 3;
	asi->waveSelectCar = *data++ & 3;
	asi->amDepth = *data++;
	++data;
	loadRegisterInstrument(data, &asi->regMod); data += 26;
	loadRegisterInstrument(data, &asi->regCar); data += 26;
}

void AdlibSoundDriverADL::setChannelFrequency(int channel, int frequency) {
	assert(channel < 4);
	AdlibSoundInstrument *ins = &_instrumentsTable[channel];
	if (ins->mode != 0) {
		channel = ins->channel;
		if (channel == 9) {
			channel = 8;
		} else if (channel == 10) {
			channel = 7;
		}
	}
	int freq, note, oct;
	findNote(frequency, &note, &oct);

	note += oct * 12;
	if (ins->amDepth) {
		note = ins->amDepth;
	}
	if (note < 0) {
		note = 0;
	}

	freq = _freqTable[note % 12];
	OPLWriteReg(_opl, 0xA0 | channel, freq);
	freq = ((note / 12) << 2) | ((freq & 0x300) >> 8);
	if (ins->mode == 0) {
		freq |= 0x20;
	}
	OPLWriteReg(_opl, 0xB0 | channel, freq);
	if (ins->mode != 0) {
		_vibrato |= 1 << (10 - channel);
		OPLWriteReg(_opl, 0xBD, _vibrato);
	}
}

void AdlibSoundDriverADL::playSample(const byte *data, int size, int channel, int volume) {
	assert(channel < 4);
	_channelsVolumeTable[channel] = 127;
	setupInstrument(data, channel);
	AdlibSoundInstrument *ins = &_instrumentsTable[channel];
	if (ins->mode != 0 && ins->channel == 6) {
		OPLWriteReg(_opl, 0xB0 | channel, 0);
	}
	if (ins->mode != 0) {
		_vibrato &= ~(1 << (10 - ins->channel));
		OPLWriteReg(_opl, 0xBD, _vibrato);
	}
	if (ins->mode != 0) {
		channel = ins->channel;
		if (channel == 9) {
			channel = 8;
		} else if (channel == 10) {
			channel = 7;
		}
	}
	uint16 note = 48;
	if (ins->amDepth) {
		note = ins->amDepth;
	}
	int freq = _freqTable[note % 12];
	OPLWriteReg(_opl, 0xA0 | channel, freq);
	freq = ((note / 12) << 2) | ((freq & 0x300) >> 8);
	if (ins->mode == 0) {
		freq |= 0x20;
	}
	OPLWriteReg(_opl, 0xB0 | channel, freq);
	if (ins->mode != 0) {
		_vibrato |= 1 << (10 - channel);
		OPLWriteReg(_opl, 0xBD, _vibrato);
	}
}

PCSoundFxPlayer::PCSoundFxPlayer(PCSoundDriver *driver)
	: _playing(false), _driver(driver) {
	memset(_instrumentsData, 0, sizeof(_instrumentsData));
	_sfxData = NULL;
	_fadeOutCounter = 0;
	_driver->setUpdateCallback(updateCallback, this);
}

PCSoundFxPlayer::~PCSoundFxPlayer() {
	_driver->setUpdateCallback(NULL, NULL);
	if (_playing) {
		stop();
	}
}

bool PCSoundFxPlayer::load(const char *song) {
	debug(9, "PCSoundFxPlayer::load('%s')", song);

	/* stop (w/ fade out) the previous song */
	while (_fadeOutCounter != 0 && _fadeOutCounter < 100) {
		g_system->delayMillis(50);
	}
	_fadeOutCounter = 0;

	if (_playing) {
		stop();
	}

	_sfxData = readBundleSoundFile(song);
	if (!_sfxData) {
		warning("Unable to load soundfx module '%s'", song);
		return 0;
	}

	for (int i = 0; i < NUM_INSTRUMENTS; ++i) {
		_instrumentsData[i] = NULL;

		char instrument[13];
		memcpy(instrument, _sfxData + 20 + i * 30, 12);
		instrument[12] = '\0';

		if (strlen(instrument) != 0) {
			char *dot = strrchr(instrument, '.');
			if (dot) {
				*dot = '\0';
			}
			strcat(instrument, _driver->getInstrumentExtension());
			_instrumentsData[i] = readBundleSoundFile(instrument);
			if (!_instrumentsData[i]) {
				warning("Unable to load soundfx instrument '%s'", instrument);
			}
		}
	}
	return 1;
}

void PCSoundFxPlayer::play() {
	debug(9, "PCSoundFxPlayer::play()");
	if (_sfxData) {
		for (int i = 0; i < NUM_CHANNELS; ++i) {
			_instrumentsChannelTable[i] = -1;
		}
		_currentPos = 0;
		_currentOrder = 0;
		_numOrders = _sfxData[470];
		_eventsDelay = (252 - _sfxData[471]) * 50 / 1060;
		_updateTicksCounter = 0;
		_playing = true;
	}
}

void PCSoundFxPlayer::stop() {
	if (_playing || _fadeOutCounter != 0) {
		_fadeOutCounter = 0;
		_playing = false;
		for (int i = 0; i < NUM_CHANNELS; ++i) {
			_driver->stopChannel(i);
		}
		_driver->stopAll();
		unload();
	}
}

void PCSoundFxPlayer::fadeOut() {
	if (_playing) {
		_fadeOutCounter = 1;
		_playing = false;
	}
}

void PCSoundFxPlayer::updateCallback(void *ref) {
	((PCSoundFxPlayer *)ref)->update();
}

void PCSoundFxPlayer::update() {
	if (_playing || (_fadeOutCounter != 0 && _fadeOutCounter < 100)) {
		++_updateTicksCounter;
		if (_updateTicksCounter > _eventsDelay) {
			handleEvents();
			_updateTicksCounter = 0;
		}
	}
}

void PCSoundFxPlayer::handleEvents() {
	const byte *patternData = _sfxData + 600;
	const byte *orderTable = _sfxData + 472;
	uint16 patternNum = orderTable[_currentOrder] * 1024;

	for (int i = 0; i < 4; ++i) {
		handlePattern(i, patternData + patternNum + _currentPos);
		patternData += 4;
	}

	if (_fadeOutCounter != 0 && _fadeOutCounter < 100) {
		_fadeOutCounter += 2;
	}
	_currentPos += 16;
	if (_currentPos >= 1024) {
		_currentPos = 0;
		++_currentOrder;
		if (_currentOrder == _numOrders) {
			_currentOrder = 0;
		}
	}
	debug(7, "_currentOrder=%d/%d _currentPos=%d", _currentOrder, _numOrders, _currentPos);
}

void PCSoundFxPlayer::handlePattern(int channel, const byte *patternData) {
	int instrument = patternData[2] >> 4;
	if (instrument != 0) {
		--instrument;
		if (_instrumentsChannelTable[channel] != instrument || _fadeOutCounter != 0) {
			_instrumentsChannelTable[channel] = instrument;
			const int volume = _sfxData[instrument] - _fadeOutCounter;
			_driver->setupChannel(channel, _instrumentsData[instrument], instrument, volume);
		}
	}
	int16 freq = (int16)READ_BE_UINT16(patternData);
	if (freq > 0) {
		_driver->stopChannel(channel);
		_driver->setChannelFrequency(channel, freq);
	}
}

void PCSoundFxPlayer::unload() {
	for (int i = 0; i < NUM_INSTRUMENTS; ++i) {
		free(_instrumentsData[i]);
		_instrumentsData[i] = NULL;
	}
	free(_sfxData);
	_sfxData = NULL;
}


PCSound::PCSound(Audio::Mixer *mixer, CineEngine *vm)
	: Sound(mixer, vm) {
	if (_vm->getGameType() == GType_FW) {
		_soundDriver = new AdlibSoundDriverINS(_mixer);
	} else {
		_soundDriver = new AdlibSoundDriverADL(_mixer);
	}
	_player = new PCSoundFxPlayer(_soundDriver);
}

PCSound::~PCSound() {
	delete _player;
	delete _soundDriver;
}

void PCSound::loadMusic(const char *name) {
	_player->load(name);
}

void PCSound::playMusic() {
	_player->play();
}

void PCSound::stopMusic() {
	_player->stop();
}

void PCSound::fadeOutMusic() {
	_player->fadeOut();
}

void PCSound::playSound(int channel, int frequency, const uint8 *data, int size, int volumeStep, int stepCount, int volume, int repeat) {
	_soundDriver->playSample(data, size, channel, volume);
}

void PCSound::stopSound(int channel) {
	_soundDriver->resetChannel(channel);
}

PaulaSound::PaulaSound(Audio::Mixer *mixer, CineEngine *vm)
	: Sound(mixer, vm) {
	memset(_soundChannelsTable, 0, sizeof(_soundChannelsTable));
	_moduleStream = 0;
}

PaulaSound::~PaulaSound() {
}

void PaulaSound::loadMusic(const char *name) {
	if (_vm->getGameType() == GType_FW) {
		// look for separate files
		Common::File f;
		if (f.open(name)) {
			_moduleStream = Audio::makeSoundFxStream(&f, 0, _mixer->getOutputRate());
		}
	} else {
		// look in bundle files
		uint32 size;
		byte *buf = readBundleSoundFile(name, &size);
		if (buf) {
			Common::MemoryReadStream s(buf, size);
			_moduleStream = Audio::makeSoundFxStream(&s, readBundleSoundFile, _mixer->getOutputRate());
			free(buf);
		}
	}
}

void PaulaSound::playMusic() {
	_mixer->stopHandle(_moduleHandle);
	if (_moduleStream) {
		_mixer->playInputStream(Audio::Mixer::kMusicSoundType, &_moduleHandle, _moduleStream);
	}
}

void PaulaSound::stopMusic() {
	_mixer->stopHandle(_moduleHandle);
}

void PaulaSound::fadeOutMusic() {
	// TODO
	stopMusic();
}

void PaulaSound::playSound(int channel, int frequency, const uint8 *data, int size, int volumeStep, int stepCount, int volume, int repeat) {
	SoundChannel *ch = &_soundChannelsTable[channel];
	ch->frequency = frequency;
	ch->data = data;
	ch->size = size;
	ch->volumeStep = volumeStep;
	ch->stepCount = stepCount;
	ch->step = stepCount;
	ch->repeat = repeat != 0;
	ch->volume = volume;
}

void PaulaSound::stopSound(int channel) {
	_mixer->stopHandle(_channelsTable[channel]);
}

void PaulaSound::update() {
	// process volume slides and start sound playback
	for (int i = 0; i < NUM_CHANNELS; ++i) {
		SoundChannel *ch = &_soundChannelsTable[i];
		if (ch->data) {
			if (ch->step) {
				--ch->step;
				continue;
			}
			ch->step = ch->stepCount;
			ch->volume = CLIP(ch->volume + ch->volumeStep, 0, 63);
			playSoundChannel(i, ch->frequency, ch->data, ch->size, ch->volume);
			if (!ch->repeat) {
				ch->data = 0;
			}
		}
	}
}

void PaulaSound::playSoundChannel(int channel, int frequency, const uint8 *data, int size, int volume) {
	stopSound(channel);
	assert(frequency > 0);
	frequency = PAULA_FREQ / frequency;
	size = MIN<int>(size - SPL_HDR_SIZE, READ_BE_UINT16(data + 4));
	data += SPL_HDR_SIZE;
	if (size > 0) {
		_mixer->playRaw(Audio::Mixer::kSFXSoundType, &_channelsTable[channel], const_cast<byte *>(data), size, frequency, 0);
		_mixer->setChannelVolume(_channelsTable[channel], volume * Audio::Mixer::kMaxChannelVolume / 63);
	}
}

} // End of namespace Cine
